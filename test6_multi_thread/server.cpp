/* server.cpp
 * 用于实现服务器端功能
 */
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <pthread.h>

using namespace std;

#define BUFFER_SIZE 1024   //缓冲区大小
#define LISTEN_PORT 8001   //监听端口号
#define SERVER_PORT 8000   //服务端口号

void *ThreadFunc(void *arg);  //子线程启动函数

int main() {
    //创建监听套接字
    int listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //配置服务器地址信息
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;                 //设置地址簇
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);  //转换IP地址的数据形式
    servaddr.sin_port = htons(SERVER_PORT);        //设置服务器地址的端口号
    //绑定协议地址
    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    //开始监听
    listen(listenfd, LISTEN_PORT);
    //客户端套接字
    struct sockaddr_in cliaddr;          //客户端地址信息
    socklen_t length = sizeof(cliaddr);  //客户端地址信息大小
    //开始循环接收连接
    while(1) {
        //接受连接（阻塞）
        int conn = accept(listenfd, (struct sockaddr *)&cliaddr, &length);
        int *connfd = &conn;
        //返回连接成功信息
        char buf[sizeof("255.255.255.255") + 1];
        cout << "System : " << "Get connection from " 
             << inet_ntop(AF_INET, &cliaddr.sin_addr, buf, sizeof(buf))
             << ":" << ntohs(cliaddr.sin_port) << endl;
        //开辟新的线程，为当前连接的客户端提供服务
        pthread_t tid;  //子线程ID
        pthread_create(&tid, NULL, ThreadFunc, (void*)connfd);
        cout << "System : " << "TID: " << tid << endl;
    }
    close(listenfd);  //关闭监听句柄
    return 0;
}

void *ThreadFunc(void *arg) {
    /* 子线程启动函数
     * 参数1：连接句柄指针
     */
    char buffer[BUFFER_SIZE];          //缓冲区数组
    char *sendBuf;         //发送缓冲区
    int connfd = *(int *)arg;          //连接句柄
    pthread_t mytid = pthread_self();  //自身线程ID
    pthread_detach(mytid);             //与主线程分离
    while(1) {                         //开始循环读
        memset(buffer, 0, sizeof(buffer));                   //将缓冲区清空
        int len = recv(connfd, buffer, sizeof(buffer), 0);   //读取数据
        if(len <= 0 || strcmp(buffer, "/exit\n") == 0) {     //若读取无效或连接断开
            cout << "System : "                              //打印退出信息
                 << mytid << " was exited" << endl;  
            break;                                           //退出
        } else {                                             //如果读取成功
            cout << mytid << " : " << buffer;  //打印消息
            // char str[] = "formServer->";
            // sendBuf = strcat(str, buffer);
            send(connfd, buffer, strlen(buffer), 0);  //发送消息
        }
    }
    close(connfd);  //关闭连接句柄
    return NULL;
}
