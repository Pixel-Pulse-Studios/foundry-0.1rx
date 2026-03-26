# Foundry
### Foundry 0.1rx is the alpha version of the C++ rewrite of the game Foundry.

(MAKE SURE TO FOLLOW THE EULA TO PROCEED)

The music content has been removed from this source release, and to use the assets make sure to get proper permission from Katch Studios, and make sure to give proper credit to specified artist.

(To compile this you must have raylib)

## Build Scripts:
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
