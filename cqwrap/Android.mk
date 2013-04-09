LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CPPFLAGS+= -fexceptions

LOCAL_MODULE := cqwrap_static

LOCAL_MODULE_FILENAME := libcqwrap

LOCAL_SRC_FILES :=  \
	./src/pattern/CustEvent.cpp \
	./src/scripting/js_bind_websocket.cpp \
	./src/third_party/json/src/lib_json/json_reader.cpp \
	./src/third_party/json/src/lib_json/json_value.cpp \
	./src/third_party/json/src/lib_json/json_writer.cpp \
	./src/third_party/websockets/lib/base64-decode.c \
	./src/third_party/websockets/lib/client-handshake.c \
	./src/third_party/websockets/lib/client-parser.c \
	./src/third_party/websockets/lib/client.c \
	./src/third_party/websockets/lib/daemonize.c \
	./src/third_party/websockets/lib/extension-deflate-frame.c \
	./src/third_party/websockets/lib/extension-deflate-stream.c \
	./src/third_party/websockets/lib/extension.c \
	./src/third_party/websockets/lib/getifaddrs.c \
	./src/third_party/websockets/lib/handshake.c \
	./src/third_party/websockets/lib/libwebsockets.c \
	./src/third_party/websockets/lib/output.c \
	./src/third_party/websockets/lib/parsers.c \
	./src/third_party/websockets/lib/server-handshake.c \
	./src/third_party/websockets/lib/server.c \
	./src/util/JsonHelper.cpp \
	./src/util/WebSocket.cpp 

LOCAL_WHOLE_STATIC_LIBRARIES := cocos2dx_static 
LOCAL_WHOLE_STATIC_LIBRARIES += cocosdenshion_static 
LOCAL_WHOLE_STATIC_LIBRARIES += cocos_extension_static 
LOCAL_WHOLE_STATIC_LIBRARIES += chipmunk_static 
LOCAL_WHOLE_STATIC_LIBRARIES += spidermonkey_static 
LOCAL_WHOLE_STATIC_LIBRARIES += scriptingcore-spidermonkey

LOCAL_C_INCLUDES := $(LOCAL_PATH)/src \
	$(LOCAL_PATH)/src/third_party/json/include \
	$(LOCAL_PATH)/src/third_party/json/include/json \
	$(LOCAL_PATH)/src/third_party/websockets/lib	

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/src \
	$(LOCAL_PATH)/src/third_party/json/include \
	$(LOCAL_PATH)/src/third_party/websockets/lib	

LOCAL_EXPORT_CFLAGS += -DCOCOS2D_JAVASCRIPT

include $(BUILD_STATIC_LIBRARY)

$(call import-module,cocos2dx)
$(call import-module,CocosDenshion/android)
$(call import-module,extensions)
$(call import-module,external/chipmunk)
$(call import-module,scripting/javascript/spidermonkey-android)
$(call import-module,scripting/javascript/bindings)
