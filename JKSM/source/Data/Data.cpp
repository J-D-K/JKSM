#include "Data/Data.hpp"
#include "FsLib.hpp"
#include "Logger.hpp"
#include "SDL/SDL.hpp"
#include <3ds.h>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>
#include <zlib.h>

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
} // namespace

// These are declarations. Defined at end of file.
static bool LoadCacheFile(void);
static void CreateCacheFile(void);

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

bool Data::Initialize(void)
{
    if (LoadCacheFile())
    {
        return true;
    }

    uint32_t TitleCount = 0;
    Result AmError = AM_GetTitleCount(MEDIATYPE_SD, &TitleCount);
    if (R_FAILED(AmError))
    {
        Logger::Log("Error getting title count for SD: 0x%08X.", AmError);
        return false;
    }

    uint32_t TitlesRead = 0;
    std::unique_ptr<uint64_t[]> TitleIDList(new uint64_t[TitleCount]);
    AmError = AM_GetTitleList(&TitlesRead, MEDIATYPE_SD, TitleCount, TitleIDList.get());
    if (R_FAILED(AmError))
    {
        Logger::Log("Error getting title ID list for SD: 0x%08X.", AmError);
        return false;
    }

    for (uint32_t i = 0; i < TitleCount; i++)
    {
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
        return false;
    }

    uint32_t NandTitlesRead = 0;
    TitleIDList = std::make_unique<uint64_t[]>(NandTitleCount);
    AmError = AM_GetTitleList(&NandTitlesRead, MEDIATYPE_NAND, NandTitleCount, TitleIDList.get());
    if (R_FAILED(AmError))
    {
        Logger::Log("Error getting title ID list for NAND: 0x%08X.", AmError);
        return false;
    }

    for (uint32_t i = 0; i < NandTitleCount; i++)
    {
        Data::TitleData NewNANDTitle(TitleIDList[i], MEDIATYPE_NAND);
        if (NewNANDTitle.HasSaveData())
        {
            s_TitleVector.push_back(std::move(NewNANDTitle));
        }
    }

    std::sort(s_TitleVector.begin(), s_TitleVector.end(), CompareTitles);

    CreateCacheFile();

    return true;
}

// To do: This works, but not up to current standards.
bool Data::GameCardUpdateCheck(void)
{
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

bool LoadCacheFile(void)
{
    /*
        Big note: Technically I should be checking the read counts but I believe in myself.
    */

    if (!FsLib::FileExists(CACHE_PATH))
    {
        return false;
    }

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

    // I'm putting these here so they don't get reallocated every loop.
    std::unique_ptr<Bytef[]> CompressedIconBuffer(new Bytef[ICON_BUFFER_SIZE]);
    std::unique_ptr<Bytef[]> DecompressedIconBuffer(new Bytef[ICON_BUFFER_SIZE]);
    for (uint16_t i = 0; i < TitleCount; i++)
    {
        uint64_t TitleID = 0;
        CacheFile.Read(&TitleID, sizeof(uint64_t));

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
        uLongf CompressedIconSize = 0;
        uLongf IconBufferSize = ICON_BUFFER_SIZE;
        CacheFile.Read(&CompressedIconSize, sizeof(uLongf));
        CacheFile.Read(CompressedIconBuffer.get(), CompressedIconSize);
        // Decompress
        int ZError = uncompress(DecompressedIconBuffer.get(), &IconBufferSize, CompressedIconBuffer.get(), CompressedIconSize);
        if (ZError != Z_OK)
        {
            Logger::Log("Error decompressing icon for %016llX.", TitleID);
            continue;
        }

        s_TitleVector.emplace_back(TitleID, MediaType, ProductCode, Title, Publisher, SaveTypes, DecompressedIconBuffer.get());
    }
    return true;
}

void CreateCacheFile(void)
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

    std::unique_ptr<Bytef[]> IconCompressionBuffer(new Bytef[ICON_BUFFER_SIZE]);
    for (Data::TitleData &CurrentTitle : s_TitleVector)
    {
        uint64_t TitleID = CurrentTitle.GetTitleID();
        FS_MediaType MediaType = CurrentTitle.GetMediaType();
        Data::TitleSaveTypes SaveTypes = CurrentTitle.GetSaveTypes();

        CacheFile.Write(&TitleID, sizeof(uint64_t));
        CacheFile.Write(&MediaType, sizeof(FS_MediaType));
        CacheFile.Write(CurrentTitle.GetProductCode(), 0x20);
        CacheFile.Write(&SaveTypes, sizeof(Data::TitleSaveTypes));
        CacheFile.Write(CurrentTitle.GetTitle(), 0x40 * sizeof(char16_t));
        CacheFile.Write(CurrentTitle.GetPublisher(), 0x40 * sizeof(char16_t));

        // Icon is trickier.
        uLongf CompressedIconSize = ICON_BUFFER_SIZE;
        int ZError = compress(IconCompressionBuffer.get(),
                              &CompressedIconSize,
                              reinterpret_cast<const Bytef *>(CurrentTitle.GetIcon()->Get()->pixels),
                              ICON_BUFFER_SIZE);
        if (ZError != Z_OK)
        {
            Logger::Log("Error compressing icon for cache!");
        }
        else
        {
            CacheFile.Write(&CompressedIconSize, sizeof(uLongf));
            CacheFile.Write(IconCompressionBuffer.get(), CompressedIconSize);
        }
    }
}
