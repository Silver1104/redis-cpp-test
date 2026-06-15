#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <string>
#include <iostream>

static void die(const char *msg) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

int main(){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0)
        die("socket()");
    
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int rv = connect(fd, (const struct sockaddr*)&addr, sizeof(addr));

    if(rv)
        die("connect()");
    else
        std::cout<<"Connected to server"<<std::endl;
    
    while(true){
        std::string msg;
        std::cout<<"Enter a message for server or '/N' to end conversation: ";
        std::cin>>msg;
        while(!msg.size()){
            std::cout<<"Message cannot be empty: ";
            std::cin>>msg;
        }
        
        if(msg == "/N"){
            std::string alert = "Client terminated";
            write(fd, alert.data(), alert.size());
            break;
        }

        write(fd, msg.data(), msg.size());

        std::string rbuf(64, '\0');

        int n = read(fd, rbuf.data(), rbuf.size());

        if(n < 0)
            die("read() error");

        if(n == 0)
        break;

        rbuf.resize(n);

        if(rbuf == "Server terminated")
            break;

        std::cerr<<"Server says: "<<rbuf<<std::endl;
    }
    close(fd);
    return 0;
}
