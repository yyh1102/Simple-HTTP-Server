#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <process.h>
#include <hash_map>
#include <sstream>
#pragma comment(lib,"ws2_32.lib")
using namespace std;

#define PORT 3367				//默认为本人学号
#define MAX_PATH 1024

// HTTP请求包结构体
typedef struct{
	string method;							//GET or POST
	string url;								//请求URL
	string protocol;						//所用协议，一般为HTTP/1.1
	hash_map<string, string> headers;		//请求头
	hash_map<string, string> postData;		//请求消息体
}HTTP_REQ;

// HTTP响应包结构体
typedef struct{
	string statusCode;						//状态码
	string status;							//状态文字描述
	hash_map<string, string> headers;		//响应头
	int length;								//消息体长度
	byte *data;								//返回消息体
}HTTP_RES;

class Socket{
private:
	SOCKET ssocket;
	sockaddr_in sin;
public:
	Socket(string type, char *ipAddr="", int port=80);		//初始化

	SOCKET getHandle();			

	sockaddr_in getsin();

	SOCKET getSSocket();

	SOCKET acceptHandle(sockaddr *addr, int *addrLen);		//封装原生的accept

	void sendData(SOCKET client, HTTP_RES *res);//发送数据(服务端)

	HTTP_REQ *getData(SOCKET client);			//接收数据

	void close();		// 关闭socket

};