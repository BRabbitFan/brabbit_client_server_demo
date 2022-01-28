/* server.cpp
 * 用于实现服务器端功能
 */
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>
#include <sys/select.h>

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
    int connfd[MAX_CON_NUM];  //连接池
    int i;                    //变量i用于定位连接池中的位置
    int fullCon = 0;          //连接池状态，0：未满，1：已满
    for (i = 0; i < MAX_CON_NUM; ++i) {
        connfd[i] = -1;       //初始化连接池数组为-1
    }
    int maxfd = listenfd;     //初始化最大描述符为监听描述符
    fd_set fdset;             //所关注的文件描述符集合
    FD_ZERO(&fdset);          //初始化清空描述符集合
    //开始循环处理描述符
    while(1) {
        //将需要关注的描述符添加至关注集合
        FD_SET(listenfd, &fdset);  //将监听描述符添加至集合
        for (i = 0; i < MAX_CON_NUM; ++i) {
            if (connfd[i] >= 0) {  //将池内收到连接的描述符添加至集合
                FD_SET(connfd[i], &fdset);
            }
        }
        //开始查看描述符集合的状态，等待可用状态
        select(maxfd + 1, &fdset, NULL, NULL, NULL);
        //处理监听描述符（若可用）
        if (FD_ISSET(listenfd, &fdset)) {
            //定位到连接池第一个闲置状态的描述符
            for (i = 0; i < MAX_CON_NUM; ++i) {
                if (connfd[i] < 0) { 
                    break; 
                }
            }
            //若连接池满且状态未更新，则报错后更新状态
            if (i == MAX_CON_NUM && fullCon == 0) {
                cout << "System : " << "Full connections!" << endl;
                fullCon = 1;
            //若连接池未满，则将描述符加入池中
            } else if (i != MAX_CON_NUM && fullCon == 0) {
                connfd[i] = accept(listenfd, (struct sockaddr *)&cliaddr, &length);
                //返回连接成功信息
                char buf[sizeof("255.255.255.255") + 1];
                cout << "System : " << "Get connection from: " 
                     << inet_ntop(AF_INET, &cliaddr.sin_addr, buf, sizeof(buf))
                     << " ; ID: " << connfd[i] << endl;
                //更新最大描述符
                if (connfd[i] > maxfd) {
                    maxfd = connfd[i];
                }
            }
        }
        //处理连接池内的描述符（若可用）
        for (i = 0; i <= maxfd; ++i) {
            //若描述符是闲置状态则跳过
            if (connfd[i] < 0) { 
                continue; 
            }
            //若描述符非限制且可用，则开始读数据
            if (FD_ISSET(connfd[i], &fdset)) {
                memset(buffer, 0, sizeof(buffer));                     //将缓冲区清空
                int len = recv(connfd[i], buffer, BUFFER_SIZE, MSG_DONTWAIT);  //读取数据
                //若读取错误或连接断开
                if(len <= 0 || strcmp(buffer, "/exit\n") == 0) {        
                    cout << "System : " << connfd[i] << " was exited" << endl;  //打印退出信息
                    FD_CLR(connfd[i], &fdset);                         //从关注集合中除去该描述符
                    close(connfd[i]);                                  //关闭描述符
                    connfd[i] = -1;                                    //将连接池中操作符状态设置为闲置
                    fullCon = 0;                                       //将连接池状态设为未满
                //如果读取成功
                } else {                                            
                    cout << connfd[i] << " : " << buffer;              //打印消息
                }
            }
        }
    }
    return 0;
}
