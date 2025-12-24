#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUF_SIZE 1024

int main(void)
{
    int server_fd, client_fd;
    struct sockaddr_in addr;
    char buf[BUF_SIZE];
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("Server listening on port %d...\n", PORT);
    client_fd = accept(server_fd, NULL, NULL);

    if (client_fd < 0) {
        perror("accept");
        close(server_fd);
        return 1;
    }

    memset(buf, 0, sizeof(buf));
    ssize_t rcv = recv(client_fd, buf, BUF_SIZE - 1, 0);

    if (rcv < 0) {
        perror("recv");
    } else {
        buf[rcv] = '\0';
        printf("Client sent: %s\n", buf);
        send(client_fd, buf, strlen(buf), 0);
    }

    close(client_fd);
    close(server_fd);
    return 0;
}
 
