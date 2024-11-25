#include "Data/Data.hpp"
#include "FsLib.hpp"
#include "JKSM.hpp"
#include "Logger.hpp"
#include "SDL/SDL.hpp"
#include "StringUtil.hpp"
#include <3ds.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

namespace
{
    // This is to check the cache for no other reason than to do it.
    constexpr uint32_t CACHE_MAGIC = 0x4D534B4A;
    // Buffer size for compressing icon data.
    constexpr size_t ICON_BUFFER_SIZE = sizeof(uint32_t) * 48 * 48;
    // The current cache revision required.
    constexpr uint8_t CURRENT_CACHE_REVISION = 0x08;
    // Path to cache.
    constexpr std::u16string_view CACHE_PATH = u"sdmc:/JKSM/cache.bin";
    // Vector instead of map to preserve order and sorting.
    std::vector<Data::TitleData> s_TitleVector;
    // Array of fake title ID's to add shared extdata to the TitleVector.
    constexpr std::array<uint64_t, 7> s_FakeSharedTitleIDs = {0x00000000F0000001,
                                                              0x00000000F0000002,
                                                              0x00000000F0000009,
                                                              0x00000000F000000B,
                                                              0x00000000F000000C,
                                                              0x00000000F000000D,
                                                              0x00000000F000000E};
    // This is to prevent the main thread from requesting a cart read before data is finished being read.
    bool s_DataInitialized = false;
} // namespace

// These are declarations. Defined at end of file.
static bool LoadCacheFile(System::Task *Task);
static void CreateCacheFile(System::Task *Task);

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

void Data::Initialize(System::Task *Task)
{
    s_DataInitialized = false;
    // Just in case.
    s_TitleVector.clear();

    if (LoadCacheFile(Task))
    {
        JKSM::RefreshSaveTypeStates();
        s_DataInitialized = true;
        Task->Finish();
        return;
    }

    Task->SetStatus("Retrieving SD title list from system...");
    uint32_t TitleCount = 0;
    Result AmError = AM_GetTitleCount(MEDIATYPE_SD, &TitleCount);
    if (R_FAILED(AmError))
    {
        Logger::Log("Error getting title count for SD: 0x%08X.", AmError);
        Task->Finish();
        return;
    }

    uint32_t TitlesRead = 0;
    std::unique_ptr<uint64_t[]> TitleIDList(new uint64_t[TitleCount]);
    AmError = AM_GetTitleList(&TitlesRead, MEDIATYPE_SD, TitleCount, TitleIDList.get());
    if (R_FAILED(AmError))
    {
        Logger::Log("Error getting title ID list for SD: 0x%08X.", AmError);
        Task->Finish();
        return;
    }

    for (uint32_t i = 0; i < TitleCount; i++)
    {
        Task->SetStatus("Loading SD title %016llX", TitleIDList[i]);

        uint32_t UpperID = static_cast<uint32_t>(TitleIDList[i] >> 32);
        if (UpperID != 0x00040000 && UpperID != 0x00040002)
        {
            continue;
        }

        Data::TitleData NewTitleData(TitleIDList[i], MEDIATYPE_SD);
        if (NewTitleData.HasSaveData())
        {
            s_TitleVector.push_back(std::move(NewTitleData));
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

    for (uint32_t i = 0; i < NandTitleCount; i++)
    {
        Task->SetStatus("Loading NAND title %016llX", TitleIDList[i]);
        Data::TitleData NewNANDTitle(TitleIDList[i], MEDIATYPE_NAND);
        if (NewNANDTitle.HasSaveData())
        {
            s_TitleVector.push_back(std::move(NewNANDTitle));
        }
    }

    // Shared Extdata. These are fake and pushed at the end just to have them.
    for (size_t i = 0; i < 7; i++)
    {
        s_TitleVector.emplace_back(s_FakeSharedTitleIDs.at(i), MEDIATYPE_NAND);
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

        Data::TitleData GameCardData(GameCardTitleID, MEDIATYPE_GAME_CARD);
        if (!GameCardData.HasSaveData())
        {
            return false;
        }
        s_TitleVector.insert(s_TitleVector.begin(), std::move(GameCardData));

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

bool LoadCacheFile(System::Task *Task)
{
    if (!FsLib::FileExists(CACHE_PATH))
    {
        return false;
    }
    Task->SetStatus("Reading cache file...");

    FsLib::InputFile CacheFile(CACHE_PATH);

    // Test magic for whatever reason. It's all the rage!
    uint32_t Magic = 0;
    CacheFile.Read(&Magic, sizeof(uint32_t));
    if (Magic != CACHE_MAGIC)
    {
        // UH OH
        Logger::Log("Error reading cache: Arbitrary value with no meaning I decided is important is WRONG!");
        return false;
    }

    uint16_t TitleCount = 0;
    CacheFile.Read(&TitleCount, sizeof(uint16_t));

    uint8_t CacheRevision = 0;
    CacheFile.Read(&CacheRevision, sizeof(uint8_t));
    if (CacheRevision != CURRENT_CACHE_REVISION)
    {
        Logger::Log("Old cache revision found. Hope you had time to wait for this.");
        return false;
    }

    std::unique_ptr<unsigned char[]> IconBuffer(new unsigned char[ICON_BUFFER_SIZE]);
    for (uint16_t i = 0; i < TitleCount; i++)
    {
        uint64_t TitleID = 0;
        CacheFile.Read(&TitleID, sizeof(uint64_t));

        Task->SetStatus("Reading data for %016llX", TitleID);

        FS_MediaType MediaType;
        CacheFile.Read(&MediaType, sizeof(FS_MediaType));

        char ProductCode[0x20] = {0};
        CacheFile.Read(ProductCode, 0x20);

        Data::TitleSaveTypes SaveTypes;
        CacheFile.Read(&SaveTypes, sizeof(Data::TitleSaveTypes));

        char16_t Title[0x40] = {0};
        CacheFile.Read(Title, 0x40 * sizeof(char16_t));

        char16_t Publisher[0x40] = {0};
        CacheFile.Read(Publisher, 0x40 * sizeof(char16_t));

        // Icon. We always know the end size since it's RAW RGBA8 pixels.
        CacheFile.Read(IconBuffer.get(), ICON_BUFFER_SIZE);

        s_TitleVector.emplace_back(TitleID, MediaType, ProductCode, Title, Publisher, SaveTypes, IconBuffer.get());
    }
    return true;
}

void CreateCacheFile(System::Task *Task)
{
    FsLib::OutputFile CacheFile(CACHE_PATH, false);
    if (!CacheFile.IsOpen())
    {
        return;
    }

    CacheFile.Write(&CACHE_MAGIC, sizeof(uint32_t));

    uint16_t TitleCount = static_cast<uint16_t>(s_TitleVector.size());
    CacheFile.Write(&TitleCount, sizeof(uint16_t));

    CacheFile.Write(&CURRENT_CACHE_REVISION, sizeof(uint8_t));

    for (Data::TitleData &CurrentTitle : s_TitleVector)
    {
        char UTF8Title[0x80] = {0};
        StringUtil::ToUTF8(CurrentTitle.GetTitle(), UTF8Title, 0x80);

        Task->SetStatus("Writing %s's data...", UTF8Title);
        uint64_t TitleID = CurrentTitle.GetTitleID();
        FS_MediaType MediaType = CurrentTitle.GetMediaType();
        Data::TitleSaveTypes SaveTypes = CurrentTitle.GetSaveTypes();

        CacheFile.Write(&TitleID, sizeof(uint64_t));
        CacheFile.Write(&MediaType, sizeof(FS_MediaType));
        CacheFile.Write(CurrentTitle.GetProductCode(), 0x20);
        CacheFile.Write(&SaveTypes, sizeof(Data::TitleSaveTypes));
        CacheFile.Write(CurrentTitle.GetTitle(), 0x40 * sizeof(char16_t));
        CacheFile.Write(CurrentTitle.GetPublisher(), 0x40 * sizeof(char16_t));
        CacheFile.Write(CurrentTitle.GetIcon()->Get()->pixels, ICON_BUFFER_SIZE);
    }
}
