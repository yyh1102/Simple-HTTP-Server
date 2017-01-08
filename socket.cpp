#include "socket.h"

Socket::Socket(string type,char *ipAddr,int port){
	// ��ʼ��WSA
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0){
		return;
	}
	// ����socket
	this->ssocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ssocket == INVALID_SOCKET){
		printf("socket error!\n");
		return;
	}
	// ��IP��˿�
	this->sin.sin_family = AF_INET;
	this->sin.sin_port = htons(PORT);
	if (type == "SERVER"){		//�����
		this->sin.sin_addr.S_un.S_addr = INADDR_ANY;
		if (bind(this->ssocket, (LPSOCKADDR)&this->sin, sizeof(this->sin)) == SOCKET_ERROR){
			printf("bind error !\n");
			return;
		}

		//��ʼ����
		if (listen(this->ssocket, 5) == SOCKET_ERROR){
			printf("listen error!\n");
			return;
		}
	}
	else if (type == "CLIENT"){		//�ͻ���
		this->sin.sin_addr.S_un.S_addr = inet_addr(ipAddr);
		if (connect(this->ssocket, (sockaddr *)&this->sin, sizeof(this->sin))==SOCKET_ERROR){
			printf("connect error!\n");
			closesocket(this->ssocket);
			return;
		}
	}
	else {
		printf("type error!\n");
		return;
	}

}

sockaddr_in Socket::getsin(){
	return this->sin;
}

SOCKET Socket::getSSocket(){
	return this->ssocket;
}

SOCKET Socket::acceptHandle(sockaddr *addr,int *addrLen){
	return accept(this->ssocket, addr, addrLen);
}

//�������ݲ�����HTTP��
HTTP_REQ *Socket::getData(SOCKET client){
	int bufferLen = 65535;
	byte *recData = new byte[bufferLen];
	byte *result = new byte[bufferLen];
	int reqEnd = 0;							//�ֶ���Ϣ����ʱ�յ�
	memset(result, 0, 65535);
	bool isEnd = false;						//���������Ƿ���
	int ret = 0;
	while (!isEnd){
		memset(recData, 0, 65535);
		ret = recv(client, (char *)recData, bufferLen, 0);
		if (ret > 0){
			char *endSymbol = strstr((char *)recData, "\r\n");
			if (endSymbol != NULL){												//�����꣬��ֱ�ӷ���
				isEnd = true;
				reqEnd = ret;
				memcpy(result, recData, reqEnd);
				bufferLen = 65535;
			}
			else{																//��δ���꣬���·���64KB����
				bufferLen += 65535;
				byte *temp = new byte[bufferLen];
				memset(temp, 0, bufferLen);
				memcpy(temp, result, reqEnd);									//�������յ�����Ϣ
				memcpy(&temp[reqEnd], recData, strlen((char *)recData));		//��������Ϣ
				reqEnd += ret;
				//delete[] result;
				result = temp;
			}
		}
		if (ret == SOCKET_ERROR || ret == 0) return NULL;
	}
	//printf("parse finish:%s\n",result);
	HTTP_REQ *request = new HTTP_REQ;
	//printf("Content-length:%d\n", reqEnd);

	int begin = 0;
	int line = 0;		//��¼ȡ��������
	// ����HTTP�ֽ���
	for (int i = 0; i < reqEnd; i++){
		if (result[i] == '\n'){
			if (line == 0){									//��һ��
				char *tmpStr = new char[i - begin + 1];
				for (int j = 0; j < 3; j++){
					int index = 0;
					while (begin < i && result[begin] != ' '){
						tmpStr[index++] = result[begin++];
					}
					begin++;
					tmpStr[index] = '\0';
					request->method = tmpStr;
					if (j == 0){							//method
						request->method = tmpStr;
						printf("method:%s\n", tmpStr);

					}
					else if (j == 1){						//url and path
						request->url = tmpStr;
						request->url = request->url.substr(1, request->url.length() - 1);
						if (request->url.length() == 0){
							request->url = "index.html";
						}
						printf("url:%s\n", request->url.c_str());

					}
					else{									//protocol
						request->protocol = tmpStr;
						printf("protocol:%s\n", tmpStr);

					}
					memset(tmpStr, 0, i - begin + 1);		//��ʼ����ʱ�ַ���
				}
			}
			else{											//����ͷ�ֶ�
				int type = 0;								//0Ϊkey,1Ϊvalue
				char *key = new char[i - begin];			
				char *value = new char[i - begin];
				int index = 0;
				while (begin < i){
					if (result[begin] == ':'){
						key[index] = '\0';
						index = 0;
						type = 1;
						begin++;
						continue;
					}
					if (type == 0)
						key[index++] = result[begin++];
					else
						value[index++] = result[begin++];
				}
				value[index] = '\0';
				printf("key:%s,value:%s\n", key, value);
				request->headers[key] = value;
			}
			begin = i + 1;
			line++;
			if (result[begin] == '\r' && result[begin + 1] == '\n'){	//ͷ�ֶ��ѽ������
				begin += 2;
				break;
			}
		}

	}

	printf("begin:%d,reqEnd:%d\n", begin, reqEnd);
	//������Ϣ������
	if (begin < reqEnd){
		byte *data = new byte[reqEnd - begin + 1];
		memcpy(data, &result[begin], reqEnd - begin);
		data[reqEnd-begin] = '\0';
		printf("POST:%s\n", data);
		bool readKey = false, readValue=false ;			//falseΪ��key,trueΪ��value
		int begin = 0;
		char *key, *value;
		for (int i = 0; i < strlen((char *)data); i++){
			if (data[i]=='='){
				key = new char[i - begin];
				memcpy(key, &data[begin], i - begin);
				key[i - begin] = '\0';
				begin = i + 1;
				readKey = true;
			}
			else if (data[i] == '&' || i + 1 == strlen((char *)data)){
				if (i + 1 == strlen((char *)data))
					i = strlen((char *)data);
				value = new char[i - begin];
				memcpy(value, &data[begin], i - begin);
				value[i - begin ] = '\0';
				begin = i + 1;
				readValue = true;
			}

			if (readKey && readValue){
				request->postData[key] = value;
				printf("key:%s,value:%s\n", key, value);
				readKey = readValue = false;
			}
		}
	}
	//printf("finish!\n");
	//printf("%s\n%s\n%s\n%s\n", request->method, request->url, request->protocol, request->data);
	return request;
}

// ��װHTTP���ݰ�������
void Socket::sendData(SOCKET client, HTTP_RES *res){
	printf("Begin to Send!\n");
	long long bufferLen = 4 * 1024 * 1024;

	string resData="";
	//��װ��Ӧͷ������
	string firstLine = "HTTP/1.1 " + res->statusCode + " " + res->status + "\n";
	resData += firstLine;

	//��װͷ��
	for (hash_map<string, string>::iterator hash = res->headers.begin();
		hash != res->headers.end(); hash++){
		string header = hash->first + ":" + hash->second + "\n";
		resData += header;
		printf("%s", header.c_str());
	}
	resData += "\r\n";
	int tmpLen = resData.length();
	byte *buffer = new byte[tmpLen + res->length];
	memset(buffer, '\0', tmpLen + res->length);
	memcpy(buffer, resData.c_str(), tmpLen);
	//��װ����
	if (res->data){
		memcpy(&buffer[tmpLen], res->data, res->length);
	}
	int finishLen=send(client, (char *)buffer, tmpLen + res->length, 0);
	printf("Send length:%d\n", finishLen);
	delete[] buffer;
}

void Socket::close(){
	closesocket(this->ssocket);
	WSACleanup();
}