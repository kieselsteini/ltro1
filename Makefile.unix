CC=cc -O2 -Wall -Wextra -I./lua/src -DLUA_USE_POSIX `sdl2-config --cflags`
LIB=`sdl2-config --libs`
OBJ_LUA=./lua/src/lapi.o ./lua/src/lauxlib.o ./lua/src/lbaselib.o ./lua/src/lcode.o ./lua/src/lcorolib.o ./lua/src/lctype.o ./lua/src/ldblib.o ./lua/src/ldebug.o ./lua/src/ldo.o ./lua/src/ldump.o ./lua/src/lfunc.o ./lua/src/lgc.o ./lua/src/linit.o ./lua/src/liolib.o ./lua/src/llex.o ./lua/src/lmathlib.o ./lua/src/lmem.o ./lua/src/loadlib.o ./lua/src/lobject.o ./lua/src/lopcodes.o ./lua/src/loslib.o ./lua/src/lparser.o ./lua/src/lstate.o ./lua/src/lstring.o ./lua/src/lstrlib.o ./lua/src/ltable.o ./lua/src/ltablib.o ./lua/src/ltm.o ./lua/src/lundump.o ./lua/src/lutf8lib.o ./lua/src/lvm.o ./lua/src/lzio.o
OBJ_SRC=./src/ltro1.o
OBJ=$(OBJ_LUA) $(OBJ_SRC)
BIN=ltro1

default: $(OBJ)
	$(CC) -o $(BIN) $(LIB) $(OBJ)

clean:
	rm -f $(BIN) $(OBJ)

