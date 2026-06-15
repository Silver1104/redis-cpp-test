#include <iostream>
#include <sys/socket.h>
#include <iostream>
#include <cstring>      // strlen
#include <cstdint>      // uint16_t, uint32_t
#include <unistd.h>     // read, write, close
#include <arpa/inet.h>  // htons, htonl
#include <netinet/in.h>
                         
/*struct in_addr {
    uint32_t s_addr;
};

struct sockaddr_in{
    uint16_t sin_family;
    uint16_t sin_port;
    struct in_addr sin_addr;
};*/

void msg(const char* s)
{
    std::cerr << s << '\n';
}

void die(const char* s)
{
    perror(s);
    exit(1);
}

static void do_something(int connfd){
    char rbuf[64] = {};
    ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 2);
    if (n < 0) {
        msg("read() error");
        return;
    }
    printf("client says: %s\n", rbuf);

    char wbuf[] = "world";
    write(connfd, wbuf, strlen(wbuf));
}

int main(){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);        // port
    addr.sin_addr.s_addr = htonl(0);    // wildcard IP 0.0.0.0
    int rv = bind(fd, (const struct sockaddr*)&addr, sizeof(addr));
    if (rv) { die("bind()"); }

    //listen
    rv = listen(fd, SOMAXCONN);
    if (rv) { die("listen()"); }

    while(true){
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr*)&client_addr, &addrlen);
        if (connfd < 0)
            continue;
        do_something(connfd);
        close(connfd);

    }
}


