/* server.cpp
 * 用于实现服务器端功能
 */
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>
#include <poll.h>

using namespace std;

#define BUFFER_SIZE 1024   //缓冲区大小
#define LISTEN_PORT 8001   //监听端口号
#define SERVER_PORT 8000   //服务端口号
#define MAX_CON_NUM 100     //最大允许连接数

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
    struct pollfd fdset[MAX_CON_NUM];    //描述符池
    int i;                               //用于定位池的位置
    int fullCon = 0;                     //池状态，0：未满，1：已满
    for (i = 0; i < MAX_CON_NUM; ++i) {  //初始化池内pollfd的描述符为-1
        fdset[i].fd = -1;
    }
    fdset[0].fd = listenfd;              //设置0号pollfd的描述符为监听描述符
    fdset[0].events = POLLIN;            //设置0号pollfd的关注状态
    int maxi = 0;                        //轮询描述符集合最大范围
    //开始循环处理描述符
    while(true) {
        //获得以准备好的描述符数量
        int nready = poll(fdset, maxi + 1, -1);
        //处理监听描述符（若可用）
        if (fdset[0].revents & POLLIN) {
            //定位到连接池第一个闲置状态的描述符
            for (i = 1; i <= MAX_CON_NUM; ++i) {
                if (fdset[i].fd < 0) { 
                    break; 
                }
            }
            //若描述符池满且状态未更新，则报错后更新状态
            if (i == MAX_CON_NUM && fullCon == 0) {
                cout << "System : " << "Full connections!" << endl;
                fullCon = 1;
            //若池未满，则将描述符加入池中
            } else if (i != MAX_CON_NUM && fullCon == 0) {
                //将获得连接的描述放入池中i号pollfd中，并且设置关注状态
                fdset[i].fd = accept(listenfd, (struct sockaddr *)&cliaddr, &length);
                fdset[i].events = POLLIN;
                //返回连接成功信息
                char buf[sizeof("255.255.255.255") + 1];
                cout << "System : " << "Get connection from: " 
                     << inet_ntop(AF_INET, &cliaddr.sin_addr, buf, sizeof(buf))
                     << " ; ID: " << fdset[i].fd << endl;
                //更新轮询描述符集合最大范围
                if (i > maxi) {
                    maxi = i;
                }
                //若描述符处理完毕则下次轮询
                if (--nready <= 0) {
                    continue;
                }
            }
        }
        //处理连接描述符（若可用）
        for (i = 1; i <= maxi; ++i) {
            //若描述符是闲置状态则跳过
            if (fdset[i].fd < 0) {
                continue; 
            }
            //若描述符非闲置且可用，则开始读数据
            if (fdset[i].revents & (POLLIN | POLLERR)) {
                memset(buffer, 0, sizeof(buffer));                       //将缓冲区清空
                int len = recv(fdset[i].fd, buffer, BUFFER_SIZE, MSG_DONTWAIT);  //读取数据
                //若读取错误或连接断开
                if(len <= 0 || strcmp(buffer, "/exit\n") == 0) {
                    cout << "System : " << fdset[i].fd << " was exited" << endl; //打印退出信息
                    close(fdset[i].fd);                                  //关闭描述符
                    fdset[i].fd = -1;                                    //将连接池中操作符状态设置为闲置
                    fullCon = 0;                                         //将连接池状态设为未满
                    //更新轮询描述符集合最大范围
                    if (i == maxi) {
                        maxi--;
                    }
                //如果读取成功
                } else {
                    cout << fdset[i].fd << " : " << buffer;              //打印消息
                }
                nready--;  //将可用操作符数量-1
            }
            //若描述符处理完毕则下次轮询
            if (nready <= 0) {
                break;
            }
        }
    }
    return 0;
}
