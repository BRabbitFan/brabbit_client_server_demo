/*
 * client.cpp
 * 用于实现客户端功能
 */
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

#define BUF_LEN 1024                //缓冲字符串长度
#define BUF_FILE "a"                //缓冲文件名

int UnLock(int file);               //解锁函数；参数1：缓冲文件标识符
int WriteLock(int file);            //写锁函数；参数1：缓冲文件标识符

int main() {
    int nByte = 0;                  //记录所输入字符串的字节数
    char buf[BUF_LEN] = {0};        //缓冲字符串，用于存放输入的信息
    int fileA = open(BUF_FILE, O_WRONLY);  //打开缓冲文件
    if (fileA <= 0) {               //判断文件是否成功打开
        puts("Open file failed!");
        return -1;
    }
    do {
        bzero(buf, BUF_LEN);        //将缓冲数组中的信息清空
        cin >> buf;                 //输入信息至缓冲数组
        WriteLock(fileA);           //给文件上写锁
        lseek(fileA, 0, SEEK_SET);  //将文件偏移量置零
        nByte = write(fileA, buf, strlen(buf));  //将信息输入缓冲文件
        ftruncate(fileA, nByte);    //将此次输入信息以外的多于信息删除
        fsync(fileA);               //同步文件数据到磁盘
        UnLock(fileA);              //解锁缓冲文件
    } while (buf[0] != '\t');       //只输入回车则跳出循环
    ftruncate(fileA, 0);            //将缓冲文件中的内容清空
    return 0;
}

int UnLock(int file) {
    /*解锁函数；参数1：缓冲文件标识符*/
    struct flock lock;              //创建锁结构体实例
    lock.l_type = F_UNLCK;          //锁的类型为“解锁”
    lock.l_start = 0;               //whence的偏移量为0
    lock.l_whence = SEEK_SET;       //锁的区域从文件首部开始
    lock.l_len = 0;                 //锁的长度为整个文件
    return fcntl(file, F_SETLKW, &lock);  //加阻塞锁
}

int WriteLock(int file) {
    /*写锁函数；参数1：缓冲文件标识符*/
    struct flock lock;              //创建锁结构体实例
    lock.l_type = F_WRLCK;          //锁的类型为“写锁”
    lock.l_start = 0;               //whence的偏移量为0
    lock.l_whence = SEEK_SET;       //锁的区域从文件首部开始
    lock.l_len = 0;                 //锁的长度为整个文件
    return fcntl(file, F_SETLKW, &lock);  //加阻塞锁
}
