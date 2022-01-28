/*
client.cpp
用于实现客户端功能
*/
#include <iostream>
#include <stdio.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

struct ShareBuf {
    /*缓冲区结构体*/
    sem_t sem;  //共享映射区包含一个信号量
    char buf[1024];  //真正用于共需的空间
};

ShareBuf *MemShm(const char *memName);  //创建缓冲区函数；参数1：缓冲文件名

int main() {
    ShareBuf *ptr = MemShm("shmem");  //获取文件shmem的内存映射指针
    sem_init(&ptr -> sem, 1, 1);  //基于内存的信号量初始化
    while (1) {
        sem_wait(&ptr -> sem);  //阻塞于信号量
        fgets(ptr -> buf, 1024, stdin);  //用户输入字符串写入存储区，未必实时写入文件
        msync(ptr, sizeof(ShareBuf), MS_SYNC);  //同步写入文件，需保护
        sem_post(&ptr -> sem);  //释放信号量
        sleep(1);  //防止过快的循环独占信号量
    }
    sem_destroy(&ptr -> sem);  //删除信号量
    shm_unlink("shmem");  //删除映射关系
    return 0;
}

ShareBuf *MemShm(const char *memName) {
    /*创建缓冲区函数；参数1：缓冲文件名*/
    //创建缓冲区结构体
    struct ShareBuf *ptr;
    //打开一个文件
    int fd = shm_open(memName, O_RDWR | O_CREAT , S_IRUSR | S_IWUSR);
    //设置文件的大小
    ftruncate(fd, sizeof(ShareBuf));
    //将文件空间映射到当前进程，返回映射成功的指针
    ptr = (ShareBuf *)mmap(NULL, sizeof(ShareBuf), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);  //映射成功后即可关闭文件
    return ptr;  //返回映射指针
}
