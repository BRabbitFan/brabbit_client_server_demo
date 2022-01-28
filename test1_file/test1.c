#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int file, val;
    char str[50] = {0};
    //打开文件“Foo”，若不存在则创建
    file = open("Foo", O_CREAT | O_RDWR, 0777);
    //将文本写入文件
    file = open("Foo", O_RDWR | O_APPEND);
    write(file, "0123451234", 10);
    //将文件中的内容读取并打印至屏幕
    file = open("Foo", O_RDWR);
    val = read(file, str, 100);
    str[val] = '\0';
    printf("%s\n", str);
    //将文本写入文件
    file = open("Foo", O_RDWR | O_APPEND);
    write(file, "0123451234", 10);
    //将文件中的内容读取并打印至屏幕
    file = open("Foo", O_RDWR);
    val = read(file, str, 100);
    str[val] = '\0';
    printf("%s\n", str);
    //修改文件中的字段
    file = open("Foo", O_RDWR | O_TRUNC);
    write(file, "98765432100123456789", 20);
    //将文件中的内容读取并打印至屏幕
    file = open("Foo", O_RDWR);
    val = read(file, str, 20);
    str[val] = '\0';
    printf("%s\n", str);
    close(file);
    return 0;
}
