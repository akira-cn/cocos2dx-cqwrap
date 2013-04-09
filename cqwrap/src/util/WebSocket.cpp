#include "WebSocket.h"

#include <queue>
#include <pthread.h>
//#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include "json.h"

USING_NS_CC_EXT;

pthread_t				WebSocket::s_networkThread;
CCArray*				WebSocket::s_requestMessageQueue = NULL;
pthread_mutex_t			WebSocket::s_socketsMutex;
pthread_mutex_t			WebSocket::s_requestQueueMutex;

/* list of supported protocols and callbacks */
static int socket_callback(struct libwebsocket_context *ctx,
	struct libwebsocket *wsi,
	enum libwebsocket_callback_reasons reason,
	void *user, void *in, size_t len);

static struct libwebsocket_protocols protocols[] = {
	{
		"default-protocol",
		 socket_callback,
		 0,
	},

	{ NULL, NULL, 0, 0 } /* end */
};

static void* networkThread(void *data);
CCArray* WebSocket::s_pool = NULL;		//socket shared resource pool

WebSocket::WebSocket(const char* host, int port/* =80 */, const char* path/* ="/" */){

	m_js_proxy = NULL;
	m_error = false;

	URL = CCString::createWithFormat("ws://%s:%d%s", host, port, path);
	URL->retain();

	if(s_pool == NULL){
		pthread_mutex_init(&s_socketsMutex, NULL);
		pthread_mutex_init(&s_requestQueueMutex, NULL);
		s_pool = new CCArray();
		s_requestMessageQueue = new CCArray();
	}
	s_pool->addObject(this);
	
	CCDirector::sharedDirector()->getScheduler()->scheduleSelector(
		schedule_selector(WebSocket::dispatchEvents), this, 0, false);
	CCDirector::sharedDirector()->getScheduler()->pauseTarget(this);
	
	Json::Value *info = new Json::Value();
	(*info)["host"] = host;
	(*info)["port"] = port;
	(*info)["path"] = path;

	pthread_mutex_init(&m_responseQueueMutex, NULL);
	this->m_responseMessage = new CCArray();
	this->readyState = CONNECTING;
	pthread_create(&m_networkThread, NULL, networkThread, info);
}

void WebSocket::js_fire(const char* type, JsonData* msg){
	if(m_js_proxy != NULL){

		jsval retval;
		
		JsonData evt;
		evt["data"] = JSON::stringify(msg);
		evt["source"] = Json::nullValue;	//not implemented yet
		evt["type"] = type;
		evt["lastEventId"] = "";	//not implemented yet
		evt["origin"] = this->URL->getCString();

		std::string handler = std::string("on");
		handler+=type;
		
		jsval dataVal = STRING_TO_JSVAL(JS_NewStringCopyZ(ScriptingCore::getInstance()->getGlobalContext(), JSON::stringify(&evt).c_str()));

		JSObject* obj = ScriptingCore::getInstance()->getGlobalObject();
		JSContext* cx = ScriptingCore::getInstance()->getGlobalContext();
		jsval nsval;
		JS_GetProperty(cx, obj, "JSON", &nsval);
		
		ScriptingCore::getInstance()->executeFunctionWithOwner(nsval, "parse", 1, &dataVal, &dataVal);
		ScriptingCore::getInstance()->executeFunctionWithOwner(OBJECT_TO_JSVAL(m_js_proxy->obj), handler.c_str(), 1, &dataVal, &retval);
	}
}

void WebSocket::setJsProxy(js_proxy_t* proxy){
	m_js_proxy = proxy;
}

void WebSocket::dispatchEvents(float dt){
	
	CCAssert(!pthread_equal(m_networkThread, pthread_self()), "dispatchEvents should not run in sub thread!");
	
	//CCLog("receive event!");

	if(this->m_error){
		this->fire("error", NULL);
		this->js_fire("error", NULL);

		CCDirector::sharedDirector()->getScheduler()->pauseTarget(this);
		this->fire("close", NULL);
		this->js_fire("close", NULL);
		
		readyState = CLOSED;
		return;
	}

	if(this->readyState == CONNECTING){
		this->readyState = OPEN;
		this->fire("open", NULL);
		this->js_fire("open", NULL);
	}

	pthread_mutex_lock(&m_responseQueueMutex);
	if(m_responseMessage->count() > 0){
		JsonData* msg = (JsonData*)m_responseMessage->objectAtIndex(0);
		if(!msg->isNull()){
			this->fire("message", msg);
			this->js_fire("message", msg);
		}else{
			CCDirector::sharedDirector()->getScheduler()->pauseTarget(this);
			this->fire("close", NULL);
			this->js_fire("close", NULL);

			readyState = CLOSED;
		}
		m_responseMessage->removeObject(msg);
		CC_SAFE_DELETE(msg);
	}
	pthread_mutex_unlock(&m_responseQueueMutex);

	if(m_responseMessage->count() <= 0){
		CCDirector::sharedDirector()->getScheduler()->pauseTarget(this);
	}
}

void WebSocket::putMessage(JsonData* message, bool error /*= fase*/){
	CCAssert(pthread_equal(m_networkThread, pthread_self()), "putMessage should run in sub thread!");
	if(readyState == CLOSED){
		return;
	}
	pthread_mutex_lock(&m_responseQueueMutex);
	this->m_responseMessage->addObject(message);
	pthread_mutex_unlock(&m_responseQueueMutex);

	CCDirector::sharedDirector()->getScheduler()->resumeTarget(this);	
	m_error |= error;
}

WebSocket::~WebSocket(){
	if(readyState != CLOSED){
		this->fire("close", NULL);
		this->js_fire("close", NULL);
		
		readyState = CLOSED;
	}
	CCAssert(!pthread_equal(m_networkThread, pthread_self()), "websocket instance should not release in sub thread!");

	pthread_mutex_lock(&m_responseQueueMutex);
	CCObject* obj;
	CCARRAY_FOREACH(m_responseMessage, obj){
		obj->release();
	}
	m_responseMessage->release();
	pthread_mutex_unlock(&m_responseQueueMutex);
	pthread_mutex_destroy(&m_responseQueueMutex);

	if(NULL != s_pool && s_pool->containsObject(this)){
		pthread_mutex_lock(&s_socketsMutex);
		s_pool->removeObject(this);
		pthread_mutex_unlock(&s_socketsMutex);
	}
	
	if(NULL != s_pool && s_pool->count() <= 0){
		CCObject* obj;
		
		CCARRAY_FOREACH(s_requestMessageQueue, obj){
			((JsonData*)obj)->release();
		}
		
		pthread_mutex_lock(&s_requestQueueMutex);
		s_requestMessageQueue->release();
		s_requestMessageQueue = NULL;
		pthread_mutex_unlock(&s_requestQueueMutex);

		pthread_mutex_destroy(&s_socketsMutex);
		pthread_mutex_destroy(&s_requestQueueMutex);
		
		s_pool->release();
		s_pool = NULL;
	}

	URL->release();
}

void WebSocket::close(){
	send(JSON::parse("null"));
}

void WebSocket::send(JsonData* data){
	if(readyState == CLOSED){
		return;
	}
	if(data->isNull()){
		readyState = CLOSING;
	}
	JsonData* pack = new JsonData();
	(*pack)["data"] = *data;
	(*pack)["tid"] = CCString::createWithFormat("%lu", m_networkThread)->getCString();

	struct timeval tv;
	gettimeofday(&tv, NULL);

	//message time out after 30sec
	(*pack)["timeout_limit"] = tv.tv_sec + 30.0;	

	pthread_mutex_lock(&s_requestQueueMutex);
	s_requestMessageQueue->addObject(pack); 
	pthread_mutex_unlock(&s_requestQueueMutex);
}

CCArray* WebSocket::getAllSockets(){
	return s_pool;
}

// Worker thread
static void* networkThread(void *data){

	struct libwebsocket_context *context;
	struct lws_context_creation_info info;
	memset(&info, 0, sizeof info);

	/*
	 * create the websocket context.  This tracks open connections and
	 * knows how to route any traffic and which protocol version to use,
	 * and if each connection is client or server side.
	 *
	 * For this client-only demo, we tell it to not listen on any port.
	 */

	info.port = CONTEXT_PORT_NO_LISTEN;
	info.protocols = protocols;
#ifndef LWS_NO_EXTENSIONS
	info.extensions = libwebsocket_get_internal_extensions();
#endif
	info.gid = -1;
	info.uid = -1;

	Json::Value serverInfo = *((Json::Value*) data);

	context = libwebsocket_create_context(&info);

	const char* server = serverInfo["host"].asCString();
	int port = serverInfo["port"].asInt();
	const char* path = serverInfo["path"].asCString();

	if(NULL != context){
		struct libwebsocket *socket;
		socket = libwebsocket_client_connect(context, server, port, 0,
			path, server, server,
			protocols[0].name, -1);

		if(socket != NULL){
			//CCLog("opened!");
			pthread_t tid = pthread_self();
			while(true){
				libwebsocket_service(context, 0);
			}
		}
		libwebsocket_context_destroy(context);
	}

	CC_SAFE_DELETE(data);
	pthread_exit(NULL);

	return 0;
}

static int
	socket_callback(struct libwebsocket_context *ctx,
	struct libwebsocket *wsi,
	enum libwebsocket_callback_reasons reason,
	void *user, void *in, size_t len)
{
	unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 4096 +
		LWS_SEND_BUFFER_POST_PADDING];
	int l;
	
	CCLog("socket callback for %d reason", reason);
	
	switch (reason) {
	case LWS_CALLBACK_PROTOCOL_DESTROY:
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		{
			CCObject* obj;
			pthread_mutex_lock(&WebSocket::s_socketsMutex);
			CCARRAY_FOREACH(WebSocket::getAllSockets(), obj){
				WebSocket* socket = (WebSocket*)obj;
				if(pthread_equal(socket->m_networkThread, pthread_self())){
					if(reason != LWS_CALLBACK_PROTOCOL_DESTROY || socket->readyState == WebSocket::CONNECTING){
						//CONNECTION_ERROR or PROTOCOL_DESTROY before OPEN
						CCLog("connect error for %d reason", reason);
						socket->putMessage(JSON::parse("null"), true);	//put a errorMessage
					}
					break;
				}
			}
			pthread_mutex_unlock(&WebSocket::s_socketsMutex);	

		}
		break;
	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		/*
		 * start the ball rolling,
		 * LWS_CALLBACK_CLIENT_WRITEABLE will come next service
		 */

		CCLog("connection complete");

		{
			CCObject* obj;
			pthread_mutex_lock(&WebSocket::s_socketsMutex);
			CCARRAY_FOREACH(WebSocket::getAllSockets(), obj){
				WebSocket* socket = (WebSocket*)obj;
				if(pthread_equal(socket->m_networkThread, pthread_self())){
					//fire open event
					CCDirector::sharedDirector()->getScheduler()->resumeTarget(socket);	
					break;
				}
			}
			pthread_mutex_unlock(&WebSocket::s_socketsMutex);	

		}

		libwebsocket_callback_on_writable(ctx, wsi);
		break;

	case LWS_CALLBACK_CLIENT_WRITEABLE:

		if(WebSocket::s_requestMessageQueue->count()){ 
			CCObject* obj;
			struct timeval tv;
			gettimeofday(&tv, NULL);

			pthread_mutex_lock(&WebSocket::s_requestQueueMutex);
			CCARRAY_FOREACH(WebSocket::s_requestMessageQueue, obj){
				JsonData* data = (JsonData*)obj;
				//CCLog(JSON::stringify(data).c_str());

				double timeout_limit = (*data)["timeout_limit"].asDouble();

				if(tv.tv_sec >= timeout_limit){
					//drop the message if timeout
					WebSocket::s_requestMessageQueue->removeObject(data);
					data->release();
					break;	
				}
				const char* tid = CCString::createWithFormat("%lu", pthread_self())->getCString();
				if(!strcmp(tid, (*data)["tid"].asCString())){
					//if the message is mine
					WebSocket::s_requestMessageQueue->removeObject(data);

					Json::Value _data = (*data)["data"];
					if(_data.isNull()){
						
						CCLog("connection closed by client"); //close by sending a null message
						
						{
							CCObject* obj;

							pthread_mutex_lock(&WebSocket::s_socketsMutex);
							CCARRAY_FOREACH(WebSocket::getAllSockets(), obj){
								WebSocket* socket = (WebSocket*)obj;
								
								if(pthread_equal(socket->m_networkThread, pthread_self())){
									socket->autorelease();
									WebSocket::getAllSockets()->removeObject(obj);

									{
										CCObject* obj;
										const char* tid = CCString::createWithFormat("%lu", pthread_self())->getCString();
										CCARRAY_FOREACH(WebSocket::s_requestMessageQueue, obj){
											JsonData* data = (JsonData*)obj;
											if(!strcmp(tid, (*data)["tid"].asCString())){
												//remove all message left before close
												WebSocket::s_requestMessageQueue->removeObject(data);	
											}
										}
									}
									//answer a null message before quit
									socket->putMessage(JSON::parse("null"));	
									break;
								}
							}
							pthread_mutex_unlock(&WebSocket::s_socketsMutex);
							pthread_mutex_unlock(&WebSocket::s_requestQueueMutex);

							libwebsocket_context_destroy(ctx);
							pthread_exit(NULL);
						}
						break;
					}
					
					CCLog("send data %s to server", JSON::stringify(&_data).c_str());
					
					std::string msg = JSON::stringify(&_data);

					l = sprintf((char *)&buf[LWS_SEND_BUFFER_PRE_PADDING],
						"%s", msg.c_str());
					libwebsocket_write(wsi,
						&buf[LWS_SEND_BUFFER_PRE_PADDING], l, LWS_WRITE_TEXT);

					data->release();
					break;	
				}
			}
			pthread_mutex_unlock(&WebSocket::s_requestQueueMutex);
		}

		/* get notified as soon as we can write again */

		libwebsocket_callback_on_writable(ctx, wsi);

		/*
		 * without at least this delay, we choke the browser
		 * and the connection stalls, despite we now take care about
		 * flow control
		 */

		usleep(200);
		break;

	case LWS_CALLBACK_CLOSED:
		//closed
		CCLog("connection closed by server");
		{
			CCObject* obj;

			pthread_mutex_lock(&WebSocket::s_socketsMutex);
			CCARRAY_FOREACH(WebSocket::getAllSockets(), obj){
				WebSocket* socket = (WebSocket*)obj;
				if(pthread_equal(socket->m_networkThread, pthread_self())){
					socket->autorelease();
					WebSocket::getAllSockets()->removeObject(obj);
					
					{
						CCObject* obj;
						CCARRAY_FOREACH(WebSocket::s_requestMessageQueue, obj){
							JsonData* data = (JsonData*)obj;
							const char* tid = CCString::createWithFormat("%lu", pthread_self())->getCString();
							if(!strcmp(tid, (*data)["tid"].asCString())){
								WebSocket::s_requestMessageQueue->removeObject(data);	
							}
						}
					}

					socket->putMessage(JSON::parse("null"));	
					break;
				}
			}
			pthread_mutex_unlock(&WebSocket::s_socketsMutex);
			pthread_mutex_unlock(&WebSocket::s_requestQueueMutex);

			libwebsocket_context_destroy(ctx);
			pthread_exit(NULL);
		}
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
		((char *)in)[len] = '\0';
		
		CCLog("data received: %d '%s' by server", (int)len, (char *)in);
		{
			CCObject* obj;

			pthread_mutex_lock(&WebSocket::s_socketsMutex);
			CCARRAY_FOREACH(WebSocket::getAllSockets(), obj){
				WebSocket* socket = (WebSocket*)obj;
				if(pthread_equal(socket->m_networkThread, pthread_self())){
					socket->putMessage(JSON::parse(std::string((char *)in)));
					break;
				}
			}
			pthread_mutex_unlock(&WebSocket::s_socketsMutex);
		}

		break;

	default:
		break;
	}

	return 0;
}
