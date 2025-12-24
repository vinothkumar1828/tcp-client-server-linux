#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUF_SIZE 1024
int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
        return 1;
    }

    struct addrinfo hints;
    struct addrinfo *res;
    char buf[BUF_SIZE];

    /* BUG FIX 1:
       Earlier bug:
           struct addrinfo hints;
           (used without initialization)
       Problem:
           Uninitialized memory causes getaddrinfo() to behave unpredictably.
       Fix:
           Clear the structure before setting required fields.
    */

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;     // Supports IPv4 and IPv6
    hints.ai_socktype = SOCK_STREAM;   // TCP
    int rc = getaddrinfo(argv[1], argv[2], &hints, &res);

    if (rc != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
        return 1;
    }

    /* BUG FIX 2:
       Earlier bug:
           socket(AF_INET, SOCK_STREAM, 0)
       Problem:
           getaddrinfo() may return IPv6; forcing AF_INET can break connection.
       Fix:
           Use parameters returned by getaddrinfo().
    */

    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        perror("socket");
        freeaddrinfo(res);
        return 1;
    }

    /* BUG FIX 3:
       Earlier bug:
           connect(sock, res->ai_addr, sizeof(struct sockaddr))
       Problem:
           Incorrect address length → connect() may fail.
       Fix:
           Use res->ai_addrlen.
    */

    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        perror("connect");
        close(sock);
        freeaddrinfo(res);
        return 1;
    }

    freeaddrinfo(res);

    /* BUG FIX 4:
       Earlier bug:
           memset(&buf, 0, sizeof(char*))
       Problem:
           Only clears pointer size (8 bytes), not the whole buffer.
       Fix:
           Clear entire buffer properly.
    */

    memset(buf, 0, sizeof(buf));
    printf("Enter a line to send: ");

    if (fgets(buf, BUF_SIZE, stdin) == NULL) {
        perror("fgets");
        close(sock);
        return 1;
    }

    size_t len = strlen(buf);

    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    }

    /* BUG FIX 5:

       Earlier bug:
           send(sock, buf, sizeof(buf), 0)
       Problem:
           Sends garbage bytes beyond actual message.
       Fix:
           Send only valid message length.
    */

    ssize_t sent = send(sock, buf, strlen(buf), 0);
    if (sent < 0) {
        perror("send");
        close(sock);
        return 1;
    }

    /* BUG FIX 6:
       Earlier bug:
           recv() data printed without null termination.
       Problem:
           printf reads beyond buffer → undefined behavior.
       Fix:
           Add explicit null terminator.
    */

    ssize_t rcv = recv(sock, buf, BUF_SIZE - 1, 0);

    if (rcv < 0) {
        perror("recv");
        close(sock);
        return 1;
    }

    buf[rcv] = '\0';
    printf("Server says: %s\n", buf);
    close(sock);
    return 0;
}
 
