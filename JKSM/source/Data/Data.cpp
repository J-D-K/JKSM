#include "Data/Data.hpp"
#include "Data/ExtData.hpp"
#include "Data/SaveDataType.hpp"
#include "FsLib.hpp"
#include "JKSM.hpp"
#include "Logger.hpp"
#include "SDL/SDL.hpp"
#include "StringUtil.hpp"
#include "UI/Strings.hpp"
#include <3ds.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

namespace
{
    // Path to cache.
    constexpr std::u16string_view CACHE_PATH = u"sdmc:/JKSM/cache.bin";
    // Magic/JKSM header.
    constexpr uint32_t CACHE_MAGIC = 0x4D534B4A;
    // The current cache revision required.
    constexpr uint8_t CURRENT_CACHE_REVISION = 0x09;
    // Buffer size for compressing icon data.
    constexpr size_t ICON_BUFFER_SIZE = sizeof(uint32_t) * 48 * 48;
    // This struct is to make reading and writing the cache quicker with fewer read/write calls.
    typedef struct
    {
            uint64_t TitleID;
            FS_MediaType MediaType;
            Data::TitleSaveTypes SaveTypes;
            char ProductCode[0x20];
            char16_t Title[0x40];
            char16_t Publisher[0x40];
            uint32_t Icon[0x900];
    } CacheEntry;

    // Vector instead of map to preserve order and sorting.
    std::vector<Data::TitleData> s_TitleVector;
    // Array of fake title ID's to add shared extdata to the TitleVector.
    constexpr std::array<uint64_t, 7> s_FakeSharedTitleIDs = {0x00048000F0000001,
                                                              0x00048000F0000002,
                                                              0x00048000F0000009,
                                                              0x00048000F000000B,
                                                              0x00048000F000000C,
                                                              0x00048000F000000D,
                                                              0x00048000F000000E};

    // Mount point for testing save archives.
    constexpr std::u16string_view TEST_MOUNT = u"TestRoot";

    // This is to prevent the main thread from requesting a cart read before data is finished being read.
    bool s_DataInitialized = false;
} // namespace

// These are declarations. Defined at end of file.
static bool LoadCacheFile(System::ProgressTask *Task);
static void CreateCacheFile(System::ProgressTask *Task);

// This is for sorting titles pseudo-alphabetically.
static bool CompareTitles(const Data::TitleData &TitleA, const Data::TitleData &TitleB)
{
    const char16_t *TitleATitle = TitleA.GetTitle();
    const char16_t *TitleBTitle = TitleB.GetTitle();

    size_t TitleALength = std::char_traits<char16_t>::length(TitleATitle);
    size_t TitleBLength = std::char_traits<char16_t>::length(TitleBTitle);
    size_t ShortestTitle = TitleALength < TitleBLength ? TitleALength : TitleBLength;
    for (size_t i = 0; i < ShortestTitle; i++)
    {
        int CharA = std::tolower(TitleATitle[i]);
        int CharB = std::tolower(TitleBTitle[i]);
        if (CharA != CharB)
        {
            return CharA < CharB;
        }
    }
    return false;
}

// This is to test what archives can be opened with the title id before bothering to allocate a new instance of Data::TitleData
static bool TestArchivesWithTitleID(uint64_t TitleID, FS_MediaType MediaType, Data::TitleSaveTypes &SaveTypesOut)
{
    uint32_t UpperID = TitleID >> 32 & 0xFFFFFFFF;
    uint32_t LowerID = TitleID & 0xFFFFFFFF;
    uint32_t ExtDataID = Data::ExtDataRedirect(TitleID);

    if ((MediaType == MEDIATYPE_SD || MediaType == MEDIATYPE_GAME_CARD) && FsLib::OpenUserSaveData(TEST_MOUNT, MediaType, LowerID, UpperID))
    {
        FsLib::CloseDevice(TEST_MOUNT);
        SaveTypesOut.HasSaveType[Data::SaveTypeUser] = true;
    }

    if ((MediaType == MEDIATYPE_SD || MediaType == MEDIATYPE_GAME_CARD) && FsLib::OpenExtData(TEST_MOUNT, ExtDataID))
    {
        FsLib::CloseDevice(TEST_MOUNT);
        SaveTypesOut.HasSaveType[Data::SaveTypeExtData] = true;
    }

    if (MediaType == MEDIATYPE_NAND && FsLib::OpenSharedExtData(TEST_MOUNT, LowerID))
    {
        FsLib::CloseDevice(TEST_MOUNT);
        SaveTypesOut.HasSaveType[Data::SaveTypeSharedExtData] = true;
    }

    if (MediaType == MEDIATYPE_NAND && FsLib::OpenBossExtData(TEST_MOUNT, ExtDataID))
    {
        FsLib::CloseDevice(TEST_MOUNT);
        SaveTypesOut.HasSaveType[Data::SaveTypeBossExtData] = true;
    }

    if (MediaType == MEDIATYPE_NAND && FsLib::OpenSystemSaveData(TEST_MOUNT, LowerID >> 8))
    {
        FsLib::CloseDevice(TEST_MOUNT);
        SaveTypesOut.HasSaveType[Data::SaveTypeSystem] = true;
    }

    // I didn't feel like typing this out one by one.
    for (size_t i = 0; i < Data::SaveTypeTotal; i++)
    {
        if (SaveTypesOut.HasSaveType[i])
        {
            return true;
        }
    }
    return false;
}

void Data::Initialize(System::ProgressTask *Task)
{
    s_DataInitialized = false;
    // Just in case.
    s_TitleVector.clear();

    if (FsLib::FileExists(CACHE_PATH) && LoadCacheFile(Task))
    {
        JKSM::RefreshSaveTypeStates();
        s_DataInitialized = true;
        Task->Finish();
        return;
    }

    uint32_t SDTitleCount = 0;
    Result AmError = AM_GetTitleCount(MEDIATYPE_SD, &SDTitleCount);
    if (R_FAILED(AmError))
    {
        Logger::Log("Error getting title count for SD: 0x%08X.", AmError);
        Task->Finish();
        return;
    }

    uint32_t TitlesRead = 0;
    std::unique_ptr<uint64_t[]> TitleIDList(new uint64_t[SDTitleCount]);
    AmError = AM_GetTitleList(&TitlesRead, MEDIATYPE_SD, SDTitleCount, TitleIDList.get());
    if (R_FAILED(AmError))
    {
        Logger::Log("Error getting title ID list for SD: 0x%08X.", AmError);
        Task->Finish();
        return;
    }

    Task->Reset(static_cast<double>(SDTitleCount - 1.0f));
    for (uint32_t i = 0; i < SDTitleCount; i++)
    {
        // Set status. Update task.
        Task->SetStatus(UI::Strings::GetStringByName(UI::Strings::Names::DataLoadingText, 0), TitleIDList[i]);
        Task->SetCurrent(static_cast<double>(i));

        uint32_t UpperID = static_cast<uint32_t>(TitleIDList[i] >> 32);
        if (UpperID != 0x00040000 && UpperID != 0x00040002)
        {
            continue;
        }

        Data::TitleSaveTypes SaveTypes = {false};
        if (TestArchivesWithTitleID(TitleIDList[i], MEDIATYPE_SD, SaveTypes))
        {
            s_TitleVector.emplace_back(TitleIDList[i], MEDIATYPE_SD, SaveTypes);
        }
    }

    uint32_t NandTitleCount = 0;
    AmError = AM_GetTitleCount(MEDIATYPE_NAND, &NandTitleCount);
    if (R_FAILED(AmError))
    {
        Logger::Log("Error getting title count for NAND: 0x%08X.", AmError);
        Task->Finish();
        return;
    }

    uint32_t NandTitlesRead = 0;
    TitleIDList = std::make_unique<uint64_t[]>(NandTitleCount);
    AmError = AM_GetTitleList(&NandTitlesRead, MEDIATYPE_NAND, NandTitleCount, TitleIDList.get());
    if (R_FAILED(AmError))
    {
        Logger::Log("Error getting title ID list for NAND: 0x%08X.", AmError);
        Task->Finish();
        return;
    }

    Task->Reset(static_cast<double>(NandTitleCount - 1.0f));
    for (uint32_t i = 0; i < NandTitleCount; i++)
    {
        Task->SetStatus(UI::Strings::GetStringByName(UI::Strings::Names::DataLoadingText, 1), TitleIDList[i]);
        Task->SetCurrent(static_cast<double>(i));

        Data::TitleSaveTypes SaveTypes = {false};
        if (TestArchivesWithTitleID(TitleIDList[i], MEDIATYPE_NAND, SaveTypes))
        {
            s_TitleVector.emplace_back(TitleIDList[i], MEDIATYPE_NAND, SaveTypes);
        }
    }

    // Shared Extdata. These are fake and pushed at the end just to have them.
    Task->SetStatus(UI::Strings::GetStringByName(UI::Strings::Names::DataLoadingText, 2));
    Task->Reset(6.0f);
    // We're gonna skip testing these.
    Data::TitleSaveTypes SharedType = {false};
    SharedType.HasSaveType[Data::SaveTypeSharedExtData] = true;
    for (size_t i = 0; i < 7; i++)
    {
        s_TitleVector.emplace_back(s_FakeSharedTitleIDs.at(i), MEDIATYPE_NAND, SharedType);
        Task->SetCurrent(static_cast<double>(i));
    }

    std::sort(s_TitleVector.begin(), s_TitleVector.end(), CompareTitles);

    CreateCacheFile(Task);

    JKSM::RefreshSaveTypeStates();
    s_DataInitialized = true;
    Task->Finish();
}

// To do: This works, but not up to current standards.
bool Data::GameCardUpdateCheck(void)
{
    if (!s_DataInitialized)
    {
        return false;
    }

    // Game card always sits at the beginning of vector.
    FS_MediaType BeginningMediaType = s_TitleVector.begin()->GetMediaType();

    bool CardInserted = false;
    Result FsError = FSUSER_CardSlotIsInserted(&CardInserted);
    if (R_FAILED(FsError))
    {
        return false;
    }

    if (!CardInserted && BeginningMediaType == MEDIATYPE_GAME_CARD)
    {
        s_TitleVector.erase(s_TitleVector.begin());
        return true;
    }

    // Only 3DS for now.
    FS_CardType CardType;
    FsError = FSUSER_GetCardType(&CardType);
    if (R_FAILED(FsError) || CardType == CARD_TWL)
    {
        return false;
    }

    if (CardInserted && BeginningMediaType != MEDIATYPE_GAME_CARD)
    {
        // This is just 1 for everything.
        uint32_t TitlesRead = 0;
        uint64_t GameCardTitleID = 0;
        Result AmError = AM_GetTitleList(&TitlesRead, MEDIATYPE_GAME_CARD, 1, &GameCardTitleID);
        if (R_FAILED(AmError))
        {
            return false;
        }

        Data::TitleSaveTypes GameCardTypes = {false};
        if (TestArchivesWithTitleID(GameCardTitleID, MEDIATYPE_GAME_CARD, GameCardTypes))
        {
            s_TitleVector.insert(s_TitleVector.begin(), std::move(Data::TitleData(GameCardTitleID, MEDIATYPE_GAME_CARD, GameCardTypes)));
        }

        return true;
    }

    return false;
}

void Data::GetTitlesWithType(Data::SaveDataType SaveType, std::vector<Data::TitleData *> &Out)
{
    Out.clear();

    auto CurrentTitle = s_TitleVector.begin();
    while ((CurrentTitle = std::find_if(CurrentTitle, s_TitleVector.end(), [SaveType](const Data::TitleData &Title) {
                return Title.GetSaveTypes().HasSaveType[SaveType];
            })) != s_TitleVector.end())
    {
        Out.push_back(&(*CurrentTitle));
        ++CurrentTitle;
    }
}

bool LoadCacheFile(System::ProgressTask *Task)
{
    FsLib::File CacheFile(CACHE_PATH, FS_OPEN_READ);
    if (!CacheFile.IsOpen())
    {
        Logger::Log("Error opening the cache for reading: %s", FsLib::GetErrorString());
        return false;
    }

    uint32_t Magic = 0;
    CacheFile.Read(&Magic, sizeof(uint32_t));
    if (Magic != CACHE_MAGIC)
    {
        Logger::Log("Error reading cache: Invalid cache file.");
        return false;
    }

    uint16_t TitleCount = 0;
    CacheFile.Read(&TitleCount, sizeof(uint16_t));

    uint8_t Revision = 0;
    CacheFile.Read(&Revision, sizeof(uint8_t));
    if (Revision != CURRENT_CACHE_REVISION)
    {
        Logger::Log("Old cache revision found. Forcing rescan.");
        return false;
    }

    Task->SetStatus(UI::Strings::GetStringByName(UI::Strings::Names::DataLoadingText, 3));
    // This was special for 3DS so loading wouldn't be so long. I didn't know it was going to be literally instantaneous...
    // Buffer for storing all entries at once.
    std::unique_ptr<CacheEntry[]> CacheBuffer(new CacheEntry[TitleCount]);
    CacheFile.Read(CacheBuffer.get(), sizeof(CacheEntry) * TitleCount);

    Task->Reset(static_cast<double>(TitleCount));
    for (uint16_t i = 0; i < TitleCount; i++)
    {
        CacheEntry *EntryData = &CacheBuffer[i];
        s_TitleVector.emplace_back(EntryData->TitleID,
                                   EntryData->MediaType,
                                   EntryData->ProductCode,
                                   EntryData->Title,
                                   EntryData->Publisher,
                                   EntryData->SaveTypes,
                                   EntryData->Icon);
        Task->SetCurrent(static_cast<double>(i));
    }
    return true;
}

void CreateCacheFile(System::ProgressTask *Task)
{
    FsLib::File CacheFile(CACHE_PATH, FS_OPEN_CREATE | FS_OPEN_WRITE);
    if (!CacheFile.IsOpen())
    {
        Logger::Log("Error opening cache file for writing: %s", FsLib::GetErrorString());
        return;
    }

    uint16_t TitleCount = s_TitleVector.size();

    // Magic, titlecount revision
    CacheFile.Write(&CACHE_MAGIC, sizeof(uint32_t));
    CacheFile.Write(&TitleCount, sizeof(uint16_t));
    CacheFile.Write(&CURRENT_CACHE_REVISION, sizeof(uint8_t));

    Task->Reset(static_cast<double>(s_TitleVector.size()));
    // This is the entry we write to.
    std::unique_ptr<CacheEntry> CurrentEntry(new CacheEntry);
    // Keep track of progress.
    double CurrentTitleCount = 0;
    for (Data::TitleData &CurrentTitle : s_TitleVector)
    {
        // Get title in UTF-8
        char UTF8Title[0x80] = {0};
        StringUtil::ToUTF8(CurrentTitle.GetTitle(), UTF8Title, 0x80);
        // Update thread.
        Task->SetStatus(UI::Strings::GetStringByName(UI::Strings::Names::DataLoadingText, 4), UTF8Title);
        // Copy needed data over from title vector.
        CurrentEntry->TitleID = CurrentTitle.GetTitleID();
        CurrentEntry->MediaType = CurrentTitle.GetMediaType();
        CurrentEntry->SaveTypes = CurrentTitle.GetSaveTypes();
        std::memcpy(CurrentEntry->ProductCode, CurrentTitle.GetProductCode(), 0x20);
        std::memcpy(CurrentEntry->Title, CurrentTitle.GetTitle(), 0x40 * sizeof(char16_t));
        std::memcpy(CurrentEntry->Publisher, CurrentTitle.GetPublisher(), 0x40 * sizeof(char16_t));
        std::memcpy(CurrentEntry->Icon, CurrentTitle.GetIcon()->Get()->pixels, ICON_BUFFER_SIZE);
        CacheFile.Write(CurrentEntry.get(), sizeof(CacheEntry));
        Task->SetCurrent(++CurrentTitleCount);
    }
}
