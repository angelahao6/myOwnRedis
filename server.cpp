#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

static void do_something(int connfd) {
    char rbuf[64] = {};
    ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
    if (n < 0) {
        msg("read() error");
        return;
    }
    printf("client says: %s\n", rbuf);

    char wbuf[] = "world";
    write(connfd, wbuf, strlen(wbuf)); // TODO: add error handling 
}

int main() {
    // obtain socket handle
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    // set socket options
    int val = 1; // set SO_REUSEADDR to 1 to bind immediately after port closes 
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    // bind to the wildcard address 0.0.0.0.1234
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);        // port
    addr.sin_addr.s_addr = htonl(0);    // wildcard IP 0.0.0.0
    int rv = bind(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if (rv) { die("bind()"); }
    
    // listen -- this is where socket is actually created
    rv = listen(fd, SOMAXCONN); // SOMAXCONN = size of queue (is 4096 on linux)
    if (rv) { die("listen()"); }

    // accept connections -- this is a loop to process each client connection
    while (true) {
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen);
        if (connfd < 0) {
            continue; // error
        }

        do_something(connfd);
        close(connfd);
    }

    return 0;
}
