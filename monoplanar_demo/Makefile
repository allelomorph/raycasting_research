# -*- MakeFile -*-
# monoplanar_demo

# $(pkg-config --cflags --libs sdl2) == `-D_REENTRANT -I/usr/include/SDL2 -lSDL2`

# for implicit compilation rules, see:
#   - https://www.gnu.org/software/make/manual/html_node/Catalogue-of-Rules.html
# n.o: n.c
#	$(CC) -c $(CPPFLAGS) $(CFLAGS) n.c (-o $@)
#
# n.o: n.cc
#	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) n.cc (-o $@)

#
# file path construction
#
SRC_DIR      := src/
INCLUDE_DIR  := include/
UTILS_DIR    := ../utils/
HEADER_DIRS  := $(INCLUDE_DIR) $(UTILS_DIR) /usr/include/SDL2

# (MP for "monoplanar")
MP_FILE_STEMS := \
	App \
	DdaRaycastEngine \
	FpsCalc \
	KbdInputMgr \
	Layout \
	LinuxKbdInputMgr \
	TtyDisplayMgr \
	TtyScreenBuffer \
	sdl_unique_ptrs \
	SdlKbdInputMgr
#   SdlDisplayMgr_ \
#   DisplayMgr_ \
#   SdlScreenBuffer_

PATHLESS_MP_SRC         := $(addsuffix .cc, $(MP_FILE_STEMS))
PATHLESS_MP_SRC_HEADERS := $(addsuffix .hh, $(MP_FILE_STEMS))
PATHLESS_MP_HEADER_ONLY := \
	KeyState.hh \
	Matrix.hh \
	Settings.hh
#   DisplayMgr.hh_

MP_SRC         := $(addprefix $(SRC_DIR), $(PATHLESS_MP_SRC))
MP_SRC_HEADERS := $(addprefix $(INCLUDE_DIR), $(PATHLESS_MP_SRC_HEADERS))
MP_HEADER_ONLY := $(addprefix $(INCLUDE_DIR), $(PATHLESS_MP_HEADER_ONLY))

UTIL_HEADERS   := $(addprefix $(UTILS_DIR), \
	errnoname.h \
	Xterm.hh )
HEADERS        := $(MP_SRC_HEADERS) $(MP_HEADER_ONLY) $(UTIL_HEADERS)

SRC            := $(MP_SRC) $(UTILS_DIR)Xterm.cc main.cc $(UTILS_DIR)errnoname.c

#
# compilation flags usable in implicit rules
#
CC           := gcc
CXX          := g++
CPPFLAGS     := -Wall -Werror -Wextra -pedantic
# errnoname.c uses designated initializers, which requires C99+ and is not
#   compatible with the C++ standard
CFLAGS       := -std=c99
CXXFLAGS     := -std=c++11 -D_REENTRANT $(addprefix -I, $(HEADER_DIRS))

#
# custom compilation flags
#
PROF_FLAG    := -pg
DEBUG_FLAG   := -ggdb

#
# linker flags
#
LDLIBS := $(addprefix -l, \
	SDL2 \
	SDL2_ttf \
	SDL2_image )

#
# shell commands
#
RM           := rm -f

#
# rules without recipes
#
.PHONY: \
	clean oclean fclean re \
	gprof_clean gprof_oclean gprof_fclean gprof_re \
	gdb_clean gdb_oclean gdb_fclean gdb_re \
	all all_clean all_oclean all_fclean all_re

#
# monoplanar_demo
#
OBJ := $(SRC:.cc=.o) $(UTILS_DIR)errnoname.o

# default rule with `make` (first rule without '.' prefix)
# all OBJ targets built with implcit rules
monoplanar_demo: $(HEADERS) $(OBJ)
	$(CXX) -o $@ $(OBJ) $(LDLIBS)

clean:
	$(RM) *~ monoplanar_demo

oclean:
	$(RM) $(OBJ)

fclean: clean oclean

re: oclean monoplanar_demo

#
# monoplanar_demo_gprof
#
$(UTILS_DIR)errnoname_gprof.o: $(UTILS_DIR)errnoname.c $(UTILS_DIR)errnoname.h
	$(CC) -c $(PROF_FLAG) $(CPPFLAGS) $(CFLAGS) $(UTILS_DIR)errnoname.c -o $@

$(UTILS_DIR)Xterm_gprof.o: $(UTILS_DIR)Xterm.cc $(UTILS_DIR)Xterm.hh
	$(CXX) -c $(PROF_FLAG) $(CPPFLAGS) $(CXXFLAGS) $(UTILS_DIR)Xterm.cc -o $@

$(MP_SRC:.cc=_gprof.o): $(subst _gprof.o,.cc,$@) \
			$(subst $(SRC_DIR),$(INCLUDE_DIR),$(subst _gprof.o,.hh,$@))
	$(CXX) -c $(PROF_FLAG) $(CPPFLAGS) $(CXXFLAGS) $(subst _gprof.o,.cc,$@) -o $@

main_gprof.o: main.cc
	$(CXX) -c $(PROF_FLAG) $(CPPFLAGS) $(CXXFLAGS) main.cc -o $@

GPROF_OBJ := \
	$(UTILS_DIR)errnoname_gprof.o \
	$(UTILS_DIR)Xterm_gprof.o \
	$(MP_SRC:.cc=_gprof.o) \
	main_gprof.o

monoplanar_demo_gprof: $(HEADERS) $(GPROF_OBJ)
	$(CXX) $(PROF_FLAG) -o $@ $(GPROF_OBJ) $(LDLIBS)

gprof_clean:
	$(RM) *~ monoplanar_demo_gprof

gprof_oclean:
	$(RM) $(GPROF_OBJ)

gprof_fclean: gprof_clean gprof_oclean

gprof_re: gprof_oclean monoplanar_demo_gprof

#
# monoplanar_demo_gdb
#
$(UTILS_DIR)errnoname_gdb.o: $(UTILS_DIR)errnoname.c $(UTILS_DIR)errnoname.h
	$(CC) -c $(DEBUG_FLAG) $(CPPFLAGS) $(CFLAGS) $(UTILS_DIR)errnoname.c -o $@

$(UTILS_DIR)Xterm_gdb.o: $(UTILS_DIR)Xterm.cc $(UTILS_DIR)Xterm.hh
	$(CXX) -c $(DEBUG_FLAG) $(CPPFLAGS) $(CXXFLAGS) $(UTILS_DIR)Xterm.cc -o $@

$(MP_SRC:.cc=_gdb.o): $(subst _gdb.o,.cc,$@) \
			$(subst $(SRC_DIR),$(INCLUDE_DIR),$(subst _gdb.o,.hh,$@))
	$(CXX) -c $(DEBUG_FLAG) $(CPPFLAGS) $(CXXFLAGS) $(subst _gdb.o,.cc,$@) -o $@

main_gdb.o: main.cc
	$(CXX) -c $(DEBUG_FLAG) $(CPPFLAGS) $(CXXFLAGS) main.cc -o $@

GDB_OBJ := \
	$(UTILS_DIR)errnoname_gdb.o \
	$(UTILS_DIR)Xterm_gdb.o \
	$(MP_SRC:.cc=_gdb.o) \
	main_gdb.o

monoplanar_demo_gdb: $(HEADERS) $(GDB_OBJ)
	$(CXX) $(DEBUG_FLAG) -o $@ $(GDB_OBJ) $(LDLIBS)

gdb_clean:
	$(RM) *~ monoplanar_demo_gdb

gdb_oclean:
	$(RM) $(GDB_OBJ)

gdb_fclean: gdb_clean gdb_oclean

gdb_re: gdb_oclean monoplanar_demo_gdb

#
# monoplanar_demo*
#
all: monoplanar_demo monoplanar_demo_gprof monoplanar_demo_gdb

all_clean:
	$(RM) *~ monoplanar_demo monoplanar_demo_gprof monoplanar_demo_gdb

all_oclean:
	$(RM) $(OBJ) $(GPROF_OBJ) $(GDB_OBJ)

all_fclean: all_clean all_oclean

all_re: all_oclean all
