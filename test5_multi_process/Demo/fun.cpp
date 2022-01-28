ssize_t WritenN(int fd, const void *buf, size_t len);  //完全写函数
ssize_t BufRead(int fd, char *charactor);  //静态缓冲区读单字节


ssize_t WritenN(int fd, const void *buf, size_t len) {
    /* 完全写函数；重复调用write函数，直到将缓冲区所有内容写入文件中
     * 变量1：文件操作符； 变量2：缓冲数组； 变量3：消息字节数
     */
    size_t nleft = len;  //剩余的字节数
    ssize_t nwritten;    //一次写入的字节数
    const char *ptr = (const char *)buf;  //缓冲区偏移指针
    //开始循环写入
    while (nleft > 0) {  //当有剩余的字节没有被写入时，继续循环
        if ((nwritten = write(fd, ptr, nleft)) <= 0) {  //当写入出错时，开始判断错误类型
            if (errno == EINTR) {  //如果因为系统信号而中断
                nwritten = 0;      //则应该恢复
            } else {               //如果是函数本身调用出错
              return -1;           //则应该返回
            }
        }
        nleft -= nwritten;  //减少剩余字节数
        ptr += nwritten;    //偏移缓冲区指针
    }
    return len;
}

ssize_t BufRead(int fd, char *charactor) {
    /* 静态缓冲区读单字节函数；重复调用read函数，直到将缓冲区内所有内容读出
     * 变量1：文件操作符； 变量2：用于存放读出的字节
     */
    static char *bufPtr;               //静态缓冲区指针
    static char readBuf[BUFFER_SIZE];  //静态缓冲区
    static int bufLeft = 0;            //缓冲区中剩余字符数
    //开始循环读取数据
    if (bufLeft <= 0) { //如果缓冲区为空，则开始循环读
        while ((bufLeft = read(fd, readBuf, sizeof(readBuf))) < 0) {  //当读取数据出错时，开始判断错误类型
            if (errno != EINTR) {  //如果因为系统信号而中断
                return -1;
            }
        }
        if (bufLeft == 0) {  //如果缓冲区没有剩余字符数
            return 0;        //则返回
        }
        bufPtr = readBuf;    //缓冲区指针指向静态缓冲区
    }
    //从缓冲区中取字符
    while (bufLeft > 0) {
        bufLeft --;              //剩余字符数递减
        *charactor = *bufPtr++;  //取出的字串通过参数传递出去
    }
        return sizeof(readBuf);                //返回读取字节数
}

