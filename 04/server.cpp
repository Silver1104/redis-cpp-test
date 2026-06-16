#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <vector>
#include <iostream>
#include <string.h>

static void msg(const std::string_view &msg) { 
    std::cerr<<msg<<std::endl;
}

static void die(const std::string_view &msg) {
    int err = errno;
    std::cerr<<err<<" "<<msg<<std::endl;
    abort();
}

const size_t k_max_msg = 4096;

static int32_t read_full(int fd, void *buf, size_t n){
    
    char *buf_ptr = static_cast<char*>(buf);
    while(n > 0){
        ssize_t rv = read(fd, buf_ptr, n);
        if(rv < 0){
            if(errno == EINTR) continue; //if there is a keyboard interrupt continue reading
            return -1;
        }
        if(rv == 0) return -1; //EOF
        assert(size_t(rv) == n);
        n -= size_t(rv);
        buf_ptr += rv; //keep moving buffer pointer forward
    }
    return 0;
}

static int32_t write_all(int fd, const void *buf, size_t n){
    const char *buf_ptr = static_cast<const char*>(buf);
    while(n > 0){
        ssize_t rv = write(fd, buf_ptr, n);
        if(rv <= 0)
            return -1;
        assert(size_t(rv) == n);
        n -= size_t(rv);
        buf_ptr += rv;
    }
    return 0;
}

int32_t one_request(int connfd){

    std::vector<char> rbuf (4 + k_max_msg);
    errno = 0;

    int32_t err = read_full(connfd, rbuf.data(), 4);
    if(err){
        msg(errno == 0 ? "EOF" : "read() error"); 
        return err;
    }

    uint32_t len{};
    memcpy(&len, rbuf.data(), 4);
    if(len > k_max_msg){
        msg("Message too long");
        return -1;
    }
    
    err = read_full(connfd, rbuf.data() + 4, len);
    if(err){
        msg("read() error");
        return -1;
    }

    std::string request(rbuf.data() + 4, len);

    std::cout << "Client says: " << request <<std::endl;

    const std::string reply {"world"};
    len = static_cast<uint32_t>(reply.size());
    std::vector<char> wbuf (4 + len);
    memcpy(wbuf.data(), &len, 4);
    memcpy(wbuf.data() + 4, reply.data(), len);
    return write_all(connfd, wbuf.data(), 4+len);

}

int main(){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(!fd){
        die("socket()");
    }
    
    int val{};
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)); 

    struct sockaddr_in addr = {};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
   
    int rv = bind(fd, (const sockaddr*)&addr, sizeof(addr));
    if(rv){
        die("bind()");
    }

    rv = listen(fd, SOMAXCONN);
    if(rv){
        die("listen()");
    }

    while(true){
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen);
        if(connfd < 0)
            continue;
        while(true){
            int32_t err = one_request(connfd);
            if(err)
                break;
        }
        close(connfd);
    }
}
