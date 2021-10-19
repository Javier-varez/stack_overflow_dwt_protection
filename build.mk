LOCAL_DIR := $(call current-dir)

include $(CLEAR_VARS)
LOCAL_NAME := stack_overflow_app
LOCAL_CFLAGS := \
    -mcpu=cortex-m3 \
    -mfloat-abi=soft \
    -mthumb \
    -Os \
    -g3 \
    -I$(LOCAL_DIR)/inc \
    -DSTM32F1 \
    -Wall \
    -Werror \
    -Wextra \
    -Wno-gnu-string-literal-operator-template \
    -Wno-infinite-recursion
LOCAL_CXXFLAGS := \
    $(LOCAL_CFLAGS) \
    -std=gnu++20 \
    -fno-exceptions \
    -fno-rtti \
    -ffunction-sections \
    -fdata-sections
LOCAL_LDFLAGS := \
    -Wl,--gc-sections \
    #-lnosys
LOCAL_LINKER_FILE := \
    $(LOCAL_DIR)/memory.ld
LOCAL_SRC := \
    $(LOCAL_DIR)/src/postform_config.cpp \
    $(LOCAL_DIR)/src/main.cpp
LOCAL_ARM_ARCHITECTURE := v7-m
LOCAL_ARM_FPU := nofp
LOCAL_COMPILER := arm_clang
LOCAL_STATIC_LIBS := \
    libcortex_m_startup \
    libcortex_m_hal \
    libpostform \
    libditto
include $(BUILD_BINARY)
