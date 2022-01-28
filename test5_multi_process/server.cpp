/* server.cpp
 * 用于实现服务器端功能
 */
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

using namespace std;

#define BUFFER_SIZE 1024   //缓冲区大小
#define LISTEN_PORT 8001   //监听端口号
#define SERVER_PORT 8000   //服务端口号

void sig_chld(int signo);  //信号处理函数

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
    //客户端套接字
    char buffer[BUFFER_SIZE];            //缓冲区：1024字节
    struct sockaddr_in cliaddr;          //客户端地址信息
    socklen_t length = sizeof(cliaddr);  //客户端地址信息大小
    //设置信号集，捕获SIGCHLD信号
    struct sigaction act, oact;       //sigaction结构体，分别用于传入传出
    act.sa_handler = sig_chld;        //设置信号处理指针：sig_chld函数
    sigemptyset(&act.sa_mask);        //初始化act的信号掩码集为空：即不屏蔽任何信号
    act.sa_flags = SA_RESTART;        //设置参数：服务重启选项
    sigaction(SIGCHLD, &act, &oact);  //注册信号处理行为：捕获SIGCHLD信号
    //开始循环接收连接
    while(1) {
        //接受连接（阻塞）
        int connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &length);
        //返回连接成功信息
        char buf[sizeof("255.255.255.255") + 1];
        cout << "System : " << "Get connection from " 
             << inet_ntop(AF_INET, &cliaddr.sin_addr, buf, sizeof(buf))
             << ":" << ntohs(cliaddr.sin_port) << endl;
        //开辟新的进程，为当前连接的客户端提供服务
        int pid, cpid, stat;        //子进程的进程ID、状态ID、状态指针
        if((pid = fork()) == 0) {   //开辟子进程
            //以下为子进程的代码
            close(listenfd);        //子进程关闭监听套接字描述符
            while(1) {              //开始循环读
                memset(buffer, 0, sizeof(buffer));                  //将缓冲区清空
                int len = recv(connfd, buffer, sizeof(buffer), 0);  //读取数据
                if(len <= 0 || strcmp(buffer, "/exit\n") == 0) {    //若读取无效或连接断开
                    cout << "System : "                             //打印退出信息
                         << getpid() << " was exited" << endl;  
                    break;                                          //退出
                } else {                                            //如果读取成功
                    cout << getpid() << " : " << buffer;            //打印消息
                }
            }
            close(connfd);       //子进程关闭连接套接字描述符
            return 0;
        } else {
            //以下为父进程的代码
            close(connfd);       //关闭父进程连接套接字操作符
            cout << "System : " << "PID: " << pid << endl;
        }
    }
    close(listenfd);             //关闭父进程监听套接字操作符
    return 0;
}

void sig_chld(int signo) {
    /* 信号处理函数
     * 参数1：处理信号
     */
    pid_t cpid;
    int stat;
    //循环等待并杀死僵尸进程，若无僵尸进程则跳出循环
    while((cpid = waitpid(-1, &stat, WNOHANG)) > 0);
    return;
}

