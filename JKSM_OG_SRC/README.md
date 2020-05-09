# JKSM
JKSM - JK's Save Manager

Requires:
	[smealum's ctrulib](https://github.com/smealum/ctrulib)
	[xerpi's portlibs](https://github.com/xerpi/3ds_portlibs)	
	[xerpi's sf2d](https://github.com/xerpi/sf2dlib/tree/effe77ea81d21c26bad457d4f5ed8bb16ce7b753)
	[xerpi's sftd](https://github.com/xerpi/sftdlib)

Place makerom inside your devkitARM's bin directory. Type 'make cia' inside the project's directory to build.

A custom font can be used by placing it in the sd:/JKSV folder and naming it "font.ttf". I also recommend replacing the font with a smaller one if you're building a 3dsx.

Due to the fact that I cannot keep up with homebrew releases anymore, I've decided to make the filter list external. If you need to add a program to it, copy its lower ID to the end of filter.txt
in your JKSV folder as 0xXXXXXXXX.

A big thanks to everyone who has worked on ctrulib.