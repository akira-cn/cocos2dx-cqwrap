LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CPPFLAGS+= -fexceptions

LOCAL_MODULE := libjson_static

LOCAL_MODULE_FILENAME := libjson

LOCAL_SRC_FILES :=  \
	./src/lib_json/json_reader.cpp \
	./src/lib_json/json_value.cpp \
	./src/lib_json/json_writer.cpp 

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include/json  \
	$(LOCAL_PATH)/src/lib_json

include $(BUILD_STATIC_LIBRARY)
