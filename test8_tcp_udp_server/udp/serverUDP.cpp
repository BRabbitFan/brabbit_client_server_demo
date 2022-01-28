/* serverUDP.cpp
 * 用于实现UDP服务器端功能
 */
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>

using namespace std;

#define BUFFER_SIZE 1024   //缓冲区大小
#define SERVER_PORT 8000   //服务端口号

int main() {
    //创建监听套接字
    int recvfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    //配置服务器地址信息
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;                 //设置地址簇
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);  //转换IP地址的数据形式
    servaddr.sin_port = htons(SERVER_PORT);        //设置服务器地址的端口号
    //绑定协议地址
    bind(recvfd, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    //初始化客户端套接字、缓冲区
    struct sockaddr_in cliaddr;          //客户端地址信息
    socklen_t length = sizeof(cliaddr);  //客户端地址信息大小
    char buffer[BUFFER_SIZE];            //缓冲区：1024字节
    char addrBuf[sizeof("255.255.255.255") + 1];
    //开始循环处理描述符
    while(true) {
        memset(buffer, 0, BUFFER_SIZE);                                                  //将缓冲区清空
        recvfrom(recvfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cliaddr, &length);  //读取数据
        cout << "[" << inet_ntop(AF_INET, &cliaddr.sin_addr, addrBuf, sizeof(addrBuf))
              << ":" << ntohs(cliaddr.sin_port) << "] : "
              << buffer;
    }
    return 0;
}
