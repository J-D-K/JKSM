.PHONY: all FsLib JKSM clean

all: FsLib JKSM

FsLib:
	$(MAKE) -C FsLib/3DS/FsLib

JKSM: FsLib
	$(MAKE) -C JKSM

clean:
	$(MAKE) -C FsLib/3DS/FsLib clean
	$(MAKE) -C JKSM clean
