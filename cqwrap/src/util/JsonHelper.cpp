#include "JsonHelper.h"

USING_NS_CC_EXT;

JsonData* JSON::parse(std::string json_str){
	Json::Reader reader;
	JsonData* data = new JsonData();

	reader.parse(json_str.c_str(), *data);
	return data;
}

std::string JSON::stringify(Json::Value* json_obj){
	if(json_obj == NULL){
		return std::string("null");
	}
	Json::FastWriter writer;
	std::string str = writer.write(*json_obj);
	//str.erase(0,str.find_first_not_of("\r\t\n "));
	str.erase(str.find_last_not_of("\r\t\n ")+1);
	return str;
}