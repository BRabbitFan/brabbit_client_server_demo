/* clinetUDP.cpp
 * 用于实现UDP客户端功能
 */
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>

using namespace std;

#define BUFFER_SIZE 1024
#define SERVER_PORT 8000
const char *SERVER_IP = "192.168.112.128";  //服务器IP地址

int main() {
    //创建IPv4地址族UDP套接字
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    //创建sockaddr_in结构体对象
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;                    //设置地址族：IPv4
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);  //设置服务器IP
    servaddr.sin_port = htons(SERVER_PORT);           //设置端口号：8000
    //初始化缓冲区，创建
    char sendBuf[BUFFER_SIZE] = {0};                  //输入缓冲区
    char addrBuf[sizeof("255.255.255.255") + 1];      //地址缓冲区，用于打印服务器信息
    //开始循环发送消息
    while (true) {
        memset(sendBuf, 0, sizeof(sendBuf));     //清空缓冲区
        cout << "\nMessage to (UDP) :"                  //打印服务器端信息
             << inet_ntop(AF_INET, &servaddr.sin_addr, addrBuf, sizeof(addrBuf))
             << ":" << ntohs(servaddr.sin_port) << "\n\t";
        fgets(sendBuf, sizeof(sendBuf), stdin);  //输入消息
        sendto(fd, sendBuf, strlen(sendBuf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));  //发送消息
        if (strcmp(sendBuf, "/exit\n") == 0) {   //如果输入/exit则退出
            cout << endl << "You are exit!" << endl;
            break;
        }
    }
    shutdown(fd, SHUT_RDWR);
    return 0;
}
