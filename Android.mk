LOCAL_PATH := $(call my-dir)/source

include $(CLEAR_VARS)

LOCAL_MODULE    := eduke32

LOCAL_C_INCLUDES := \
$(LOCAL_PATH)/glad/include \
$(LOCAL_PATH)/build/include \
$(LOCAL_PATH)/audiolib/include \
$(LOCAL_PATH)/mact/include \
$(LOCAL_PATH)/duke3d/src \
$(LOCAL_PATH)/mimalloc/include \
$(LOCAL_PATH)/libxmp-lite/include \
$(SDL_INCLUDE_PATHS) \
$(TOP_DIR)/Clibs_OpenTouch \
$(TOP_DIR)/Clibs_OpenTouch/raze \
$(TOP_DIR)/MobileTouchControls  \
$(TOP_DIR)/AudioLibs_OpenTouch/liboggvorbis/include  \


LOCAL_CFLAGS := -Wno-ignored-attributes -DRENDERTYPESDL=1 -DUSE_OPENGL -DHAVE_XMP -DHAVE_VORBIS -DHAVE_FLAC -DENGINE_NAME=\"eduke32\" -DEDUKE32
LOCAL_CPPFLAGS := -fexceptions


engine_objs := \
    build/src/2d.cpp \
    build/src/baselayer.cpp \
    build/src/cache1d.cpp \
    build/src/clip.cpp \
    build/src/colmatch.cpp \
    build/src/common.cpp \
    build/src/communityapi.cpp \
    build/src/compat.cpp \
    build/src/cpuid.cpp \
    build/src/crc32.cpp \
    build/src/defs.cpp \
    build/src/dxtfilter.cpp \
    build/src/enet.cpp \
    build/src/engine.cpp \
    build/src/fix16.cpp \
    build/src/hash.cpp \
    build/src/hightile.cpp \
    build/src/klzw.cpp \
    build/src/kplib.cpp \
    build/src/lz4.c \
    build/src/md4.cpp \
    build/src/mhk.cpp \
    build/src/miniz.c \
    build/src/miniz_tdef.c \
    build/src/miniz_tinfl.c \
    build/src/mmulti.cpp \
    build/src/mutex.cpp \
    build/src/osd.cpp \
    build/src/palette.cpp \
    build/src/pngwrite.cpp \
    build/src/polymost.cpp \
    build/src/polymost1Frag.glsl \
    build/src/polymost1Vert.glsl \
    build/src/pragmas.cpp \
    build/src/rev.cpp \
    build/src/screenshot.cpp \
    build/src/screentext.cpp \
    build/src/scriptfile.cpp \
    build/src/sjson.cpp \
    build/src/smalltextfont.cpp \
    build/src/softsurface.cpp \
    build/src/texcache.cpp \
    build/src/textfont.cpp \
    build/src/tiles.cpp \
    build/src/timer.cpp \
    build/src/vfs.cpp \
    build/src/xxhash.c \
    build/src/glsurface.cpp \
    build/src/glbuild.cpp \
    build/src/voxmodel.cpp \
    build/src/mdsprite.cpp \
    build/src/tilepacker.cpp \
    build/src/a-c.cpp \
    build/src/sdlayer.cpp \
    build/src/smmalloc.cpp \
    build/src/smmalloc_tls.cpp \
    build/src/smmalloc_generic.cpp


glad_objs := \
    glad/src/glad.c \

mact_objs := \
    mact/src/animlib.cpp \
    mact/src/control.cpp \
    mact/src/joystick.cpp \
    mact/src/keyboard.cpp \
    mact/src/scriplib.cpp \


audiolib_objs := \
    audiolib/src/driver_adlib.cpp \
    audiolib/src/driver_sf2.cpp \
    audiolib/src/drivers.cpp \
    audiolib/src/flac.cpp \
    audiolib/src/formats.cpp \
    audiolib/src/fx_man.cpp \
    audiolib/src/gmtimbre.cpp \
    audiolib/src/midi.cpp \
    audiolib/src/mix.cpp \
    audiolib/src/mixst.cpp \
    audiolib/src/multivoc.cpp \
    audiolib/src/music.cpp \
    audiolib/src/opl3.cpp \
    audiolib/src/pitch.cpp \
    audiolib/src/vorbis.cpp \
    audiolib/src/xa.cpp \
    audiolib/src/xmp.cpp \
    audiolib/src/driver_sdl.cpp\


duke3d_game_objs := \
    duke3d/src/actors.cpp \
    duke3d/src/anim.cpp \
    duke3d/src/cheats.cpp \
    duke3d/src/cmdline.cpp \
    duke3d/src/common.cpp \
    duke3d/src/config.cpp \
    duke3d/src/demo.cpp \
    duke3d/src/game.cpp \
    duke3d/src/gamedef.cpp \
    duke3d/src/gameexec.cpp \
    duke3d/src/gamestructures.cpp \
    duke3d/src/gamevars.cpp \
    duke3d/src/global.cpp \
    duke3d/src/grpscan.cpp \
    duke3d/src/input.cpp \
    duke3d/src/menus.cpp \
    duke3d/src/network.cpp \
    duke3d/src/osdcmds.cpp \
    duke3d/src/osdfuncs.cpp \
    duke3d/src/player.cpp \
    duke3d/src/premap.cpp \
    duke3d/src/rts.cpp \
    duke3d/src/savegame.cpp \
    duke3d/src/sbar.cpp \
    duke3d/src/screens.cpp \
    duke3d/src/sector.cpp \
    duke3d/src/sounds.cpp \
    duke3d/src/text.cpp \
    duke3d/src/dnames.cpp \


mimalloc_objs := \
    mimalloc/src/alloc.c \
    mimalloc/src/alloc-aligned.c \
    mimalloc/src/alloc-posix.c \
    mimalloc/src/arena.c \
    mimalloc/src/bitmap.c \
    mimalloc/src/heap.c \
    mimalloc/src/init.c \
    mimalloc/src/options.c \
    mimalloc/src/os.c \
    mimalloc/src/page.c \
    mimalloc/src/random.c \
    mimalloc/src/region.c \
    mimalloc/src/segment.c \
    mimalloc/src/stats.c \


ANDROID_SRC_FILES = \
     android/game_interface.cpp \
     ../../Clibs_OpenTouch/raze/touch_interface.cpp \
     ../../Clibs_OpenTouch/touch_interface_base.cpp \
     ../../Clibs_OpenTouch/android_jni_inc.cpp \

LOCAL_SRC_FILES =  $(ANDROID_SRC_FILES) $(engine_objs) $(glad_objs) $(mact_objs) $(audiolib_objs) $(duke3d_common_editor_objs) $(duke3d_game_objs)#$(mimalloc_objs)

LOCAL_LDLIBS :=  -llog -lOpenSLES

LOCAL_STATIC_LIBRARIES := logwritter flac xmp
LOCAL_SHARED_LIBRARIES := touchcontrols openal SDL2 SDL2_mixer SDL2_image saffal


include $(BUILD_SHARED_LIBRARY)
