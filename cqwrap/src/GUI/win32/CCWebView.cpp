#include "../CCWebView.h"

#include <atlbase.h>
CComModule _Module;
#include <atlwin.h>
#include <atlconv.h>
#pragma comment(lib,"atl")
#pragma comment(lib,"User32.lib")

USING_NS_CC_EXT;
static CAxWindow WinContainer;

DECLARE_SINGLETON_MEMBER(CCWebView);

bool CCWebView::open(const char* url){

	CCDirector::sharedDirector()->stopAnimation();

	RECT rc;
	IWebBrowser2* iWebBrowser;
	VARIANT varMyURL;
	
	LPOLESTR pszName=OLESTR("shell.Explorer.2");    
	GetClientRect(CCEGLView::sharedOpenGLView()->getHWnd(), &rc);  

	WinContainer.Create(CCEGLView::sharedOpenGLView()->getHWnd(), rc, 0,WS_CHILD |WS_VISIBLE);  
	WinContainer.CreateControl(pszName);  
	WinContainer.QueryControl(__uuidof(IWebBrowser2),(void**)&iWebBrowser);   
	VariantInit(&varMyURL);  
	varMyURL.vt = VT_BSTR;   
	USES_CONVERSION;
	varMyURL.bstrVal = SysAllocString(A2OLE(url));  
	iWebBrowser-> Navigate2(&varMyURL,0,0,0,0);  

	VariantClear(&varMyURL);   
	iWebBrowser->Release();
	
	return true;
}

void CCWebView::close(){
	WinContainer.DestroyWindow();
	CCDirector::sharedDirector()->startAnimation();
}

CCWebView::~CCWebView(){
	CCWebView::dropInstance();
	WinContainer.DestroyWindow();
}
