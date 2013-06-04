
#ifndef __CQWrap_JSON_H__
#define __CQWrap_JSON_H__

#include "cocos2d.h"
#include "cocos-ext.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
#include "../third_party/json/include/json/json.h"
#else
#include "json/json.h"
#endif

NS_CC_EXT_BEGIN
	
class JsonData: public Json::Value, public cocos2d::CCObject{

};

class JSON{
public:
	static JsonData* parse(std::string json_str);
	static std::string stringify(Json::Value* json_obj);
};

#define _JSON_STR(DAT, ...) #DAT","#__VA_ARGS__
#define _JSON_OBJ(DAT, ...) cocos2d::extension::JSON::parse(_JSON_STR(DAT, __VA_ARGS__))

NS_CC_EXT_END

#endif