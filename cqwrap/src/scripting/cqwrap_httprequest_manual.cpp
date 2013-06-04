#include "cqwrap_register_all_manual.h"
#include "../util/JsonHelper.h"
#include "../pattern/EventProxy.h"

#include "cocos-ext.h"

USING_NS_CC_EXT;

void js_register_cocos2dx_extension_httprequest(JSContext *cx, JSObject *global);
JSClass  *jsb_HttpRequest_Class;
JSObject *jsb_HttpRequest_prototype;

void register_cqwrap_httprequest(JSContext* cx, JSObject* obj) {
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

	js_register_cocos2dx_extension_httprequest(cx, obj);
}

class HttpRequest: public CCObject{
protected:
	CCHttpRequest* m_request;
	std::vector<std::string> m_headers;

	void responseCallback(cocos2d::CCNode *sender, void *data){
		CCHttpResponse *response = (CCHttpResponse*)data; 

		if (!response) 
		{ 
			CCLog("no response...");
			return; 
		} 

		int statusCode = response->getResponseCode(); 
		char statusString[64] = {}; 
		sprintf(statusString, "HTTP Status Code: %d", statusCode); 
		
		CCLOG("response code: %d", statusCode); 

		if (!response->isSucceed())  
		{ 
			CCLOG("response failed"); 
			CCLOG("error buffer: %s", response->getErrorBuffer()); 

			JsonData* msg = new JsonData();
			(*msg)["data"] = response->getErrorBuffer();
			PROXY_FIRE("error", msg);
			CC_SAFE_DELETE(msg);

			return; 
		}

		// dump data 
		std::vector<char> *buffer = response->getResponseData(); 
		//std::string path = CCFileUtils::sharedFileUtils()->getWriteablePath(); 
		std::string buf(buffer->begin(),buffer->end()); 

		JsonData* msg = new JsonData();
		(*msg)["data"] = buf.c_str();
		PROXY_FIRE("complete", msg);
		CC_SAFE_DELETE(msg);
	};
public:
	void send(const char* buffer = NULL){
		if(m_request != NULL){
			if(NULL != buffer){
				m_request->setRequestData(buffer, sizeof buffer);
			}
			m_request->setHeaders(m_headers);
			CCHttpClient::getInstance()->send(m_request);
			CC_SAFE_RELEASE_NULL(m_request);
			m_headers.clear();
		}
	};
	void setRequestHeader(std::string key, std::string content){
		if(m_request != NULL){
			key += ": ";
			key += content;
			m_headers.push_back(key);
		}
	};
	void open(CCHttpRequest::HttpRequestType type, const char* url){

		CC_SAFE_RELEASE_NULL(m_request);
		m_request = new CCHttpRequest(); 

		m_request->setUrl(url); 
		m_request->setRequestType(type); 
		m_request->setResponseCallback(this, callfuncND_selector(HttpRequest::responseCallback));
	};
	HttpRequest(){
		m_request = NULL;
		m_headers = std::vector<std::string>();
	};
	~HttpRequest(){
		CC_SAFE_RELEASE_NULL(m_request);
	};
};

JSBool js_cocos2dx_extension_HttpRequest_setRequestHeader(JSContext *cx, uint32_t argc, jsval *vp){
	jsval *argv = JS_ARGV(cx, vp);
	JSObject *obj = JS_THIS_OBJECT(cx, vp);
	js_proxy_t *proxy; JS_GET_NATIVE_PROXY(proxy, obj);
	HttpRequest* cobj = (HttpRequest *)(proxy ? proxy->ptr : NULL);
	JSB_PRECONDITION2( cobj, cx, JS_FALSE, "Invalid Native Object");

	if(argc == 2){
		std::string* key = new std::string();

		do {
			JSBool ok = jsval_to_std_string(cx, argv[0], key);
			JSB_PRECONDITION2( ok, cx, JS_FALSE, "Error processing arguments");
		} while (0);

		std::string* value = new std::string();

		do {
			JSBool ok = jsval_to_std_string(cx, argv[1], value);
			JSB_PRECONDITION2( ok, cx, JS_FALSE, "Error processing arguments");
		} while (0);

		cobj->setRequestHeader(*key, *value);

		JS_SET_RVAL(cx, vp, JSVAL_VOID);

		CC_SAFE_DELETE(key);
		CC_SAFE_DELETE(value);

		return JS_TRUE;	
	}
	JS_ReportError(cx, "wrong number of arguments: %d, was expecting %d", argc, 1);
	return JS_FALSE;
}

JSBool js_cocos2dx_extension_HttpRequest_open(JSContext *cx, uint32_t argc, jsval *vp){
	jsval *argv = JS_ARGV(cx, vp);
	JSObject *obj = JS_THIS_OBJECT(cx, vp);
	js_proxy_t *proxy; JS_GET_NATIVE_PROXY(proxy, obj);
	HttpRequest* cobj = (HttpRequest *)(proxy ? proxy->ptr : NULL);
	JSB_PRECONDITION2( cobj, cx, JS_FALSE, "Invalid Native Object");

	if(argc == 2){
		std::string* method = new std::string();
		
		do {
			JSBool ok = jsval_to_std_string(cx, argv[0], method);
			JSB_PRECONDITION2( ok, cx, JS_FALSE, "Error processing arguments");
		} while (0);

		std::string* url = new std::string();

		do {
			JSBool ok = jsval_to_std_string(cx, argv[1], url);
			JSB_PRECONDITION2( ok, cx, JS_FALSE, "Error processing arguments");
		} while (0);

		if(*method == "POST"){
			cobj->open(CCHttpRequest::kHttpPost, url->c_str());
		}else{
			cobj->open(CCHttpRequest::kHttpGet, url->c_str());
		}
		JS_SET_RVAL(cx, vp, JSVAL_VOID);
		
		CC_SAFE_DELETE(url);
		CC_SAFE_DELETE(method);

		return JS_TRUE;	
	}
	JS_ReportError(cx, "wrong number of arguments: %d, was expecting %d", argc, 1);
	return JS_FALSE;
}

JSBool js_cocos2dx_extension_HttpRequest_send(JSContext *cx, uint32_t argc, jsval *vp){
	jsval *argv = JS_ARGV(cx, vp);
	JSObject *obj = JS_THIS_OBJECT(cx, vp);
	js_proxy_t *proxy; JS_GET_NATIVE_PROXY(proxy, obj);
	HttpRequest* cobj = (HttpRequest *)(proxy ? proxy->ptr : NULL);
	JSB_PRECONDITION2( cobj, cx, JS_FALSE, "Invalid Native Object");

	if(argc == 1){
		std::string* data = new std::string();
		do {
			JSBool ok = jsval_to_std_string(cx, argv[0], data);
			JSB_PRECONDITION2( ok, cx, JS_FALSE, "Error processing arguments");
		} while (0);

		cobj->send(data->c_str());
		JS_SET_RVAL(cx, vp, JSVAL_VOID);
		CC_SAFE_DELETE(data);
		return JS_TRUE;	
	}
	if(argc == 0){
		cobj->send();
		JS_SET_RVAL(cx, vp, JSVAL_VOID);
		return JS_TRUE;	
	}
	JS_ReportError(cx, "wrong number of arguments: %d, was expecting %d", argc, 1);
	return JS_FALSE;
}

JSBool js_cocos2dx_extension_HttpRequest_oncomplete(JSContext *cx, uint32_t argc, jsval *vp){
	JS_SET_RVAL(cx, vp, JSVAL_VOID);
	return JS_FALSE;
}

JSBool js_cocos2dx_extension_HttpRequest_onerror(JSContext *cx, uint32_t argc, jsval *vp){
	JS_SET_RVAL(cx, vp, JSVAL_VOID);
	return JS_TRUE;
}

void js_cocos2dx_extension_HttpRequest_finalize(JSFreeOp *fop, JSObject *obj){

}

JSBool js_cocos2dx_extension_HttpRequest_constructor(JSContext *cx, uint32_t argc, jsval *vp){
	if(argc == 0){
		HttpRequest* cobj = new HttpRequest(); 
		cocos2d::CCObject *_ccobj = dynamic_cast<cocos2d::CCObject *>(cobj);
		if (_ccobj) {
			_ccobj->autorelease();
		}

		TypeTest<cocos2d::extension::CCHttpRequest> t;
		js_type_class_t *typeClass;
		uint32_t typeId = t.s_id();
		HASH_FIND_INT(_js_global_type_ht, &typeId, typeClass);
		assert(typeClass);
		JSObject *obj = JS_NewObject(cx, typeClass->jsclass, typeClass->proto, typeClass->parentProto);

		JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));
		// link the native object with the javascript object
		js_proxy_t *p;
		JS_NEW_PROXY(p, cobj, obj);
		JS_AddNamedObjectRoot(cx, &p->obj, "HttpRequest");

		return JS_TRUE;
	}
	JS_ReportError(cx, "wrong number of arguments: %d, was expecting %d", argc, 0);
	return JS_FALSE;
}

void js_register_cocos2dx_extension_httprequest(JSContext *cx, JSObject *global) {
	jsb_HttpRequest_Class = (JSClass *)calloc(1, sizeof(JSClass));
	jsb_HttpRequest_Class->name = "HttpRequest";
	jsb_HttpRequest_Class->addProperty = JS_PropertyStub;
	jsb_HttpRequest_Class->delProperty = JS_PropertyStub;
	jsb_HttpRequest_Class->getProperty = JS_PropertyStub;
	jsb_HttpRequest_Class->setProperty = JS_StrictPropertyStub;
	jsb_HttpRequest_Class->enumerate = JS_EnumerateStub;
	jsb_HttpRequest_Class->resolve = JS_ResolveStub;
	jsb_HttpRequest_Class->convert = JS_ConvertStub;
	jsb_HttpRequest_Class->finalize = js_cocos2dx_extension_HttpRequest_finalize;
	jsb_HttpRequest_Class->flags = JSCLASS_HAS_RESERVED_SLOTS(2);

	static JSPropertySpec properties[] = {
		{0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
	};

	static JSFunctionSpec funcs[] = {
		JS_FN("oncomplete",js_cocos2dx_extension_HttpRequest_oncomplete, 1, JSPROP_PERMANENT | JSPROP_ENUMERATE),
		JS_FN("onerror",js_cocos2dx_extension_HttpRequest_onerror, 1, JSPROP_PERMANENT | JSPROP_ENUMERATE),
		JS_FN("send",js_cocos2dx_extension_HttpRequest_send, 0, JSPROP_PERMANENT | JSPROP_ENUMERATE),
		JS_FN("open",js_cocos2dx_extension_HttpRequest_open, 2, JSPROP_PERMANENT | JSPROP_ENUMERATE),
		JS_FN("setRequestHeader",js_cocos2dx_extension_HttpRequest_setRequestHeader, 2, JSPROP_PERMANENT | JSPROP_ENUMERATE),
		JS_FS_END
	};

	jsb_HttpRequest_prototype = JS_InitClass(
		cx, global,
		jsb_HttpRequest_prototype,
		jsb_HttpRequest_Class,
		js_cocos2dx_extension_HttpRequest_constructor, 0, // constructor
		properties,
		funcs,
		NULL, // no static properties
		NULL);

	// make the class enumerable in the registered namespace
	JSBool found;
	JS_SetPropertyAttributes(cx, global, "HttpRequest", JSPROP_ENUMERATE | JSPROP_READONLY, &found);

	// add the proto and JSClass to the type->js info hash table
	TypeTest<cocos2d::extension::CCHttpRequest> t;
	js_type_class_t *p;
	uint32_t typeId = t.s_id();
	HASH_FIND_INT(_js_global_type_ht, &typeId, p);
	if (!p) {
		p = (js_type_class_t *)malloc(sizeof(js_type_class_t));
		p->type = typeId;
		p->jsclass = jsb_HttpRequest_Class;
		p->proto = jsb_HttpRequest_prototype;
		p->parentProto = NULL;
		HASH_ADD_INT(_js_global_type_ht, type, p);
	}
}
