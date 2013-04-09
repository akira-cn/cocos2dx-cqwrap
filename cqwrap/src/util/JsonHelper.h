
#ifndef __CQWrap_JSON_H__
#define __CQWrap_JSON_H__

#include "cocos2d.h"
#include "cocos-ext.h"
#include "json/json.h"

NS_CC_EXT_BEGIN
	
//typedef Json::Value JsonData;
class JsonData: public Json::Value, public cocos2d::CCObject{

};

class JSON{
public:
	static JsonData* parse(std::string json_str);
	static std::string stringify(Json::Value* json_obj);
};

NS_CC_EXT_END

#endif