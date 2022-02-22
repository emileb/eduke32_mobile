LOCAL_PATH := $(call my-dir)/source/libxmp-lite/src/

include $(CLEAR_VARS)

LOCAL_MODULE := xmp

LOCAL_C_INCLUDES := \
$(LOCAL_PATH)/../include/libxmp-lite


LOCAL_CFLAGS := -DHAVE_ROUND -DLIBXMP_CORE_PLAYER -DLIBXMP_NO_PROWIZARD -DLIBXMP_NO_DEPACKERS -DBUILDING_STATIC  -Wno-unused-parameter -Wno-sign-compare


libxmplite_objs := \
    common.c \
    control.c \
    dataio.c \
    effects.c \
    filter.c \
    format.c \
    hio.c \
    it_load.c \
    itsex.c \
    lfo.c \
    load.c \
    load_helpers.c \
    memio.c \
    mix_all.c \
    mixer.c \
    mod_load.c \
    mtm_load.c \
    period.c \
    player.c \
    read_event.c \
    s3m_load.c \
    sample.c \
    scan.c \
    smix.c \
    virtual.c \
    win32.c \
    xm_load.c \


LOCAL_SRC_FILES =  $(libxmplite_objs)


include $(BUILD_STATIC_LIBRARY)
