#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <process.h>
#include <hash_map>
#include <sstream>
#pragma comment(lib,"ws2_32.lib")
using namespace std;

#define PORT 3367				//Ĭ��Ϊ����ѧ��
#define MAX_PATH 1024

// HTTP������ṹ��
typedef struct{
	string method;							//GET or POST
	string url;								//����URL
	string protocol;						//����Э�飬һ��ΪHTTP/1.1
	hash_map<string, string> headers;		//����ͷ
	hash_map<string, string> postData;		//������Ϣ��
}HTTP_REQ;

// HTTP��Ӧ���ṹ��
typedef struct{
	string statusCode;						//״̬��
	string status;							//״̬��������
	hash_map<string, string> headers;		//��Ӧͷ
	int length;								//��Ϣ�峤��
	byte *data;								//������Ϣ��
}HTTP_RES;

class Socket{
private:
	SOCKET ssocket;
	sockaddr_in sin;
public:
	Socket(string type, char *ipAddr="", int port=80);		//��ʼ��

	SOCKET getHandle();			

	sockaddr_in getsin();

	SOCKET getSSocket();

	SOCKET acceptHandle(sockaddr *addr, int *addrLen);		//��װԭ����accept

	void sendData(SOCKET client, HTTP_RES *res);//��������(�����)

	HTTP_REQ *getData(SOCKET client);			//��������

	void close();		// �ر�socket

};