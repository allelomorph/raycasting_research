# -*- MakeFile -*-
# monoplanar_demo

# $(pkg-config --cflags --libs sdl2) == `-D_REENTRANT -I/usr/include/SDL2 -lSDL2`

# implicit compilation rules:  (see https://web.mit.edu/gnu/doc/html/make_10.html)
# n.o: n.c
#	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o n.o
#
# n.o: n.cc
#	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -o n.o

CC           = gcc
CXX          = g++
RM           = rm -f

INCLUDE_DIR  = include/
UTILS_DIR    = ../utils/
HEADER_DIRS  = $(INCLUDE_DIR) $(UTILS_DIR) /usr/include/SDL2

CPPFLAGS     = -Wall -Werror -Wextra -pedantic
# errnoname uses designated initializers, which requires C99+ and is not
#   compatible with the C++ standard
CFLAGS       = -std=c99
CXXFLAGS     = -std=c++11 -D_REENTRANT $(addprefix -I, $(HEADER_DIRS))

LDLIBS      = $(addprefix -l, \
	SDL2 \
	SDL2_ttf \
	SDL2_image )

SRC_HEADERS = $(addprefix $(INCLUDE_DIR), \
	App.hh \
	DdaRaycastEngine.hh \
	FpsCalc.hh \
	KbdInputMgr.hh \
	Layout.hh \
	LinuxKbdInputMgr.hh \
	TtyDisplayMgr.hh \
	TtyScreenBuffer.hh \
	sdl_unique_ptrs.hh \
	SdlKbdInputMgr.hh )
#   SdlDisplayMgr.hh_ \
#   DisplayMgr.hh_ \
#   SdlScreenBuffer.hh_
HDR_ONLY_SRC = $(addprefix $(INCLUDE_DIR), \
	KeyState.hh \
	Matrix.hh \
	Settings.hh )
#   DisplayMgr.hh_
UTIL_HEADERS = $(addprefix $(UTILS_DIR), \
	errnoname.h \
	Xterm.hh )
HEADERS  = $(SRC_HEADERS) $(HDR_ONLY_SRC) $(UTIL_HEADERS)

SRC_DIR      = src/
SRC          = $(addprefix $(SRC_DIR), \
	App.cc \
	DdaRaycastEngine.cc \
	FpsCalc.cc \
	KbdInputMgr.cc \
	Layout.cc \
	LinuxKbdInputMgr.cc \
	sdl_unique_ptrs.cc \
	SdlKbdInputMgr.cc \
	TtyDisplayMgr.cc \
	TtyScreenBuffer.cc ) \
		$(addprefix $(UTILS_DIR), \
	Xterm.cc ) \
	main.cc
#   utils/errnoname.c

OBJ = $(UTILS_DIR)errnoname.o $(SRC:.cc=.o)


.PHONY: all clean oclean fclean re

monoplanar_demo: $(HEADERS) $(OBJ)
	$(CXX) -o $@ $(OBJ) $(LDLIBS)

all: monoplanar_demo

clean: $(RM) *~ monoplanar_demo

oclean: $(RM) $(OBJ)

fclean: clean oclean

re: oclean all
