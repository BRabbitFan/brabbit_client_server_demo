/* 
 * sever.cpp
 * 用于实现服务器端功能
 */
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

#define BUF_LEN 1024                     //缓冲字符串长度
#define BUF_FILE "a"                     //缓冲文件名

int UnLock(int file);                    //解锁函数；参数1：缓冲文件标识符
int ReadLock(int file);                  //写锁参数；参数1：缓冲文件标识符

int main() {
    struct stat fileStat;                //文件信息结构体
    time_t lastModify = 0;               //记录文件最后修改时间
    char buf[BUF_LEN] = {0};             //缓冲字符串，用于存放读入的信息
    int fileA = open(BUF_FILE, O_RDONLY);  //打开缓冲文件
    if (fileA <= 0) {                    //判断文件是否打开
        puts("Open file failed!");
        return -1;
    }
    while (1) {
        bzero(buf, BUF_LEN);             //将缓冲数组中的信息清空
        do {
            sleep(1);                    //睡眠1秒
            fstat(fileA, &fileStat);     //更新文件信息
        } while (lastModify == fileStat.st_mtime);  //当文件被更新则跳出循环
        lastModify = fileStat.st_mtime;  //更新最后修改时间
        ReadLock(fileA);                 //给文件上读锁
        lseek(fileA, 0, SEEK_SET);       //将文件偏移量置零
        read(fileA, buf, BUF_LEN - 1);   //将信息读入缓冲数组
        UnLock(fileA);                   //解锁缓冲文件
        cout << buf << endl;             //打印所读入的字符串
    }
    return 0;
}

int UnLock(int file) {
    /*解锁函数；参数1：缓冲文件标识符*/
    struct flock lock;                   //创建锁结构体实例
    lock.l_type = F_UNLCK;               //锁的类型为“解锁”
    lock.l_start = 0;                    //whence的偏移量为0
    lock.l_whence = SEEK_SET;            //锁的区域从文件首部开始
    lock.l_len = 0;                      //锁的长度为整个文件
    return fcntl(file, F_SETLKW, &lock);  //加阻塞锁
}

int ReadLock(int file) {
    /*读锁函数；参数1：缓冲文件标识符*/
    struct flock lock;                   //创建锁结构体实例
    lock.l_type = F_RDLCK;               //锁的类型为“读锁”
    lock.l_start = 0;                    //whence的偏移量为0
    lock.l_whence = SEEK_SET;            //锁的区域从文件首部开始
    lock.l_len = 0;                      //锁的长度为整个文件
    return fcntl(file, F_SETLKW, &lock);  //加阻塞锁
}
