#include <winsock2.h>
#include <iostream>
#include <thread>
#include <string.h>
#include <mutex>
#include <string>
using namespace std;

#pragma comment(lib, "ws2_32.lib")			//add ws2_32.lib

const bool USE_TCP = true;
const int DEFAULT_PORT = 1234;
const char* SRV_IP = "10.91.254.134";
const char* CLI_IP = "10.26.245.161";
std::mutex buffer_mutex;
bool closed = false;

void receive_tcp(SOCKET sockConn) {

	while (true)
	{
		string str;
		char buffer[100];
		recv(sockConn, buffer, 99, 0);
		if (string(buffer) == "q"||closed) {
			closed = true;
			break;
		}
		{
			std::lock_guard<std::mutex> lock(buffer_mutex);
			string str(buffer);
			
			cout << "recv:" << str << endl;
		}
		
	}
}

void send_tcp(SOCKET sockConn) {

	while (true)
	{
		if (closed) return;
		string str;
		cin >> str;

		{
			std::lock_guard<std::mutex> lock(buffer_mutex);
			
			send(sockConn, str.c_str(), strlen(str.c_str())+1, 0);
			if (str == "q") {
				closed = true;
				break;
			}
		}
	}
}

void receive_udp(SOCKET sockSrv) {
	while (true)
	{
		if (closed) return;
		string str;
		char buffer[102];
		SOCKADDR_IN addrClt;
		int len = sizeof(addrClt);
		in_addr in;
		recvfrom(sockSrv, buffer, 99, 0, (sockaddr*)&addrClt, &len);

		{
			std::lock_guard<std::mutex> lock(buffer_mutex);
			string str(buffer);
			if (str == "q") closed = true;

			//print client ip
			memcpy(&in, &addrClt.sin_addr.s_addr, 4);
			cout << "message from: "<<inet_ntoa(in)<<endl;

			cout << "recv:" << str << endl;
		}
	}
}
void send_udp(SOCKET sockSrv, SOCKADDR_IN addrClt) {
	while (true)
	{
		if (closed) return;
		string str;
		cin >> str;
		int len = sizeof(addrClt);

		{
			std::lock_guard<std::mutex> lock(buffer_mutex);
			if (str == "q") closed = true;
			sendto(sockSrv, str.c_str(), strlen(str.c_str())+1, 0, (sockaddr*)&addrClt, len);
		}
	}
}
int main(int argc, char* argv[])
{

	WORD	wVersionRequested;
	WSADATA wsaData;
	int		err;
	SOCKET sockSrv;
	wVersionRequested = MAKEWORD(2, 2);//create 16bit data
	//Load WinSock
	err = WSAStartup(wVersionRequested, &wsaData);	//load win socket
	if (err != 0)
	{
		cout << "Load WinSock Failed!";
		return -1;
	}
	//create socket
	if (USE_TCP) {
		sockSrv = socket(AF_INET, SOCK_STREAM, 0);
	}
	else {
		sockSrv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	}
	if (sockSrv == INVALID_SOCKET) {
		cout << "socket() fail:" << WSAGetLastError() << endl;
		return -2;
	}
	//server IP
	SOCKADDR_IN addrSrv;
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_addr.s_addr = inet_addr(SRV_IP);
	addrSrv.sin_port = htons(DEFAULT_PORT);
	//bind
	err = bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
	if (err != 0)
	{

		cout << "bind()fail" << WSAGetLastError() << endl;
		return -3;
	}
	if (USE_TCP) {
		//listen
		err = listen(sockSrv, 5);
		if (err != 0)
		{

			cout << "listen()fail" << WSAGetLastError() << endl;
			return -4;
		}
		cout << "Server waitting...:" << endl;
		//client ip
		SOCKADDR_IN addrClt;
		int len = sizeof(SOCKADDR);


		//accept
		SOCKET sockConn = accept(sockSrv, (SOCKADDR*)&addrClt, &len);
		cout << "Connected" << endl;

		//send recv
		std::thread send_t = std::thread(send_tcp, sockConn);
		std::thread rcv = std::thread(receive_tcp, sockConn);

		send_t.detach();
		rcv.join();


		//close connected sock
		closesocket(sockConn);
	}
	else
	{
		//client ip
		cout << "UDP connetion" << endl;
		SOCKADDR_IN addrClt;
		addrClt.sin_family = AF_INET;
		addrClt.sin_port = htons(DEFAULT_PORT);
		addrClt.sin_addr.s_addr = inet_addr(CLI_IP);

		std::thread send_t = std::thread(send_udp, sockSrv,addrClt);
		std::thread rcv = std::thread(receive_udp, sockSrv);

		send_t.join();
		rcv.join();
	}
	//close server sock
	closesocket(sockSrv);
	//clean up winsock
	WSACleanup();
	return 0;
}