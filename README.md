# KatchFoundry - 0.22rx
The C++ rewrite of the game KatchFoundry.

This is only the source code of releases of foundry. The source code is only released for specific versions.

(MAKE SURE TO FOLLOW THE EULA TO PROCEED)

The music content has been removed from this source release, and to use the assets make sure to get proper permission from Katch Studios, and make sure to give proper credit to specified artist.

(To compile this you must have raylib)

## Build Scripts:
The main way of building Foundry is with the Makefile.

On all platforms make sure to install raylib, gcc, and make

On Mac/Linux just use run the command `make run` while in the foundry dir

On Windows do the same but make sure to run it in git bash or something similar.

On BSD operating systems it should work the same as on Mac/Linux but it has not been tested

### NPP Script:
```
SET RAYLIB_PATH=D:\raylib\raylib
SET COMPILER_PATH=D:\raylib\w64devkit\bin
ENV_SET PATH=$(COMPILER_PATH)
SET CC=g++

SET CFLAGS=$(RAYLIB_PATH)\src\raylib.rc.data -s -static -O2 -Wall -I$(RAYLIB_PATH)\src -Iexternal -DPLATFORM_DESKTOP
SET LDFLAGS=-lraylib -llua54 -lopengl32 -lgdi32 -lwinmm -pthread -L$(LUA_LIB)

cd $(CURRENT_DIRECTORY)
cmd /c IF EXIST $(NAME_PART).exe del /F $(NAME_PART).exe
npp_save
$(CC) --version
$(CC) -o $(NAME_PART).exe $(FILE_NAME) $(CFLAGS) $(LDFLAGS)
ENV_UNSET PATH
```
