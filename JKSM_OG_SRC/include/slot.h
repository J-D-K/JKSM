#ifndef SLOT_H
#define SLOT_H

#include <string>
#include "titledata.h"

//nSlot = whether to allow new dir creation
//Data = TitleData object containing info
//Ext = Using ExtData
std::u16string getFolder(const titleData dat, int mode, bool newFolder);

#endif // SLOT_H
