#include <iostream>

#include <unistd.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

int main() {
    int ret = mkfifo("myfifo", 0666);
    return 0;
}
