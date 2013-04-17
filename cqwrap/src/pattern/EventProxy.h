#ifndef __Pattern_EventProxy_H__
#define __Pattern_EventProxy_H__

#include "cocos-ext.h"
#include "util/JsonHelper.h"

#include "ScriptingCore.h"
#include "generated/jsb_cocos2dx_auto.hpp"
#include "cocos2d_specifics.hpp"

#define PROXY_FIRE(type, evt) \
do{ \
	js_proxy_t * p;	\
	void* ptr = (void*)this; \
	JS_GET_PROXY(p, ptr); \
	if(p){ \
		JSContext* cx = ScriptingCore::getInstance()->getGlobalContext(); \
		JSObject* obj = ScriptingCore::getInstance()->getGlobalObject(); \
\
		jsval dataVal = c_string_to_jsval(cx, JSON::stringify(evt).c_str()); \
\
		/*convert to js-json*/ \
		jsval nsval; \
		JS_GetProperty(cx, obj, "JSON", &nsval); \
		ScriptingCore::getInstance()->executeFunctionWithOwner(nsval, "parse", 1, &dataVal, &dataVal);\
\
		/*fire the event*/ \
		std::string handler = std::string("on"); \
		handler+=type; \
		jsval retval; \
		ScriptingCore::getInstance()->executeFunctionWithOwner(OBJECT_TO_JSVAL(p->obj), handler.c_str(), 1, &dataVal, &retval); \
	} \
}while(0) \

#endif
