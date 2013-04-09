#ifndef __CQWrap_WebSocket_H__
#define __CQWrap_WebSocket_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

#include "cocos2d.h"
#include "cocos-ext.h"

#include "libwebsockets.h"
#include "pattern/CustEvent.h"
#include "util/JsonHelper.h"

#include <queue>
#include <pthread.h>
//#include <semaphore.h>
#include <errno.h>

#include "ScriptingCore.h"
#include "generated/jsb_cocos2dx_auto.hpp"
#include "cocos2d_specifics.hpp"

NS_CC_EXT_BEGIN

class WebSocket: public CCObject, public CustEvent{
protected:
	static CCArray* s_pool;	

	CCArray* m_responseMessage;
	pthread_mutex_t  m_responseQueueMutex;
	bool m_error;

	void dispatchEvents(float dt);
	void js_fire(const char* type, JsonData* msg);

	js_proxy_t* m_js_proxy;
	
public:
	static pthread_t s_networkThread;
	static CCArray* s_requestMessageQueue;
	static pthread_mutex_t  s_socketsMutex;
	static pthread_mutex_t  s_requestQueueMutex;

	static const unsigned short CONNECTING = 0;
	static const unsigned short OPEN = 1;
	static const unsigned short CLOSING = 2;
	static const unsigned short CLOSED = 3;

	static CCArray* getAllSockets();
	
	pthread_t  m_networkThread;
	unsigned short readyState;
	CCString* URL;

	WebSocket(const char* host, int port=80, const char* path="/");
	~WebSocket();
	void send(JsonData* data);
	void close();

	void putMessage(JsonData* message, bool error = false);

	void setJsProxy(js_proxy_t* m_js_proxy);
};

NS_CC_EXT_END

#endif