#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

/**
    Process
        |
        v
    Process File Descriptor Table
        |
        v
    Open File Table
        |
        v
    Inode Table
 */

/**
 * Who has the file open? → FD table
 * How is this particular open instance being used? → Open file table
 * What is the file itself? → Inode table
 */

/**
 * https://www.codequoi.com/images/file-descriptor-c/file_descriptors_fr.drawio-1.png
 * 
 * Two processes can of course have the same file open: process A can access file B through its descriptor 4, 
 * and the same is true for process B and its descriptor 3. This fact can help facilitate inter-process communication.
 * 
 * A process can also have two or more references to the same file, as with process C. This can happen when we open the same file twice.
 */

int main()
{
    int fd1 = open("../../data/data.txt", O_RDONLY);
    int fd2 = dup(fd1);
    int fd3 = open("../../data/data.txt", O_RDONLY);

    printf("PID = %d\n", getpid());
    printf("FD  = %d\n", fd1);
    printf("FD2 = %d\n", fd2);
    printf("FD3 = %d\n", fd3);

    char c;

    read(fd1, &c, 1);

    printf("%lld\n", (long long)lseek(fd1, 0, SEEK_CUR)); // 1
    printf("%lld\n", (long long)lseek(fd3, 0, SEEK_CUR)); // 0

    getchar();
}

// lsof -p <pid>
// lsof -i ../../data/data.txt