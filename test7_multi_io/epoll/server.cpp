/* server.cpp
 * 用于实现服务器端功能
 */
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>
#include <sys/epoll.h>

using namespace std;

#define BUFFER_SIZE 1024   //缓冲区大小
#define LISTEN_PORT 8001   //监听端口号
#define SERVER_PORT 8000   //服务端口号
#define MAX_CON_NUM 100    //最大允许连接数

int main() {
    //创建监听套接字
    int listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //配置服务器地址信息
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;                 //设置地址簇
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);  //转换IP地址的数据形式
    servaddr.sin_port = htons(SERVER_PORT);        //设置服务器地址的端口号
    //绑定协议地址
    bind(listenfd, (const sockaddr *)&servaddr, sizeof(servaddr));
    //开始监听
    listen(listenfd, LISTEN_PORT);
    //初始化客户端套接字
    char buffer[BUFFER_SIZE];            //缓冲区：1024字节
    struct sockaddr_in cliaddr;          //客户端地址信息
    socklen_t length = sizeof(cliaddr);  //客户端地址信息大小
    //初始化多路IO复用
    int maxi = 0;                            //轮询描述符集合最大范围
    int epfd = epoll_create(MAX_CON_NUM);    //创建epoll句柄
    struct epoll_event event;                //用于构造关注对象
    struct epoll_event revent[MAX_CON_NUM];  //关注对象的状态集合
    event.data.fd = listenfd;                //设置关注对象的描述符为监听描述符
    event.events = (EPOLLIN | EPOLLET);                //设置关注对象的关注事件为EPOLLIN边缘触发模式
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &event);  //将监听描述符注册至epfd
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
                    char buf[sizeof("255.255.255.255") + 1];
                    cout << "System : " << "Get connection from: " 
                         << inet_ntop(AF_INET, &cliaddr.sin_addr, buf, sizeof(buf))
                         << " ; ID: " << event.data.fd << endl;
                //处理连接描述符
                } else {
                    //读取消息
                    memset(buffer, 0, sizeof(buffer));                                     //将缓冲区清空
                    int len = recv(revent[i].data.fd, buffer, BUFFER_SIZE, MSG_DONTWAIT);  //读取数据
                    //若读取错误或连接断开，则退出
                    if(len <= 0 || strcmp(buffer, "/exit\n") == 0) {
                        close(revent[i].data.fd);                                   //关闭该描述符
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
