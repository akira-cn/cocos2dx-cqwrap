LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CPPFLAGS+= -fexceptions

LOCAL_MODULE := libwebsockets_shared

LOCAL_MODULE_FILENAME := libwebsockets

LOCAL_SRC_FILES :=  \
	../src/third_party/websockets/lib/base64-decode.c \
	../src/third_party/websockets/lib/client-handshake.c \
	../src/third_party/websockets/lib/client-parser.c \
	../src/third_party/websockets/lib/client.c \
	../src/third_party/websockets/lib/daemonize.c \
	../src/third_party/websockets/lib/extension-deflate-frame.c \
	../src/third_party/websockets/lib/extension-deflate-stream.c \
	../src/third_party/websockets/lib/extension.c \
	../src/third_party/websockets/lib/getifaddrs.c \
	../src/third_party/websockets/lib/handshake.c \
	../src/third_party/websockets/lib/libwebsockets.c \
	../src/third_party/websockets/lib/output.c \
	../src/third_party/websockets/lib/parsers.c \
	../src/third_party/websockets/lib/server-handshake.c \
	../src/third_party/websockets/lib/server.c \
	../src/third_party/websockets/lib/sha-1.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../src \
	$(LOCAL_PATH)/../src/third_party/json/include \
	$(LOCAL_PATH)/../src/third_party/json/include/json \
	$(LOCAL_PATH)/../src/third_party/websockets/lib	

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../src \
	$(LOCAL_PATH)/../src/third_party/json/include \
	$(LOCAL_PATH)/../src/third_party/websockets/lib	

LOCAL_LDLIBS := -lz

include $(BUILD_SHARED_LIBRARY)
