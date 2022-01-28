/* clinet.cpp
 * 用于实现客户端功能
 */
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>
#include <stdio.h>

using namespace std;

#define BUFFER_SIZE 1024
#define SERVER_PORT 8000
const char *SERVER_IP = "192.168.112.128";  //服务器IP地址

int main() {
    //创建IPv4地址族TCP套接字
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //创建sockaddr_in结构体对象
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;                    //设置地址族：IPv4
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);  //设置服务器IP
    servaddr.sin_port = htons(SERVER_PORT);           //设置端口号：8000
    //连接服务器socket地址
    cout << "Connect to " << SERVER_IP << ":" << SERVER_PORT << endl;
    if (connect(fd, (const struct sockaddr *) &servaddr, sizeof(struct sockaddr_in)) == 0) {
        cout << "Success!" << endl;
    }
    //循环发送数据
    char sendBuf[BUFFER_SIZE];                //输入缓冲区
    char buf[sizeof("255.255.255.255") + 1];  //用于打印服务器信息
    memset(sendBuf, 0, sizeof(sendBuf));      //开始循环前清空一次缓冲区
    do {
        cout << "\nMessage to:"                 //开始打印服务器信息
             << inet_ntop(AF_INET, &servaddr.sin_addr, buf, sizeof(buf))
             << ":" << ntohs(servaddr.sin_port) << "\n\t";
        send(fd, sendBuf, strlen(sendBuf), 0);  //发送消息
        if (strcmp(sendBuf, "/exit\n") == 0) {  //如果输入/exit
            cout << endl << "You are exit!" << endl;
            break;                              //则退出
        }
        memset(sendBuf, 0, sizeof(sendBuf));    //清空缓冲区
        //当输入不为空的时候继续循环
    } while(fgets(sendBuf, sizeof(sendBuf), stdin) != NULL);
    close(fd);
    return 0;
}
