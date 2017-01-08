#include "socket.h"
#include <time.h>

Socket *Server;

unsigned int __stdcall loopRecv(PVOID pM){
	SOCKET *client = (SOCKET *)pM;
	HTTP_REQ *request;
	HTTP_RES *response;
	while (true){
		request = Server->getData(*client);
		if (request != NULL){
			response = new HTTP_RES;
			FILE *fp;
			string path = "dir\\" + request->url;			//文件路径

			//实验设定:如果action为dopost
			if (request->url == "dopost"){		
				stringstream length;
				if (request->postData["login"] == "3140103367" && request->postData["pass"]=="3367"){
					response->statusCode = "200";
					response->status = "OK";
					string resData = "<html><body>登录成功</body></html>";
					int len = resData.length();
					length << len;
					response->length = len;
					response->headers["Content-Length"] = length.str();
					response->headers["Content-Type"] = "text/html";
					response->data = new byte[len];
					memcpy(response->data, resData.c_str(), len);
				}
				else{
					response->statusCode = "404";
					response->status = "Not Found";
					string resData = "<html><body>登录失败，账号或密码错误</body></html>";
					//printf("resData:%s", resData.c_str());
					int len = resData.length();
					length << len;
					response->length = len;
					response->headers["Content-Length"] = length.str();
					response->headers["Content-Type"] = "text/html";
					response->data = new byte[len];
					memcpy(response->data, resData.c_str(), len);
				}
			}
			else{
				// 解析文件类型
				char *fileType = (char *)request->url.c_str();
				while (*(fileType++) != '.');
				if (strcmp(fileType, "jpg") == 0){
					response->headers["Content-Type"] = "image/jpeg";
				}
				else if (strcmp(fileType, "png") == 0){
					response->headers["Content-Type"] = "image/png";
				}
				else if (strcmp(fileType, "txt") == 0){
					response->headers["Content-Type"] = "text/plain";
				}
				else if (strcmp(fileType, "html") == 0){
					response->headers["Content-Type"] = "text/html";
				}
				else {
					response->headers["Content-Type"] = "text/html";
				}

				printf("path:%s\n", path.c_str());
				fp = fopen(path.c_str(), "rb");
				if (fp == NULL){
					printf("Not Found!\n");
					response->statusCode = "404";
					response->status = "Not Found";
					response->headers["Content-Length"] = "0";
					response->data = NULL;
					response->length = 0;
				}
				else{
					stringstream str;
					response->statusCode = "200";
					response->status = "OK";
					fseek(fp, 0, SEEK_END);								//文件指针移至末尾
					int length = ftell(fp);
					rewind(fp);
					response->length = length;
					byte *buffer = new byte[length];
					int realLen = fread(buffer, 1, length, fp);
					//printf("fileContent:%x,Length:%d\n", buffer,realLen);
					str << length;
					//printf("fileLength:%s",str.str().c_str());
					response->headers["Content-Length"] = str.str();
					response->data = buffer;
					fclose(fp);
				}
			}
			Server->sendData(*client, response);
			delete response;
		}
		else return 0;
	}
}

void main(){
	Server = new Socket("SERVER");
	printf("***** 欢迎使用LowesYang HTTP服务器 *****\n\n");
	//循环接受连接
	while (true){
		SOCKET *sClient=new SOCKET;
		sockaddr_in *remoteAddr=new sockaddr_in;
		int nAddrlen = sizeof(*remoteAddr);
		printf("等待客户端连接...\n");
		*sClient = Server->acceptHandle((SOCKADDR *)remoteAddr, &nAddrlen);		//等待客户端连接
		if (*sClient == INVALID_SOCKET){
			printf("accept error!\n");
			continue;
		}
		printf("接收到一个连接:%s \r\n", inet_ntoa(remoteAddr->sin_addr));

		//开启子线程，并以socket句柄为参数
		HANDLE recvThread = (HANDLE)_beginthreadex(NULL, 0, loopRecv, (PVOID)sClient, 0, NULL);
		WaitForMultipleObjects(1, &recvThread, TRUE, 0);
	}
}