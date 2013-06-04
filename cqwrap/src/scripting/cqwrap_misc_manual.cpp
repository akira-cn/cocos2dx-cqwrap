#include "cqwrap_register_all_manual.h"
#include "../GUI/CCWebView.h"
#include "../util/MessageDelegate.h"

static JSBool js_cocos2dx_alert(JSContext *cx, uint32_t argc, jsval *vp)
{
	jsval *argv = JS_ARGV(cx, vp);

	if (argc == 1) {
		std::string* message = new std::string();

		do {
			JSBool ok = jsval_to_std_string(cx, argv[0], message);
			JSB_PRECONDITION2( ok, cx, JS_FALSE, "Error processing arguments");
		} while (0);

		CCMessageBox(message->c_str(), "");

		CC_SAFE_DELETE(message);

		JS_SET_RVAL(cx, vp, JSVAL_VOID);
		return JS_TRUE;
	}
	JS_ReportError(cx, "wrong number of arguments: %d, was expecting %d", argc, 1);
	return JS_FALSE;
}

static JSBool js_cocos2dx_open(JSContext *cx, uint32_t argc, jsval *vp)
{
	jsval *argv = JS_ARGV(cx, vp);

	if (argc == 1) {
		std::string* url = new std::string();

		do {
			JSBool ok = jsval_to_std_string(cx, argv[0], url);
			JSB_PRECONDITION2( ok, cx, JS_FALSE, "Error processing arguments");
		} while (0);

		extension::CCWebView::getInstance()->open(url->c_str());

		CC_SAFE_DELETE(url);

		JS_SET_RVAL(cx, vp, JSVAL_VOID);
		return JS_TRUE;
	}
	JS_ReportError(cx, "wrong number of arguments: %d, was expecting %d", argc, 1);
	return JS_FALSE;
}

static JSBool js_cocos2dx_close(JSContext *cx, uint32_t argc, jsval *vp)
{
	jsval *argv = JS_ARGV(cx, vp);

	if (argc == 0) {
		extension::CCWebView::getInstance()->close();
		JS_SET_RVAL(cx, vp, JSVAL_VOID);
		return JS_TRUE;
	}
	JS_ReportError(cx, "wrong number of arguments: %d, was expecting %d", argc, 0);
	return JS_FALSE;
}

static JSBool js_cocos2dx_postMessage(JSContext *cx, uint32_t argc, jsval *vp)
{
	jsval *argv = JS_ARGV(cx, vp);
	if (argc == 1) {
		std::string* msg = new std::string();

		do {
			JSBool ok = jsval_to_std_string(cx, argv[0], msg);
			JSB_PRECONDITION2( ok, cx, JS_FALSE, "Error processing arguments");
		} while (0);

		extension::MessageDelegate::sharedMessageDelegate()->postMessage("message", msg->c_str());

		CC_SAFE_DELETE(msg);

		JS_SET_RVAL(cx, vp, JSVAL_VOID);
		return JS_TRUE;	
	}
	JS_ReportError(cx, "wrong number of arguments: %d, was expecting %d", argc, 1);
	return JS_FALSE;
}

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#define USER_AGENT "Mozilla/5.0 (Windows NT 6.2; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/27.0.1453.94 Safari/537.36 Android/4.2 Cocos2dx/2.1"
#endif

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
#define USER_AGENT "Mozilla/5.0 (Windows NT 6.2; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/27.0.1453.94 Safari/537.36 iOS/6.1 Cocos2dx/2.1"
#endif

#ifndef USER_AGENT
#define  USER_AGENT "Mozilla/5.0 (Windows NT 6.2; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/27.0.1453.94 Safari/537.36 Cocos2dx/2.1"
#endif

void register_all_misc_manual(JSContext* cx, JSObject* global){
	
	JS_DefineProperty(cx, global, "window", OBJECT_TO_JSVAL(global), NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY);

	JS_DefineFunction(cx, global, "alert", js_cocos2dx_alert, 1, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineFunction(cx, global, "open", js_cocos2dx_open, 1, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineFunction(cx, global, "close", js_cocos2dx_close, 0, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineFunction(cx, global, "postMessage", js_cocos2dx_postMessage, 1, JSPROP_READONLY | JSPROP_PERMANENT);

	ScriptingCore::getInstance()->evalString("(function(){var timers=[];cc.setTimeout=function(callback,interval){cc.Director.getInstance().getScheduler().scheduleCallbackForTarget(this,callback,interval/1000,0,0,false);timers.push(callback);return timers.length-1};cc.setInterval=function(callback,interval){cc.Director.getInstance().getScheduler().scheduleCallbackForTarget(this,callback,interval/1000,cc.REPEAT_FOREVER,0,false);timers.push(callback);return timers.length-1};cc.clearTimeout=cc.clearInterval=function(id){var callback=timers[id];if(callback!=null){cc.Director.getInstance().getScheduler().unscheduleCallbackForTarget(this,callback);timers[id]=null;}};if(typeof setTimeout=='undefined'){setTimeout=cc.setTimeout;clearTimeout=cc.clearTimeout}if(typeof setInterval=='undefined'){setInterval=cc.setInterval;clearInterval=cc.clearInterval}})();", NULL);
	ScriptingCore::getInstance()->evalString(CCString::createWithFormat("(function(){if(typeof navigator == 'undefined'){navigator = {userAgent:'%s'}}})()", USER_AGENT)->getCString(), NULL);
	ScriptingCore::getInstance()->evalString("(function(){cc.range=function(begin,end){arguments.length<=1&&(end=begin||0,begin=0);for(var i=begin;i<end;++i)yield i};if(typeof range == 'undefined') range = cc.range;})();", NULL);
}