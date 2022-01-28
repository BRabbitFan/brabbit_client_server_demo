/* serverTCPUDP.cpp
 * 用于实现服务器端功能
 */
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <sys/epoll.h>

using namespace std;

#define BUFFER_SIZE 1024   //缓冲区大小
#define LISTEN_PORT 8001   //监听端口号
#define SERVER_PORT 8000   //服务端口号
#define MAX_CON_NUM 100    //最大允许连接数

int main() {
    //配置服务器地址信息
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;                 //设置地址簇
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);  //转换IP地址的数据形式
    servaddr.sin_port = htons(SERVER_PORT);        //设置服务器地址的端口号
    //初始化TCP套接字
    int listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    bind(listenfd, (const sockaddr *)&servaddr, sizeof(servaddr));
    listen(listenfd, LISTEN_PORT);
    //初始化UDP套接字
    int udpfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    bind(udpfd, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    //初始化客户端地址信息
    struct sockaddr_in cliaddr;          //客户端地址信息
    socklen_t length = sizeof(cliaddr);  //客户端地址信息大小
    char buffer[BUFFER_SIZE];            //缓冲区：1024字节
    char addrBuf[sizeof("255.255.255.255") + 1];  //地址缓冲区，用于打印客户端地址
    //初始化多路IO复用
    int maxi = 0;                            //轮询描述符集合最大范围
    int epfd = epoll_create(MAX_CON_NUM);    //创建epoll句柄
    struct epoll_event event;                //用于构造关注对象
    struct epoll_event revent[MAX_CON_NUM];  //关注对象的状态集合
        //将监听描述符注册至epfd
    event.data.fd = listenfd;                //设置关注对象的描述符为监听描述符
    event.events = (EPOLLIN | EPOLLET);               //设置关注对象的关注事件为EPOLLIN边缘触发模式
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &event);  //将监听描述符注册至epfd
        //将UDP描述符注册至epfd
    event.data.fd = udpfd;
    event.events = (EPOLLIN | EPOLLET);
    epoll_ctl(epfd, EPOLL_CTL_ADD, udpfd, &event);
    //开始循环处理描述符
    while(true) {
        //获得以准备好的描述符数量
        int nready = epoll_wait(epfd, revent, maxi + 1, -1);
        //循环处理每个关注且可用的描述符
        for (int i = 0; i < nready; ++i) {
            if (revent[i].events & (EPOLLIN | EPOLLET)) {
                //处理监听描述符
                if (revent[i].data.fd == listenfd) {
                    //接受客户端的连接，并将对应的描述符注册至epfd
                    event.data.fd = accept(listenfd, (struct sockaddr *)&cliaddr, &length);
                    event.events = (EPOLLIN | EPOLLET);
                    epoll_ctl(epfd, EPOLL_CTL_ADD, event.data.fd, &event);
                    maxi++;  //更新集合最大范围
                    //返回连接成功信息
                    cout << "System : " << "Get connection from: " 
                         << inet_ntop(AF_INET, &cliaddr.sin_addr, addrBuf, sizeof(addrBuf))
                         << " ; ID: " << event.data.fd << endl;
                //处理UDP描述符
                } else if (revent[i].data.fd == udpfd) {        
                    memset(buffer, 0, BUFFER_SIZE);                   //将缓冲区清空
                    recvfrom(udpfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cliaddr, &length);//读取数据
                    cout << "[" << inet_ntop(AF_INET, &cliaddr.sin_addr, addrBuf, sizeof(addrBuf))
                         << ":" << ntohs(cliaddr.sin_port) << "] : " << buffer;
                //处理连接描述符
                } else {
                    //读取消息
                    memset(buffer, 0, sizeof(buffer));                                     //将缓冲区清空
                    int len = recv(revent[i].data.fd, buffer, BUFFER_SIZE, MSG_DONTWAIT);  //读取数据
                    //若读取错误或连接断开，则退出
                    if(len <= 0 || strcmp(buffer, "/exit\n") == 0) {
                        shutdown(revent[i].data.fd, SHUT_RDWR);                                   //关闭该描述符
                        epoll_ctl(epfd, EPOLL_CTL_DEL, revent[i].data.fd, &event);  //将描述符从edfp中删除
                        maxi--;                                                     //更新集合最大范围
                        cout << "System : " << revent[i].data.fd << " was exited" << endl; //打印退出信息
                    //如果读取成功，则打印消息
                    } else {
                        cout << revent[i].data.fd << " : " << buffer;               //打印消息
                    }
                }
            }
        }
    }
    return 0;
}
