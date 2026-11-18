# Rocks'n'Diamonds for MS-DOS / FreeDOS (DJGPP, 32-bit DPMI)
#
#   make            rnd.exe with CWSDPMI embedded, plus the game's data next
#                   to it: this folder is then a working game
#   make dist       build/dist/RND: the game folder, ready for a DOS disk
#   make clean
#
# The game is the normal Rocks'n'Diamonds source (../src), an SDL2 program.
# Underneath:
#
#   SDL3/          SDL3 with its DOS port (VESA/VGA, Sound Blaster, keyboard,
#                  mouse, gameport), as in SDLPoPDOS. Its SDL_* functions are
#                  renamed S3_SDL_* in build/libSDL3.a ...
#   deps/sdl2-compat  ... so that sdl2-compat (the SDL2 API on SDL3) can sit
#                  on top of it in the same program: build/libSDL2.a.
#   deps/SDL_image (2.8)  PNG, PCX, BMP, XPM and the rest through stb_image.
#   deps/SDL_mixer (2.8)  WAV, MOD/XM/S3M/IT (libxmp-lite), OGG (stb_vorbis),
#                  MP3 (minimp3), FLAC (dr_flac).
#   deps/SDL_net (2.2)    on Watt-32 (deps/watt32): TCP/IP over a packet
#                  driver (vmdos: VMPKT), for network games.
#   deps/zlib      for level sets in .zip files.
#   cwsdpmi/       CWSDPMI r7, the DPMI host put in front of the EXE.
#
# Needs GNU make and Perl (the game generates some of its source), and a DJGPP
# cross compiler: `sh build-djgpp.sh` builds one (gcc 14.2.0, into ~/djgpp,
# found there), or any on the PATH or DJGPP=dir (i586-pc-msdosdjgpp-gcc;
# prebuilt: https://github.com/andrewwutw/build-djgpp/releases); or Docker,
# which gets the djfdyuruiry/djgpp image.

# The compiler: a local DJGPP cross compiler when there is one (on the PATH,
# DJGPP=dir, or where build-djgpp.sh puts it: ~/djgpp, /opt/djgpp,
# /usr/local/djgpp), otherwise the djfdyuruiry/djgpp Docker image. Force
# either with DOCKER=1 / DOCKER=0.
DJGPP_IMAGE = djfdyuruiry/djgpp

USER_ID  = $(shell id -u)
GROUP_ID = $(shell id -g)

CROSS ?= i586-pc-msdosdjgpp-
ifndef DJGPP
ifeq ($(shell command -v $(CROSS)gcc 2>/dev/null),)
DJGPP := $(firstword $(foreach d,$(HOME)/djgpp /opt/djgpp /usr/local/djgpp,$(if $(wildcard $(d)/bin/$(CROSS)gcc),$(d))))
endif
endif
ifdef DJGPP
export PATH := $(DJGPP)/bin:$(PATH)
endif
ifeq ($(origin DOCKER),undefined)
DOCKER := $(if $(shell PATH="$(PATH)" command -v $(CROSS)gcc 2>/dev/null),0,1)
endif

ifeq ($(DOCKER),1)
# The whole Rocks'n'Diamonds tree is mounted, so ../src and the data are
# reachable. Inside the image the DJGPP tools have no prefix; make runs there
# with DOCKER=0 CROSS=.
RUN      = docker run --rm -v $(abspath ..):/src:z -w /src/msdos -u $(USER_ID):$(GROUP_ID) $(DJGPP_IMAGE) /bin/sh -c
else
CC_DOS   = $(CROSS)gcc
AR_DOS   = $(CROSS)ar
OBJCOPY  = $(CROSS)objcopy
NM       = $(CROSS)nm
EXE2COFF = $(shell PATH="$(PATH)" command -v $(CROSS)exe2coff 2>/dev/null || echo $(dir $(shell PATH="$(PATH)" command -v $(CROSS)gcc))../i586-pc-msdosdjgpp/bin/exe2coff)
endif

SRC     := ../src
SDL     := SDL3
DEPS    := deps
BUILD   := build

TARGET  = rnd.exe

# Headers that the libraries' own build steps would make (and that their
# .gitignore files keep out of the repository): made here first, before
# anything that includes them, so parallel builds from a clean checkout work.
GEN_HDRS = $(DEPS)/zlib/zconf.h $(DEPS)/watt32/inc/sys/djgpp.err

$(DEPS)/zlib/zconf.h: $(DEPS)/zlib/zconf.h.in
	cp $< $@

$(DEPS)/watt32/inc/sys/djgpp.err: $(DEPS)/watt32-djgpp.err
	cp $< $@

all: msdos

OPT = -O2 -march=i486 -mtune=i586
# make DEBUG=1: frame pointers (DJGPP's crash traceback) and symbols kept
DEBUG ?= 0
ifeq ($(DEBUG),1)
OPT += -fno-omit-frame-pointer -g
STRIP =
BUILD := build-debug
else
STRIP = -s
endif

# ---------------------------------------------------------------- SDL3
SDL_CFLAGS = $(OPT) -DNDEBUG -fno-strict-aliasing -w \
             -DSDL_BUILD_MAJOR_VERSION=3 -DSDL_BUILD_MINOR_VERSION=5 -DSDL_BUILD_MICRO_VERSION=0 \
             -DSDL_STATIC_LIB -DUSING_GENERATED_CONFIG_H \
             -Isdl3-config/build_config -Isdl3-config -I$(SDL)/include -I$(SDL)/src \
             -idirafter $(SDL)/src/video/khronos

include sdl3-srcs.mk

SDL_OBJS = $(addprefix $(BUILD)/sdl3/,$(SDL_SRCS:.c=.o))

$(BUILD)/sdl3/%.o: $(SDL)/%.c
	@mkdir -p $(dir $@)
	@echo CC $<
	@$(CC_DOS) $(SDL_CFLAGS) -MMD -MP -c $< -o $@

# Every SDL_* symbol SDL3 defines becomes S3_SDL_* (the SDL2 API takes the
# plain names); the objects refer to each other by the new names too.
$(BUILD)/libSDL3.a: $(SDL_OBJS)
	rm -f $@ $@.syms
	$(NM) -g --defined-only $(SDL_OBJS) | awk '$$3 ~ /^_SDL_/ { print $$3 " _S3" $$3 }' | sort -u > $@.syms
	mkdir -p $(BUILD)/sdl3-renamed
	for o in $(SDL_OBJS); do \
	  n=$(BUILD)/sdl3-renamed/$$(echo $$o | sed 's|$(BUILD)/sdl3/||; s|/|_|g'); \
	  $(OBJCOPY) --redefine-syms=$@.syms $$o $$n || exit 1; \
	done
	$(AR_DOS) rcs $@ $(BUILD)/sdl3-renamed/*.o

# ---------------------------------------------------------------- sdl2-compat
S2C      := $(DEPS)/sdl2-compat
SDL2_INC := -I$(S2C)/include/SDL2 -I$(S2C)/include
S2C_CFLAGS = $(OPT) -fno-strict-aliasing -w -I$(SDL)/include $(SDL2_INC) -I$(S2C)/src

$(BUILD)/s2c/sdl2_compat.o: $(S2C)/src/sdl2_compat.c
	@mkdir -p $(dir $@)
	$(CC_DOS) $(S2C_CFLAGS) -c $< -o $@

# The table sdl2-compat looks SDL3's functions up in (instead of dlsym()).
$(BUILD)/s2c/sdl3_dos_table.o: src/sdl3_dos_table.c $(S2C)/src/sdl3_syms.h
	@mkdir -p $(dir $@)
	$(CC_DOS) $(OPT) -w -I$(SDL)/include -I$(S2C)/src -c $< -o $@

$(BUILD)/libSDL2.a: $(BUILD)/s2c/sdl2_compat.o $(BUILD)/s2c/sdl3_dos_table.o
	rm -f $@
	$(AR_DOS) rcs $@ $^

# ---------------------------------------------------------------- SDL2_image
IMG := $(DEPS)/SDL_image
IMG_SRCS = IMG.c IMG_bmp.c IMG_gif.c IMG_jpg.c IMG_lbm.c IMG_pcx.c IMG_png.c IMG_pnm.c \
           IMG_qoi.c IMG_stb.c IMG_tga.c IMG_xcf.c IMG_xpm.c IMG_xv.c IMG_svg.c \
           IMG_avif.c IMG_jxl.c IMG_tif.c IMG_webp.c IMG_WIC.c
IMG_CFLAGS = $(OPT) -w $(SDL2_INC) -I$(IMG)/include -DUSE_STBIMAGE \
             -DLOAD_BMP -DLOAD_GIF -DLOAD_JPG -DLOAD_LBM -DLOAD_PCX -DLOAD_PNG -DLOAD_PNM \
             -DLOAD_QOI -DLOAD_TGA -DLOAD_XCF -DLOAD_XPM -DLOAD_XV -DLOAD_SVG -DSDL_IMAGE_SAVE_PNG=0 -DSDL_IMAGE_SAVE_JPG=0
IMG_OBJS = $(addprefix $(BUILD)/img/,$(IMG_SRCS:.c=.o))

$(BUILD)/img/%.o: $(IMG)/src/%.c
	@mkdir -p $(dir $@)
	@echo CC $<
	@$(CC_DOS) $(IMG_CFLAGS) -c $< -o $@

$(BUILD)/libSDL2_image.a: $(IMG_OBJS)
	rm -f $@
	$(AR_DOS) rcs $@ $^

# ---------------------------------------------------------------- libxmp-lite
XMP := $(DEPS)/libxmp
XMP_SRCS = $(notdir $(wildcard $(XMP)/src/lite/lite-*.c))
XMP_CFLAGS = $(OPT) -w -DLIBXMP_CORE_PLAYER -DLIBXMP_STATIC -DBUILDING_STATIC -DHAVE_FNMATCH=0 \
             -I$(XMP)/include -I$(XMP)/src
XMP_OBJS = $(addprefix $(BUILD)/xmp/,$(XMP_SRCS:.c=.o))

$(BUILD)/xmp/%.o: $(XMP)/src/lite/%.c
	@mkdir -p $(dir $@)
	@echo CC $<
	@$(CC_DOS) $(XMP_CFLAGS) -c $< -o $@

$(BUILD)/libxmp-lite.a: $(XMP_OBJS)
	rm -f $@
	$(AR_DOS) rcs $@ $^

# ---------------------------------------------------------------- SDL2_mixer
MIX := $(DEPS)/SDL_mixer
MIX_SRCS = effect_position.c effect_stereoreverse.c effects_internal.c mixer.c music.c utils.c \
           codecs/load_aiff.c codecs/load_voc.c codecs/mp3utils.c codecs/music_wav.c \
           codecs/music_ogg.c codecs/music_ogg_stb.c codecs/music_minimp3.c \
           codecs/music_drflac.c codecs/music_xmp.c codecs/remap_channels.c
MIX_CFLAGS = $(OPT) -w $(SDL2_INC) -I$(MIX)/include -I$(MIX)/src -I$(MIX)/src/codecs \
             -DMUSIC_WAV -DMUSIC_OGG -DOGG_USE_STB -DMUSIC_MP3_MINIMP3 -DMUSIC_FLAC_DRFLAC \
             -DMUSIC_MOD_XMP -DLIBXMP_STATIC -I$(XMP)/include
MIX_OBJS = $(addprefix $(BUILD)/mix/,$(MIX_SRCS:.c=.o))

$(BUILD)/mix/%.o: $(MIX)/src/%.c
	@mkdir -p $(dir $@)
	@echo CC $<
	@$(CC_DOS) $(MIX_CFLAGS) -c $< -o $@

$(BUILD)/libSDL2_mixer.a: $(MIX_OBJS)
	rm -f $@
	$(AR_DOS) rcs $@ $^

# ---------------------------------------------------------------- Watt-32
WATT := $(DEPS)/watt32
$(BUILD)/libwatt.a:
	cd $(WATT)/src && WATT_ROOT=$(abspath $(WATT)) sh configur.sh djgpp > /dev/null
	cp $(WATT)/../watt32-djgpp.err $(WATT)/inc/sys/djgpp.err
	cp $(WATT)/../watt32-syserr.c $(WATT)/src/build/djgpp/syserr.c
	$(MAKE) -C $(WATT)/src -f djgpp.mak WATT_ROOT=$(abspath $(WATT)) BIN_PREFIX=$(CROSS)
	cp $(WATT)/lib/libwatt.a $@

# ---------------------------------------------------------------- SDL2_net
NET := $(DEPS)/SDL_net
NET_SRCS = SDLnet.c SDLnetTCP.c SDLnetUDP.c SDLnetselect.c
NET_CFLAGS = $(OPT) -w $(SDL2_INC) -I$(WATT)/inc -I$(NET)
NET_OBJS = $(addprefix $(BUILD)/net/,$(NET_SRCS:.c=.o))

$(BUILD)/net/%.o: $(NET)/%.c | $(GEN_HDRS)
	@mkdir -p $(dir $@)
	@echo CC $<
	@$(CC_DOS) $(NET_CFLAGS) -c $< -o $@

$(BUILD)/libSDL2_net.a: $(NET_OBJS)
	rm -f $@
	$(AR_DOS) rcs $@ $^

# ---------------------------------------------------------------- zlib
ZLIB := $(DEPS)/zlib
ZLIB_SRCS = adler32.c compress.c crc32.c deflate.c gzclose.c gzlib.c gzread.c gzwrite.c \
            infback.c inffast.c inflate.c inftrees.c trees.c uncompr.c zutil.c
ZLIB_OBJS = $(addprefix $(BUILD)/zlib/,$(ZLIB_SRCS:.c=.o))

$(BUILD)/zlib/%.o: $(ZLIB)/%.c | $(GEN_HDRS)
	@mkdir -p $(dir $@)
	@echo CC $<
	@$(CC_DOS) $(OPT) -w -c $< -o $@

$(BUILD)/libz.a: $(ZLIB_OBJS)
	rm -f $@
	$(AR_DOS) rcs $@ $^

# (zlib before Watt-32, which carries a copy of zlib's inflate)
LIBS = $(BUILD)/libSDL2_image.a $(BUILD)/libSDL2_mixer.a $(BUILD)/libxmp-lite.a $(BUILD)/libz.a \
       $(BUILD)/libSDL2_net.a $(BUILD)/libwatt.a \
       $(BUILD)/libSDL2.a $(BUILD)/libSDL3.a

ifneq ($(DOCKER),1)
libs: $(LIBS)
endif

# ---------------------------------------------------------------- the game
GAME_DIRS = $(SRC) $(SRC)/libgame $(SRC)/libgame/zip $(SRC)/game_bd $(SRC)/game_em \
            $(SRC)/game_mm $(SRC)/game_sp
# (conf_*.c other than these four are generated tables the others #include)
GAME_SRCS = $(filter-out $(SRC)/libgame/zip/iowin32.c $(filter-out $(addprefix $(SRC)/,conf_gfx.c conf_snd.c conf_mus.c conf_hlp.c),$(wildcard $(SRC)/conf_*.c)), \
              $(foreach d,$(GAME_DIRS),$(wildcard $(d)/*.c)))
GAME_OBJS = $(patsubst $(SRC)/%.c,$(BUILD)/game/%.o,$(GAME_SRCS)) $(BUILD)/game/dos_main.o

GAME_CFLAGS = $(OPT) -Wall -Wno-unused -Wno-format-truncation -Wno-stringop-truncation \
              -DTARGET_SDL2 -DPLATFORM_MSDOS $(SDL2_INC) -I$(IMG)/include -I$(MIX)/include -I$(NET) \
              -I$(ZLIB) -I$(SRC) -I$(SRC)/libgame -Dmain=rnd_main

$(BUILD)/game/%.o: $(SRC)/%.c | $(GEN_HDRS)
	@mkdir -p $(dir $@)
	@echo CC $<
	@$(CC_DOS) $(GAME_CFLAGS) -MMD -MP -c $< -o $@

-include $(GAME_OBJS:.o=.d)

$(BUILD)/game/dos_main.o: src/dos_main.c
	@mkdir -p $(dir $@)
	$(CC_DOS) $(OPT) -Wall -I$(SDL)/include -c $< -o $@

# ../src/conf_*.[ch], conftime.h and confhash.h are generated by the game's
# own Makefile (a Perl script and the date / git commit).
$(SRC)/conf_gfx.h:
	$(MAKE) -C $(SRC) TARGET=sdl2 auto-conf conf-time conf-hash

$(GAME_OBJS): $(SRC)/conf_gfx.h

ifeq ($(DOCKER),1)
pull-djgpp:
	docker pull $(DJGPP_IMAGE)

# Everything that needs the compiler happens in the container. The generated
# game source (Perl, git) is made here first.
$(BUILD)/rnd-go32.exe $(BUILD)/rnd.coff libs: FORCE $(SRC)/conf_gfx.h | pull-djgpp
	$(RUN) "make DOCKER=0 CROSS= CC_DOS=gcc AR_DOS=ar OBJCOPY=objcopy NM=nm EXE2COFF=exe2coff DEBUG=$(DEBUG) $(if $(filter libs,$@),libs,$@)"

FORCE:
.PHONY: pull-djgpp FORCE
else
$(BUILD)/rnd-go32.exe: $(GAME_OBJS) $(LIBS)
	$(CC_DOS) $(STRIP) -o $@ $(GAME_OBJS) $(LIBS) -lm

$(BUILD)/rnd.coff: $(BUILD)/rnd-go32.exe
	cp $< $(BUILD)/rnd.exe
	cd $(BUILD) && $(EXE2COFF) rnd.exe && mv rnd rnd.coff && rm rnd.exe
endif

# CWSDPMI's stub in front: one file that runs on plain DOS.
$(TARGET): $(BUILD)/rnd.coff
	cat cwsdpmi/bin/CWSDSTUB.EXE $< > $@
	chmod +x $@

msdos: $(TARGET) runtime-files

# What rnd.exe reads at run time, next to it, so this folder is a working game.
runtime-files:
	rm -rf graphics sounds music levels conf docs
	cp -r ../graphics ../sounds ../music ../levels ../conf ../docs .
	cp cwsdpmi/bin/CWSDPMI.EXE .

# ---------------------------------------------------------------- packaging
dist: msdos
	rm -rf $(BUILD)/dist && mkdir -p $(BUILD)/dist/RND
	cp $(TARGET) $(BUILD)/dist/RND/RND.EXE
	cp -r ../graphics ../sounds ../music ../levels ../conf ../docs $(BUILD)/dist/RND/
	cp WATTCP.CFG dosbox-x.conf $(BUILD)/dist/RND/
	sed 's/$$/\r/' README.md > $(BUILD)/dist/RND/README.TXT
	sed 's/$$/\r/' ../COPYING > $(BUILD)/dist/RND/COPYING.TXT
	@echo "Created $(BUILD)/dist/RND"

clean:
	rm -rf $(BUILD) $(TARGET) CWSDPMI.EXE graphics sounds music levels conf docs $(GEN_HDRS)

.PHONY: all msdos libs runtime-files dist clean
