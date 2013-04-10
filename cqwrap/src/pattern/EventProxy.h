#ifndef __Pattern_JsProxy_H__
#define __Pattern_JsProxy_H__

#include "cocos-ext.h"
#include "util/JsonHelper.h"

NS_CC_EXT_BEGIN

template <class T>
class EventProxy{

protected:
	virtual void proxy_fire(const char* type, JsonData* msg) = 0;
	T* m_proxy;
public:
	virtual void setProxy(T* proxy){
		this->m_proxy = proxy;
	};

};

NS_CC_EXT_END

#endif
