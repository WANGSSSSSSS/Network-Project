//
// Created by wang_shuai on 2020/10/18.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>
#include <mutex>

#include <iostream>
std::mutex buffer_mutex;
std::string str = "hello , hihihi";



bool e = false;
void tweet(int socket)
{
    while(1)
    {
        std::string buffer;
        std::cin >> buffer;
        {
            std::lock_guard<std::mutex> lock(buffer_mutex);
            str = buffer;
            write(socket, str.c_str(), str.length()+1);
        }

        if (buffer == "q" ||  e)
        {
            e = true;
            break;
        }
        sleep(1);
    }
    close(socket);
    exit(0);

}

void receive(int socket)
{
    while(1)
    {
        char buffer[100];
        read(socket, buffer, 99);
        {
            std::lock_guard<std::mutex> lock(buffer_mutex);
            str = std::string(buffer);
            if (str == "q") {
                e = true;
                break;
            }
            std::cout << "receive data : " + str << std::endl;
        };

        if (e) {
            break;
        }

    }
    close(socket);
    exit(0);
}


void send_udp(int socket, sockaddr*  dst)
{
    while(1)
    {
        std::string buffer;
        std::cin >> buffer;

        if (str == "q" ||  e)
        {
            e = true;
            break;
        }

        {
            std::lock_guard<std::mutex> lock(buffer_mutex);
            str = buffer;
            sendto(socket, str.c_str(), str.length()+1,0,dst, sizeof(*dst));
        }

        sleep(1);
    }
    close(socket);
    exit(0);
}


void receive_udp(int socket)
{
    while (1) {
        char buffer[100];
        //read(socket, buffer, 99);

        sockaddr_in v6_rec;

        socklen_t len = sizeof(sockaddr_in);
        recvfrom(socket, buffer, 99, 0, (sockaddr*)&v6_rec,&len);

        {
            std::lock_guard<std::mutex> lock(buffer_mutex);
            str = std::string(buffer);
            if (str == "q")
            {
                e = true;
                break;
            }
            in_addr in;
            memcpy(&in,&v6_rec.sin_addr.s_addr,4);
            std::cout << "receive data : " + str + " from address : " << inet_ntoa(in)  << std::endl;
        };

        if(e)
        {
            break;
        }
        // sleep(1);
    }

    close(socket);
    exit(0);
}

int main(int argc, char** args){
    //创建套接字
    int sock;
    if(argc == 0)return 0;

    std::string Type(args[1]);

    std::cout<< argc<<std::endl;
    std::cout<< Type <<std::endl;
    if(Type == "UDP")
    {
        sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }
    else sock = socket(AF_INET, SOCK_STREAM, 0);


    struct sockaddr_in serv_addr, self_addr;

    memset(&serv_addr, 0, sizeof(serv_addr));  //每个字节都用0填充
    serv_addr.sin_family = AF_INET;  //使用IPv6地址

    serv_addr.sin_addr.s_addr = inet_addr("10.91.254.134");

    serv_addr.sin_port = htons(1234);  //端口



    //std::cout << s;
    //读取服务器传回的数



    if(Type == "UDP")
    {

        self_addr.sin_family = AF_INET;  //使用IPv6地址

        self_addr.sin_addr.s_addr = inet_addr("10.26.245.161");

        self_addr.sin_port = htons(1234);  //端口
        bind(sock, (sockaddr*)&self_addr,sizeof(sockaddr));
        std::cout<<"UDP  transmission"<<std::endl;
        std::thread R = std::thread(receive_udp, sock);//std::thread(receive,sock);
        std::thread W = std::thread(send_udp, sock, (struct sockaddr*)&serv_addr);//std::thread(tweet, sock);

        W.join();
        R.join();
    } else
    {
        while (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1);
        std::cout<<"TCP transmission"<<std::endl;
        std::thread R = std::thread(receive,sock);
        std::thread W = std::thread(tweet, sock);

        W.join();
        R.join();
    }






    //关闭套接字
    close(sock);
    return 0;
}
