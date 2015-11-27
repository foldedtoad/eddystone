#------------------------------------------------------------------------------
#  gen_dat.mk
#------------------------------------------------------------------------------

ifeq ($(OS),Windows_NT)
    EXT = ".exe"
else
    UNAME_S := $(shell uname -s)

    ifeq ($(UNAME_S),Linux)
        EXT = ""
    endif
    ifeq ($(UNAME_S),Darwin)
        EXT = ".osx"
    endif
endif

gen_dat$(EXT):
	gcc gen_dat.c -o gen_dat$(EXT)
