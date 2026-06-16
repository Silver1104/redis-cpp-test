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
#include <thread> 
#include <chrono>

static void msg(std::string_view msg) {
    std::cerr<<msg<<std::endl;
}

static void die(std::string_view msg) {
    int err = errno;
    std::cerr<<err<<" "<<msg<<std::endl;
    abort();
}

const size_t k_max_msg = 4096;

static int32_t read_full(int fd, void *buf, size_t n){
    
    char *buf_ptr = static_cast<char*>(buf);
    while(n > 0){
        ssize_t rv = read(fd, buf_ptr, n);
        if(rv <= 0)
            return -1;
        assert(size_t(rv) == n);
        n -= size_t(rv);
        buf_ptr += rv;
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

int32_t query(int fd, const std::string_view &message){

    uint32_t len = static_cast<uint32_t>(message.size());
    std::vector<char> wbuf (4 + len);
    memcpy(wbuf.data(), &len, 4);
    memcpy(wbuf.data() + 4, message.data(), len);
    int32_t err = write_all(fd, wbuf.data(), 4+len);
    if(err)
        return err;

    std::vector<char> rbuf (4 + k_max_msg);
    errno = 0;

    err = read_full(fd, rbuf.data(), 4);
    if(err){
        msg(errno == 0 ? "EOF" : "read() error"); 
        return err;
    }

    memcpy(&len, rbuf.data(), 4);
    if(len > k_max_msg){
        msg("Message too long");
        return -1;
    }
    
    err = read_full(fd, rbuf.data() + 4, len);
    if(err){
        msg("read() error");
        return -1;
    }

    std::string request(rbuf.data() + 4, len);

    std::cout << "Server says: " << request <<std::endl;
    return 0;
   
}

int main(){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd<0){
        die("socket()");
    }
    
    int val{};
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)); 

    struct sockaddr_in addr = {};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
   
    int rv = connect(fd, (const struct sockaddr*)&addr, sizeof(addr)); 
    
    if(rv){
        die("connect()");
    }

    int32_t err = query(fd, "hello1");
    if (err) {
        goto L_DONE;
    }
    std::this_thread::sleep_for(std::chrono::seconds(3));
    err = query(fd, "hello2");
    if (err) {
        goto L_DONE;
    }
    std::this_thread::sleep_for(std::chrono::seconds(3));
    err = query(fd, "hello3");
    if (err) {
        goto L_DONE;
    }
   
L_DONE:
    
    close(fd);
    return 0;

}

