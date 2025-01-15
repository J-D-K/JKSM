# JK's Save Manager for 3DS
**Note: Rewrite is currently work-in-progress. There may be issues, bugs, and crashes. You're using this at your own risk until I consider it finished.** This is a work-in-progress rewrite and partial backport of the Switch version. **You have been warned for the time being.**

# Quick start guide:
#### Global controls:
* Pressing the L or R shoulder buttons will change what save data type is currently being targeted or take you to the settings menu.
* Pressing Start will exit JKSM.
* Pressing Select will allow you to set your play coins to whatever you want (for now).

#### Title selection(s):
* Up, down, left, and right will change which title is selected in both Tile and Text selection mode.
* Pressing A will open the currently highlighted title for backup and restore.
* Pressing X will open a small menu with a few extra options such as erasing save data, exporting and importing secure values from SD.

#### Backup Menu:
* Pressing A while "New Backup" is highlighted will open the keyboard and allow you to create a new backup.
* Pressing A while an already existing backup is highlighted will allow you to overwrite that backup with a new one.
* Pressing Y while an already existing backup is highlighted will allow you to restore the data contained within it.
* Pressing X while an already existing backup is highlighted will allow you to delete that backup from your SD card.
* Pressing B will close the currently open title and exit the backup menu.

#### Settings menu:
* Pressing up, down, left, or right will change which option is highlighted.
* Pressing A will toggle the option highlighted to On or Off. A short description of what each option controls is printed on the bottom screen.
* **NOTE:** ZIP support is only semi-functional at the current point in time.

# Building:
Building JKSM requires installing [DevkitPro](https://devkitpro.org/wiki/Getting_Started) for your respective platform and installing the following libraries:
```
3ds-sdl
3ds-libpng
3ds-freetype 
3ds-libjson-c
3ds-curl
3ds-libzstd
3ds-zlib
3ds-minizip
```
With those installed run the following command:
```
git clone --recurse-submodules https://github.com/J-D-K/JKSM.git
```
This will clone my FS Wrapper library along with JKSM's source code. Then, navigate to the JKSM directory and type the following command:
```
make -j
```
You should now have JKSM.3dsx inside the JKSM subfolder inside the repo on your machine.
## Customization:
JKSM currently uses [Google's NotoSanJP](https://fonts.google.com/noto/specimen/Noto+Sans+JP) font for drawing text. This font _is_ rather large. If you would like to use a different or smaller font, simply build the included fontcompressor program and replace the font in JKSM's romfs when building it.

## Credits:
JKSM uses code from:
* [The original 3DS homebrew menu/bch2obj.py](https://github.com/smealum/3ds_hb_menu/tree/master) - For loading SMDH's and un-tiling the icons into SDL_Surfaces.
