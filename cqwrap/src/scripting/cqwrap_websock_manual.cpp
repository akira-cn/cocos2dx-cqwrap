#include "cqwrap_register_all_manual.h"
#include "../util/WebSocket.h"
#include "../util/JsonHelper.h"
/*
[Constructor(in DOMString url, in optional DOMString protocols)]
[Constructor(in DOMString url, in optional DOMString[] protocols)]
interface WebSocket {
	readonly attribute DOMString url;

	// ready state
	const unsigned short CONNECTING = 0;
	const unsigned short OPEN = 1;
	const unsigned short CLOSING = 2;
	const unsigned short CLOSED = 3;
	readonly attribute unsigned short readyState;
	readonly attribute unsigned long bufferedAmount;

	// networking
	attribute Function onopen;
	attribute Function onmessage;
	attribute Function onerror;
	attribute Function onclose;
	readonly attribute DOMString protocol;
	void send(in DOMString data);
	void close();
};
WebSocket implements EventTarget;
*/

#include "cocos-ext.h"

USING_NS_CC_EXT;

void js_register_cocos2dx_extension_websocket(JSContext *cx, JSObject *global);
JSClass  *jsb_WebSocket_Class;
JSObject *jsb_WebSocket_prototype;

void register_cqwrap_websocket(JSContext* cx, JSObject* obj) {
	// first, try to get the ns
	jsval nsval;
	JSObject *ns;
	JS_GetProperty(cx, obj, "cc", &nsval);
	if (nsval == JSVAL_VOID) {
		ns = JS_NewObject(cx, NULL, NULL, NULL);
		nsval = OBJECT_TO_JSVAL(ns);
		JS_SetProperty(cx, obj, "cc", &nsval);
	} else {
		JS_ValueToObject(cx, nsval, &ns);
	}
	obj = ns;

	js_register_cocos2dx_extension_websocket(cx, obj);

	ScriptingCore::getInstance()->evalString("cc.WebSocket.__defineGetter__('CONNECTING', function(){return 0}); cc.WebSocket.__defineGetter__('OPEN', function(){return 1}); cc.WebSocket.__defineGetter__('CLOSING', function(){return 2}); cc.WebSocket.__defineGetter__('CLOSED', function(){return 3}); cc.WebSocket.prototype.__defineGetter__('readyState', function(){return this.__private_readystate()}); if(typeof WebSocket == 'undefined') WebSocket=cc.WebSocket;", NULL);
}

//TODO
JSBool js_cocos2dx_extension_WebSocket_send(JSContext *cx, uint32_t argc, jsval *vp){
	jsval *argv = JS_ARGV(cx, vp);
	JSObject *obj = JS_THIS_OBJECT(cx, vp);
	js_proxy_t *proxy; JS_GET_NATIVE_PROXY(proxy, obj);
	WebSocket* cobj = (WebSocket *)(proxy ? proxy->ptr : NULL);
	JSB_PRECONDITION2( cobj, cx, JS_FALSE, "Invalid Native Object");

	if(argc == 1){
		std::string* msg = new std::string();
		do {
			JSBool ok = jsval_to_std_string(cx, argv[0], msg);
			JSB_PRECONDITION2( ok, cx, JS_FALSE, "Error processing arguments");
		} while (0);
		cobj->send(JSON::parse(*msg));
		JS_SET_RVAL(cx, vp, JSVAL_VOID);
		CC_SAFE_DELETE(msg);
		return JS_TRUE;
	}
	JS_ReportError(cx, "wrong number of arguments: %d, was expecting %d", argc, 0);
	return JS_TRUE;
}

JSBool js_cocos2dx_extension_WebSocket_close(JSContext *cx, uint32_t argc, jsval *vp){
	jsval *argv = JS_ARGV(cx, vp);
	JSObject *obj = JS_THIS_OBJECT(cx, vp);
	js_proxy_t *proxy; JS_GET_NATIVE_PROXY(proxy, obj);
	WebSocket* cobj = (WebSocket *)(proxy ? proxy->ptr : NULL);
	JSB_PRECONDITION2( cobj, cx, JS_FALSE, "Invalid Native Object");

	if(argc == 0){
		cobj->close();
		JS_SET_RVAL(cx, vp, JSVAL_VOID);
		return JS_TRUE;
	}
	JS_ReportError(cx, "wrong number of arguments: %d, was expecting %d", argc, 0);
	return JS_FALSE;
}

JSBool js_cocos2dx_extension_WebSocket_readystate(JSContext *cx, uint32_t argc, jsval *vp){
	jsval *argv = JS_ARGV(cx, vp);
	JSObject *obj = JS_THIS_OBJECT(cx, vp);
	js_proxy_t *proxy; JS_GET_NATIVE_PROXY(proxy, obj);
	WebSocket* cobj = (WebSocket *)(proxy ? proxy->ptr : NULL);
	JSB_PRECONDITION2( cobj, cx, JS_FALSE, "Invalid Native Object");

	if(argc == 0){
		JS_SET_RVAL(cx, vp, UINT_TO_JSVAL(cobj->readyState));
		return JS_TRUE;
	}
	JS_ReportError(cx, "wrong number of arguments: %d, was expecting %d", argc, 0);
	return JS_FALSE;
}

JSBool js_cocos2dx_extension_WebSocket_onopen(JSContext *cx, uint32_t argc, jsval *vp){
	JS_SET_RVAL(cx, vp, JSVAL_VOID);
	return JS_FALSE;
}

JSBool js_cocos2dx_extension_WebSocket_onclose(JSContext *cx, uint32_t argc, jsval *vp){
	JS_SET_RVAL(cx, vp, JSVAL_VOID);
	return JS_TRUE;
}

JSBool js_cocos2dx_extension_WebSocket_onerror(JSContext *cx, uint32_t argc, jsval *vp){
	JS_SET_RVAL(cx, vp, JSVAL_VOID);
	return JS_TRUE;
}

JSBool js_cocos2dx_extension_WebSocket_onmessage(JSContext *cx, uint32_t argc, jsval *vp){
	JS_SET_RVAL(cx, vp, JSVAL_VOID);
	return JS_TRUE;
}

void js_cocos2dx_extension_WebSocket_finalize(JSFreeOp *fop, JSObject *obj){

}

/*
	new cc.WebSocket('ws://host:port/path');		
	TODO: support sub-protocols
 */
JSBool js_cocos2dx_extension_WebSocket_constructor(JSContext *cx, uint32_t argc, jsval *vp)
{
	jsval *argv = JS_ARGV(cx, vp);
	if (argc == 1 || argc == 2) {
		std::string* url = new std::string();
		int pos;
		do {
			JSBool ok = jsval_to_std_string(cx, argv[0], url);
			JSB_PRECONDITION2( ok, cx, JS_FALSE, "Error processing arguments");
		} while (0);
		//ws://
		std::string host = *url;

		pos = host.find("ws://");
		if(pos == 0){
			host.erase(0,5);
		}

		int port = 80;
		pos = host.find(":");
		if(pos >= 0){
			port = atoi(host.substr(pos+1, host.size()).c_str());
		}

		pos = host.find("/", pos);
		std::string path = "/";
		if(pos >= 0){
			path += host.substr(pos + 1, host.size());
		}

		pos = host.find(":");
		if(pos >= 0){
			host.erase(pos, host.size());
		}
		
		WebSocket * cobj = new WebSocket(host.c_str(), port, path.c_str());
		cocos2d::CCObject *_ccobj = dynamic_cast<cocos2d::CCObject *>(cobj);
		/*if (_ccobj) {
			_ccobj->autorelease();
		}*/

		TypeTest<cocos2d::extension::WebSocket> t;
		js_type_class_t *typeClass;
		uint32_t typeId = t.s_id();
		HASH_FIND_INT(_js_global_type_ht, &typeId, typeClass);
		assert(typeClass);
		JSObject *obj = JS_NewObject(cx, typeClass->jsclass, typeClass->proto, typeClass->parentProto);

		JS_DefineProperty(cx, obj, "URL", argv[0]
			, NULL, NULL, JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY);

		//protocol not support yet (always return "")
		JS_DefineProperty(cx, obj, "protocol", c_string_to_jsval(cx, "")
			, NULL, NULL, JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY);	

		JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));
		// link the native object with the javascript object
		js_proxy_t *p;
		JS_NEW_PROXY(p, cobj, obj);
		JS_AddNamedObjectRoot(cx, &p->obj, "WebSocket");

		CC_SAFE_DELETE(url);
		return JS_TRUE;
	}


	JS_ReportError(cx, "wrong number of arguments: %d, was expecting %d", argc, 1);
	return JS_FALSE;
}

void js_register_cocos2dx_extension_websocket(JSContext *cx, JSObject *global) {
	jsb_WebSocket_Class = (JSClass *)calloc(1, sizeof(JSClass));
	jsb_WebSocket_Class->name = "WebSocket";
	jsb_WebSocket_Class->addProperty = JS_PropertyStub;
	jsb_WebSocket_Class->delProperty = JS_PropertyStub;
	jsb_WebSocket_Class->getProperty = JS_PropertyStub;
	jsb_WebSocket_Class->setProperty = JS_StrictPropertyStub;
	jsb_WebSocket_Class->enumerate = JS_EnumerateStub;
	jsb_WebSocket_Class->resolve = JS_ResolveStub;
	jsb_WebSocket_Class->convert = JS_ConvertStub;
	jsb_WebSocket_Class->finalize = js_cocos2dx_extension_WebSocket_finalize;
	jsb_WebSocket_Class->flags = JSCLASS_HAS_RESERVED_SLOTS(2);

	static JSPropertySpec properties[] = {
		{0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
	};

	static JSFunctionSpec funcs[] = {
		JS_FN("onopen",js_cocos2dx_extension_WebSocket_onopen, 0, JSPROP_PERMANENT | JSPROP_ENUMERATE),
		JS_FN("onclose",js_cocos2dx_extension_WebSocket_onclose, 0, JSPROP_PERMANENT | JSPROP_ENUMERATE),
		JS_FN("onerror",js_cocos2dx_extension_WebSocket_onerror, 0, JSPROP_PERMANENT | JSPROP_ENUMERATE),
		JS_FN("onmessage",js_cocos2dx_extension_WebSocket_onmessage, 1, JSPROP_PERMANENT | JSPROP_ENUMERATE),
		JS_FN("send",js_cocos2dx_extension_WebSocket_send, 1, JSPROP_PERMANENT | JSPROP_ENUMERATE),
		JS_FN("close",js_cocos2dx_extension_WebSocket_close, 0, JSPROP_PERMANENT | JSPROP_ENUMERATE),
		JS_FN("__private_readystate",js_cocos2dx_extension_WebSocket_readystate, 0, JSPROP_PERMANENT),
		JS_FS_END
	};

	jsb_WebSocket_prototype = JS_InitClass(
		cx, global,
		jsb_WebSocket_prototype,
		jsb_WebSocket_Class,
		js_cocos2dx_extension_WebSocket_constructor, 0, // constructor
		properties,
		funcs,
		NULL, // no static properties
		NULL);

	JS_DefineProperty(cx, jsb_WebSocket_prototype, "CONNECTING", INT_TO_JSVAL((int)WebSocket::CONNECTING)
		, NULL, NULL, JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY);
	JS_DefineProperty(cx, jsb_WebSocket_prototype, "OPEN", INT_TO_JSVAL((int)WebSocket::OPEN)
		, NULL, NULL, JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY);
	JS_DefineProperty(cx, jsb_WebSocket_prototype, "CLOSING", INT_TO_JSVAL((int)WebSocket::CLOSING)
		, NULL, NULL, JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY);
	JS_DefineProperty(cx, jsb_WebSocket_prototype, "CLOSED", INT_TO_JSVAL((int)WebSocket::CLOSED)
		, NULL, NULL, JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY);

	// make the class enumerable in the registered namespace
	JSBool found;
	JS_SetPropertyAttributes(cx, global, "WebSocket", JSPROP_ENUMERATE | JSPROP_READONLY, &found);

	// add the proto and JSClass to the type->js info hash table
	TypeTest<cocos2d::extension::WebSocket> t;
	js_type_class_t *p;
	uint32_t typeId = t.s_id();
	HASH_FIND_INT(_js_global_type_ht, &typeId, p);
	if (!p) {
		p = (js_type_class_t *)malloc(sizeof(js_type_class_t));
		p->type = typeId;
		p->jsclass = jsb_WebSocket_Class;
		p->proto = jsb_WebSocket_prototype;
		p->parentProto = NULL;
		HASH_ADD_INT(_js_global_type_ht, type, p);
	}

	
}