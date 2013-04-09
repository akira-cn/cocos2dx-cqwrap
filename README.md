CQWrap
==================

Extensions for cocos2d-x javascript bindings.

1. WebSocket for cocos2d-x

Export websocket api in javascript for cocos2d-x javascript bindings.
based on: 
[http://git.warmcat.com/cgi-bin/cgit/libwebsockets/]
[http://sourceforge.net/projects/jsoncpp/]

```js
var host = "ws://localhost:8080/test";
var ws = new WebSocket(host,'default-protocol'); // create the web socket
ws.onopen = function () {
   cc.log("connected... ", 'success'); // we are in! :D
};

ws.onmessage = function (evt) {
   cc.log(evt.data, 'response'); // we got some data - show it omg!!
};

ws.onclose = function () {
   cc.log("Socket closed!", 'error'); // the socket was closed (this could be an error or simply that there is no server)
};
```