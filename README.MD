# JKSM for 3DS
JKSM or JK's Save Manager for 3DS. Switch UI is currently being backported, so may be missing features if built from source.

* Circle Pad Left or Right Changes Save Data Type
* While in title selection:
    * A Opens the highlighted title
	* Y Adds the highlighted title to favorites.
	* More later
	
* While Folder Selection is open:
    * A Overwrites an already dumped backup or selects New
	* X Deletes the highlighted folder
	* Y Restores the highlighted folder
	* B Closes and returns to title select

**Google Drive Instructions:**
Follow the guide [here](https://github.com/J-D-K/JKSV/blob/master/GD_INSTRUCTIONS.MD). For 3DS, step 24 is a bit different as JKSM 3DS does not have a text config file. Instead, create a file named `drive.json` and use this as the template:
```
{
    "driveClientID" : "YOUR_DRIVE_CLIENT_ID_HERE",
    "driveClientSecret" : "YOUR_DRIVE_CLIENT_SECRET",
    "driveAuthCode" : "YOU_CODE_FROM_STEP_22_HERE"
}
```
Place `drive.json` inside the `JKSV` folder in the root of your SD card. 

**NOTE: Google Drive support currently requires JKSM to be built from source or be downloaded from my google drive links below. Only saves created with the `Export To Zip` option turned on are uploadable for now.**


This is a rewrite that is based on the original's code with the Switch's UI backported. Basically, sort of what I wanted to do in the first place before I disappeared.

**See [releases](https://github.com/J-D-K/JKSM/releases) for the latest version and QR code or my [Google Drive](https://drive.google.com/drive/folders/11UPij1YqIzJW_Mh8JnR9o-7AYuxRlN-S?usp=sharing) for early betas and updates.** 

## Building:
* Setup [devkitPro](https://www.3dbrew.org/wiki/Setting_up_Development_Environment)
* Run this command. You may need to remove `dkp`:
	* `dkp-pacman -S 3ds-curl 3ds-libjson-c 3ds-mbedtls 3ds-zlib`