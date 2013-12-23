all:
	mkdir -p build/temp.macosx-10.6-universal-2.6
	mkdir -p build/lib.macosx-10.6-universal-2.6
	gcc-4.2 -fno-strict-aliasing -fno-common -dynamic -DNDEBUG -g -fwrapv -Os -Wall -Wstrict-prototypes -DENABLE_DTRACE -pipe -I. -I/System/Library/Frameworks/Python.framework/Versions/2.6/include/python2.6 -c PicoScope.c -o build/temp.macosx-10.6-universal-2.6/PicoScope.o -std=c99
	gcc-4.2 -fno-strict-aliasing -fno-common -dynamic -DNDEBUG -g -fwrapv -Os -Wall -Wstrict-prototypes -DENABLE_DTRACE -pipe -I. -I/System/Library/Frameworks/Python.framework/Versions/2.6/include/python2.6 -c libPicoScope.c -o build/temp.macosx-10.6-universal-2.6/libPicoScope.o -std=c99
	gcc-4.2 -fno-strict-aliasing -fno-common -dynamic -DNDEBUG -g -fwrapv -Os -Wall -Wstrict-prototypes -DENABLE_DTRACE -pipe -I. -I/System/Library/Frameworks/Python.framework/Versions/2.6/include/python2.6 -c libPicoScope-data.c -o build/temp.macosx-10.6-universal-2.6/libPicoScope-data.o -std=c99
	gcc-4.2 -Wl,-F. -undefined dynamic_lookup build/temp.macosx-10.6-universal-2.6/PicoScope.o build/temp.macosx-10.6-universal-2.6/libPicoScope.o build/temp.macosx-10.6-universal-2.6/libPicoScope-data.o -lusb-1.0 -o build/lib.macosx-10.6-universal-2.6/PicoScope.so

