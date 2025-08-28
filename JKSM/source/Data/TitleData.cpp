#include "Data/TitleData.hpp"

#include "Assets.hpp"
#include "Config.hpp"
#include "Data/SMDH.hpp"
#include "SDL/SDL.hpp"
#include "StringUtil.hpp"
#include "fslib.hpp"
#include "logging/logger.hpp"

#include <array>
#include <cstring>

namespace
{
    // For untiling 3DS Icons. SDL 3DS doesn't support hardware acceleration.
    // Taken from 3DS Homebrew menu which took it from bch2obj.py?
    // Cleaned up to be easier to read and look nicer.
    constexpr std::array<uint8_t, 64> TILE_ORDER = {
        0x00, 0x01, 0x08, 0x09, 0x02, 0x03, 0x0A, 0x0B, 0x10, 0x11, 0x18, 0x19, 0x12, 0x13, 0x1A, 0x1B,
        0x04, 0x05, 0x0C, 0x0D, 0x06, 0x07, 0x0E, 0x0F, 0x14, 0x15, 0x1C, 0x1D, 0x16, 0x17, 0x1E, 0x1F,
        0x20, 0x21, 0x28, 0x29, 0x22, 0x23, 0x2A, 0x2B, 0x30, 0x31, 0x38, 0x39, 0x32, 0x33, 0x3A, 0x3B,
        0x24, 0x25, 0x2C, 0x2D, 0x26, 0x27, 0x2E, 0x2F, 0x34, 0x35, 0x3C, 0x3D, 0x36, 0x37, 0x3E, 0x3F};
    // For being super safe and memcpying the icon data and hoping someone doesn't pull some funny business with the cache :)
    constexpr size_t ICON_BUFFER_SIZE = sizeof(uint32_t) * 48 * 48;

    // Publisher for blank/unknown.
    constexpr std::u16string_view PUBLISHER_NOT_KNOWN = u"A Company?";
} // namespace

// Same as above, but modified to work with SDL_Surfaces.
static inline void WritePixelToSurface(SDL_Surface *Surface, int X, int Y, uint16_t Color)
{
    uint16_t SurfaceWidth = Surface->w;

    // Get pointer to surface pixels
    uint32_t *SurfacePixels = reinterpret_cast<uint32_t *>(Surface->pixels);

    // Separate R, G, B, convert from RGB656 to RGB888
    uint32_t Red   = (Color >> 11 & 0x1F) << 3;
    uint32_t Green = ((Color >> 5) & 0x3F) << 2;
    uint32_t Blue  = ((Color) & 0x1F) << 3;

    // Set pixel in surface to color.
    SurfacePixels[X + (Y * SurfaceWidth)] = Red << 24 | Green << 16 | Blue << 8 | 0xFF;
}

Data::TitleData::TitleData(uint64_t TitleID, FS_MediaType MediaType, Data::TitleSaveTypes SaveTypes)
    : m_TitleID(TitleID)
    , m_MediaType(MediaType)
    , m_TitleSaveTypes(SaveTypes)
{
    Result AMError = AM_GetTitleProductCode(m_MediaType, m_TitleID, m_ProductCode);
    if (R_FAILED(AMError))
    {
        logger::log("Error getting product code for %016llX.", m_TitleID);
        std::memset(m_ProductCode, 0x00, 0x20);
    }

    Data::SMDH TitleSMDH;
    if (TitleData::HasSaveData() && !Data::LoadSMDH(m_TitleID, m_MediaType, TitleSMDH)) { TitleData::TitleInitializeDefault(); }
    else if (TitleData::HasSaveData()) { TitleData::TitleInitializeSMDH(TitleSMDH); }
}

Data::TitleData::TitleData(uint64_t TitleID,
                           FS_MediaType MediaType,
                           const char *ProductCode,
                           const char16_t *Title,
                           const char16_t *Publisher,
                           Data::TitleSaveTypes SaveTypes,
                           const void *IconData)
    : m_TitleID(TitleID)
    , m_MediaType(MediaType)
    , m_TitleSaveTypes(SaveTypes)
{
    // The rest needs manual copying.
    std::memcpy(m_ProductCode, ProductCode, 0x20);
    std::memcpy(m_Title, Title, 0x40 * sizeof(char16_t));
    std::memcpy(m_Publisher, Publisher, 0x40 * sizeof(char16_t));

    StringUtil::SanitizeStringForPath(m_Title, m_PathSafeTitle, 0x40);

    // Now we allocate and memcpy the icon data.
    std::string TitleIDString = StringUtil::GetFormattedString("%016llX", m_TitleID);
    m_Icon                    = SDL::SurfaceManager::CreateLoadResource(TitleIDString, 48, 48, false);
    std::memcpy(m_Icon->Get()->pixels, IconData, ICON_BUFFER_SIZE);
}

bool Data::TitleData::HasSaveData() const
{
    for (size_t i = 0; i < Data::SaveTypeTotal; i++)
    {
        if (m_TitleSaveTypes.HasSaveType[i]) { return true; }
    }
    return false;
}

uint64_t Data::TitleData::GetTitleID() const { return m_TitleID; }

uint32_t Data::TitleData::GetLowerID() const { return static_cast<uint32_t>(m_TitleID & 0xFFFFFFFF); }

uint32_t Data::TitleData::GetUpperID() const { return static_cast<uint32_t>(m_TitleID >> 32 & 0xFFFFFFFF); }

uint32_t Data::TitleData::GetUniqueID() const { return TitleData::GetLowerID() >> 8; }

uint32_t Data::TitleData::GetExtDataID() const { return Data::ExtDataRedirect(m_TitleID); }

FS_MediaType Data::TitleData::GetMediaType() const { return m_MediaType; }

bool Data::TitleData::IsFavorite() const { return m_IsFavorite; }

const char *Data::TitleData::GetProductCode() const { return m_ProductCode; }

const char16_t *Data::TitleData::GetTitle() const { return m_Title; }

const char16_t *Data::TitleData::GetPathSafeTitle() const { return m_PathSafeTitle; }

const char16_t *Data::TitleData::GetPublisher() const { return m_Publisher; }

Data::TitleSaveTypes Data::TitleData::GetSaveTypes() const { return m_TitleSaveTypes; }

SDL::SharedSurface Data::TitleData::GetIcon() { return m_Icon; }

void Data::TitleData::TitleInitializeDefault()
{
    std::string TitleIDString = StringUtil::GetFormattedString("%016llX", m_TitleID);

    StringUtil::ToUTF16(TitleIDString.c_str(), m_Title, 0x40);
    StringUtil::ToUTF16(TitleIDString.c_str(), m_PathSafeTitle, 0x40);

    // Memcpy publisher string
    std::memcpy(m_Publisher, PUBLISHER_NOT_KNOWN.data(), PUBLISHER_NOT_KNOWN.length() * sizeof(char16_t));

    // This should just grab a pointer. Not load the font again.
    SDL::SharedFont Noto =
        SDL::FontManager::CreateLoadResource(Asset::Names::NOTO_SANS, Asset::Paths::NOTO_SANS_PATH, SDL::Colors::White);
    m_Icon = SDL::SurfaceManager::CreateLoadResource(TitleIDString, 48, 48, false);
    if (!Noto || !m_Icon) { return; }

    // Clear icon to bar color.
    uint32_t *IconPixels = reinterpret_cast<uint32_t *>(m_Icon->Get()->pixels);
    for (int i = 0; i < 48 * 48; i++) { *IconPixels++ = SDL::Colors::BarColor.RAW; }

    std::string UniqueString = StringUtil::GetFormattedString("%04X", m_TitleID & 0xFFFF);

    int TextX = 24 - (Noto->GetTextWidth(12, UniqueString.c_str()) / 2);
    Noto->BlitTextAt(m_Icon->Get(), TextX, 18, 12, Noto->NO_TEXT_WRAP, UniqueString.c_str());
}

void Data::TitleData::TitleInitializeSMDH(const Data::SMDH &SMDH)
{
    uint8_t SystemLanguage = Config::GetSystemLanguage();

    Result AmError = AM_GetTitleProductCode(m_MediaType, m_TitleID, m_ProductCode);
    if (R_FAILED(AmError)) { logger::log("Error getting product code for %016llX: 0x%08X.", m_TitleID, AmError); }

    if (std::char_traits<uint16_t>::length(SMDH.applicationTitles[SystemLanguage].shortDescription) <= 0)
    {
        std::memcpy(m_Title, SMDH.applicationTitles[CFG_LANGUAGE_EN].shortDescription, 0x40 * sizeof(uint16_t));
    }
    else { std::memcpy(m_Title, SMDH.applicationTitles[SystemLanguage].shortDescription, 0x40 * sizeof(uint16_t)); }

    StringUtil::SanitizeStringForPath(m_Title, m_PathSafeTitle, 0x40);

    if (std::char_traits<uint16_t>::length(SMDH.applicationTitles[SystemLanguage].publisher) <= 0)
    {
        std::memcpy(m_Publisher, SMDH.applicationTitles[CFG_LANGUAGE_EN].publisher, 0x40 * sizeof(uint16_t));
    }
    else { std::memcpy(m_Publisher, SMDH.applicationTitles[SystemLanguage].publisher, 0x40 * sizeof(uint16_t)); }

    // Here comes the icon part. I'm using SDL instead of citro so these need to be untiled. This is from the hbmenu.
    std::string TitleIDString = StringUtil::GetFormattedString("%016llX", m_TitleID);
    m_Icon                    = SDL::SurfaceManager::CreateLoadResource(TitleIDString, 48, 48, false);
    if (!m_Icon)
    {
        logger::log("Error creating icon surface for %016llX: %s.", m_TitleID, SDL_GetError());
        return;
    }

    const uint16_t *IconPixelData = SMDH.bigIconData;
    for (int Y = 0; Y < 48; Y += 8)
    {
        for (int X = 0; X < 48; X += 8)
        {
            for (int i = 0; i < 64; i++)
            {
                uint8_t TargetX = TILE_ORDER[i] & 0x07;
                uint8_t TargetY = TILE_ORDER[i] >> 3;
                WritePixelToSurface(m_Icon->Get(), X + TargetX, Y + TargetY, *IconPixelData++);
            }
        }
    }
}
