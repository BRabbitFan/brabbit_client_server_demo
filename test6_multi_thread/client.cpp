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

#include <pthread.h>

using namespace std;

#define BUFFER_SIZE 1024
#define SERVER_PORT 8000
const char *SERVER_IP = "101.132.165.52";  //服务器IP地址

void *ThreadFunc(void *arg) {
    char buffer[BUFFER_SIZE];          //缓冲区数组
    int connfd = *(int *)arg;          //连接句柄
    pthread_t mytid = pthread_self();  //自身线程ID
    pthread_detach(mytid);             //与主线程分离
    while(1) {                         //开始循环读
        memset(buffer, 0, sizeof(buffer));                   //将缓冲区清空
        int len = recv(connfd, buffer, sizeof(buffer), 0);   //读取数据
        if(len <= 0 || strcmp(buffer, "/exit\n") == 0) {     //若读取无效或连接断开
            cout << "System was exited" << endl;  
            break;                                           //退出
        } else {                                             //如果读取成功
            cout << buffer;  //打印消息
        }
    }
    close(connfd);  //关闭连接句柄
    return NULL;
}

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
    char buffer[BUFFER_SIZE];                //输入缓冲区
    char buf[sizeof("255.255.255.255") + 1];  //用于打印服务器信息
    memset(sendBuf, 0, sizeof(sendBuf));      //开始循环前清空一次缓冲区
    while (fgets(sendBuf, sizeof(sendBuf), stdin) != NULL) {
        send(fd, sendBuf, strlen(sendBuf), 0);  //发送消息
        // if (strcmp(sendBuf, "/exit\n") == 0) {  //如果输入/exit
        //     cout << endl << "You are exit!" << endl;
        //     break;                              //则退出
        // } else {
        //     cout << "Send : " << sendBuf << endl;
        // }
        // memset(sendBuf, 0, sizeof(sendBuf));    //清空缓冲区

        // int len = recv(fd, buffer, sizeof(buffer), 0);   //读取数据
        // if(len <= 0 || strcmp(buffer, "/exit\n") == 0) {     //若读取无效或连接断开
        //     cout << "Recv : System was exited" << endl;
        //     break;                                           //退出
        // } else {                                             //如果读取成功
        //     cout << "Recv : " << buffer;  //打印消息
        // }
        // memset(buffer, 0, sizeof(buffer));                   //将缓冲区清空
    }
    cout << "NULL";
    close(fd);
    return 0;
}
