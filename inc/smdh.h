#ifndef SMDH_H
#define SMDH_H

//Stolen from 3DS HB menu
typedef struct
{
    uint32_t magic;
    uint16_t version;
    uint16_t reserved;
} smdhHeader_s;

typedef struct
{
    uint16_t shortDescription[0x40];
    uint16_t longDescription[0x80];
    uint16_t publisher[0x40];
} smdhTitle_s;

typedef struct
{
    uint8_t gameRatings[0x10];
    uint32_t regionLock;
    uint8_t matchMakerId[0xC];
    uint32_t flags;
    uint16_t eulaVersion;
    uint16_t reserved;
    uint16_t defaultFrame;
    uint32_t cecId;
} smdhSettings_s;

typedef struct
{
    smdhHeader_s header;
    smdhTitle_s applicationTitles[16];
    smdhSettings_s settings;
    uint8_t reserved[0x8];
    uint8_t smallIconData[0x480];
    uint16_t bigIconData[0x900];
} smdh_s;
#endif // SMDH_H
