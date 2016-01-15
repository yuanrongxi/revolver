LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE     := revolver
LOCAL_CFLAGS     := -D__STDC_LIMIT_MACROS \
                    -D__STDC_CONSTANT_MACROS \
                    -Wall -O3 -fexceptions
LOCAL_C_INCLUDES := ../common/

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -lz -lm -llog 

LOCAL_SRC_FILES :=  $(wildcard ../common/rudp/*.c) \
	$(wildcard ../common/rudp/*.cpp) \
	$(wildcard ../common/core/*.c) \
	$(wildcard ../common/core/*.cpp) \
	$(wildcard ../common/revolver/*.c) \
	$(wildcard ../common/revolver/*.cpp)

include $(BUILD_STATIC_LIBRARY)

