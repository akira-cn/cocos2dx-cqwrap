#include "NativeAdapter.h"

USING_NS_CC_EXT;

#if(CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

#include <jni.h>
#include "android/log.h"
#include "platform/android/jni/JniHelper.h"

void NativeAdapter::postMessage(const char* message, const char* data = "null"){
	JniMethodInfo info;
	if(JniHelper::getStaticMethodInfo(info, "com.weizoo.utils.CocosMessageDelegate", "onMessage", "(Ljava/lang/String;Ljava/lang/String;)V")){
		CCLog("function onMessage was found");

		jstring msg = info.env->NewStringUTF(message);
		jstring dat = info.env->NewStringUTF(data);

		info.env->CallStaticVoidMethod(info.classID, info.methodID, msg, dat);
		info.env->DeleteLocalRef(msg);
		info.env->DeleteLocalRef(dat);
		info.env->DeleteLocalRef(info.classID);
	}else{
		CCLog("function onMessage was not found");
	}
}

extern "C" bool Java_com_weizoo_utils_CocosMessageDelegate_receiveMessage(JNIEnv *env, jobject thiz, 
																		  jbyteArray message, 
																		  jbyteArray data)
{


	char* szMsg = NULL;
	char* szData = NULL;

	jsize len1 = env->GetArrayLength(message); 
	jbyte* msg = env->GetByteArrayElements(message, JNI_FALSE);  

	szMsg = (char*)malloc(len1 + 1);
	memcpy(szMsg, msg, len1);
	szMsg[len1] = 0;

	jsize len2 = env->GetArrayLength(data);
	jbyte* dat = env->GetByteArrayElements(data, JNI_FALSE);

	szData = (char*)malloc(len2 + 1);
	memcpy(szData, dat, len2);
	szData[len2] = 0;
	env->ReleaseByteArrayElements(message, msg,0);
	env->ReleaseByteArrayElements(data, dat,0);

	CCLog("received: %s, %s, %d, %d", szMsg, szData, len1, len2);
	//CCNotificationCenter::sharedNotificationCenter()->postNotification(szMsg, CCString::create(szData));
	MessageDelegate::sharedMessageDelegate()->getNotifier()->postNotification(szMsg, CCString::create(szData));
	free(szMsg);
	free(szData);
};
#else
#if(CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
///TODO
void NativeAdapter::postMessage(const char* message, const char* data = "null"){
	CCLog("send: %s -> %s", message, data);
}
#else
void NativeAdapter::postMessage(const char* message, const char* data = "null"){
	CCLog("send: %s -> %s", message, data);
}
#endif
#endif