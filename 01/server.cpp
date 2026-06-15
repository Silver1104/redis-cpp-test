#include <sys/socket.h> //creating socket 
#include <stdint.h> 
#include <stdlib.h>
#include <string.h>
#include <string>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h> //for htons
#include <netinet/ip.h> //for sockaddr_in and in_addr
#include <iostream>

static void die(const char *msg) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

void do_something(int connfd){
   while(true){
        std::string rbuf(64, '\0');
        ssize_t n = read(connfd, rbuf.data(), rbuf.size());

        if(n < 0)
            die("read() error");
        if(n == 0)
            break;
        rbuf.resize(n);
        if(rbuf == "Client terminated")
            break;

        std::cerr<<"Client says: "<<rbuf<<std::endl;


        std::cout<<"Write a message for client or write '/N' to close conversation: "<<std::endl;
        std::string wbuf;
        std::cin>>wbuf;

        while(!wbuf.size()){
            std::cout<<"String cannot be empty. Retry: "<<std::endl;
            std::cin>>wbuf;
        }
        if(wbuf == "/N"){
            std::string alert = "Server terminated";
            write(connfd, alert.data(), alert.size());
            break;
        }
        write(connfd, wbuf.data(), wbuf.size());
   }
}

int main(){
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if(fd < 0) die("socket()");

    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR , &val, sizeof(val));

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int rv = bind(fd, (const struct sockaddr*)&addr, sizeof(addr));
    if(rv)
        die("bind()");

    rv = listen(fd, SOMAXCONN);
    if(rv)
        die("listen()");

    while(true){
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr*)&client_addr, &addrlen);
        if(connfd < 0){
            die("accept()");
            continue;
        }
        else{
            std::cout<<"Connected to client"<<std::endl;
        }
        do_something(connfd);
        close(connfd);

    }

}
