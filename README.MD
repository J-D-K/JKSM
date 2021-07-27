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

This is a rewrite that is based on the original's code with the Switch's UI backported. Basically, sort of what I wanted to do in the first place before I disappeared.

**See [releases](https://github.com/J-D-K/JKSM/releases) for the latest version and QR code or my [Google Drive](https://drive.google.com/drive/folders/11UPij1YqIzJW_Mh8JnR9o-7AYuxRlN-S?usp=sharing) for early betas and updates.** 

## Requires:
[libctru](https://github.com/smealum/ctrulib), [citro3d](https://github.com/fincs/citro3d), [citro2d](https://github.com/devkitPro/citro2d), and 3ds-zlib