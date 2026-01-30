#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

void send_looped(int fd, const void *buf, size_t sz) // Sending data untill all of the buffer items have been sent
{
    const char *ptr = buf;
    size_t remain = sz;
    while (remain > 0) 
    {
        ssize_t sent = write(fd, ptr, remain);
        if (sent == -1) 
        {
            perror("write()");
            exit(1);
        }
        ptr += sent;
        remain -= sent;
    }
}

void send_message(int fd, const char *buf) // Used to get a message over a socket
{
    uint32_t len = htonl(strlen(buf));
    send_looped(fd, &len, sizeof(len));
    send_looped(fd, buf, strlen(buf));
}

int valid_floors(const char* floors)   // Used for finding valid basement peramiter
{
    int check = 0;
    if (strlen(floors) > 3)
    {
        check = -1;
    }
    else if (isalpha(floors[0]) && isdigit(floors[1]) && isdigit(floors[2]))
    {
        if (floors[0] != 'B' || isalpha(floors[1]) || isalpha(floors[2]))   // Checking if the first value is B and the others are digits
        {
            check = -1;
        }
    }
        else if (isalpha(floors[1]) || isalpha(floors[2]))  // Making sure that the last 2 values are number not letters eg. B 12 not B BA etc.
    {
        check = -1;
    }
    return check;
}

int main(int argc, char **argv)
{

    if (argc != 3)  // Checking for correct inputs for file
    {
        fprintf(stderr, "Usage: %s {source floor} {destination floor}\n", argv[0]);
        exit(1);
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);   // AF_INET is IPv4 and SOCK_STREAM is TCP, 0 is default protocol
    if (sockfd == -1) {
        perror("socket()");
        exit(1);
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr)); // Setting the memory to 0
    addr.sin_family = AF_INET;  // IPv4
    addr.sin_port = htons(3000); // Setting port
    const char *ipaddress = "127.0.0.1";    // Setting ip address
    if (inet_pton(AF_INET, ipaddress, &addr.sin_addr) != 1) 
    {
        fprintf(stderr, "inet_pton(%s)\n", ipaddress);
        exit(1);
    }
    if (connect(sockfd, (const struct sockaddr *)&addr, sizeof(addr)) == -1) // Attempting to connect to server
    {
        fprintf(stderr, "Unable to connect to elevator system.\n");
        close(sockfd);
        exit(1);
    }
a
    

    if (valid_floors(argv[1]) == -1 || valid_floors(argv[2]) == -1) //Checking for valid floor input
    {
        printf("Invalid floor(s) specified.\n");
        exit(1);
    }
    if (argc < 3)   // Checking to make sure there are the extra 2 inputs
    {
        fprintf(stderr, "Usage: {lowest floor} {Highest floor}\n");
        exit(1);
    }
    if (strcmp(argv[1], argv[2]) == 0)  // Checking for same floor input
    {
        printf("You are already on that floor!\n");
        exit(1);
    }

    char buf[256]; // Can be any value for the buffer desired
    sprintf(buf, "RECV: CALL %.3s %.3s : Car is arriving.\n", argv[1], argv[2]);
    printf("%s", buf);

    //send_message(sockfd, buf);
    close(sockfd);

    return 0;
}