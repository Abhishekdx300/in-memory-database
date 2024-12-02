#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <system_error>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <errno.h>

#include "common.h"



class NetworkClient {
private:
    static constexpr size_t k_max_msg = 4096;



    static void msg(const std::string& message) {
        std::cerr << message << std::endl;
    }

    [[noreturn]] static void die(const std::string& message) {
        std::error_code ec(errno, std::system_category());
        std::cerr << "[" << ec.value() << "] " << message << std::endl;
        std::abort();
    }



    static int32_t read_full(int fd, char* buf, size_t n) {
        while (n > 0) {
            ssize_t rv = read(fd, buf, n);
            if (rv <= 0) {
                return -1;  

            }
            assert(static_cast<size_t>(rv) <= n);
            n -= static_cast<size_t>(rv);
            buf += rv;
        }
        return 0;
    }


    static int32_t write_all(int fd, const char* buf, size_t n) {
        while (n > 0) {
            ssize_t rv = write(fd, buf, n);
            if (rv <= 0) {
                return -1;  
            }
            assert(static_cast<size_t>(rv) <= n);
            n -= static_cast<size_t>(rv);
            buf += rv;
        }
        return 0;
    }



    static int32_t send_req(int fd, const std::vector<std::string>& cmd) {
        uint32_t len = 4;
        for (const std::string& s : cmd) {
            len += 4 + s.size();
        }
        if (len > k_max_msg) {
            return -1;
        }

        std::vector<char> wbuf(4 + k_max_msg);
        memcpy(&wbuf[0], &len, 4); 


        uint32_t n = cmd.size();
        memcpy(&wbuf[4], &n, 4);
        size_t cur = 8;
        for (const std::string& s : cmd) {
            uint32_t p = static_cast<uint32_t>(s.size());
            memcpy(&wbuf[cur], &p, 4);
            memcpy(&wbuf[cur + 4], s.data(), s.size());
            cur += 4 + s.size();
        }
        return write_all(fd, wbuf.data(), 4 + len);
    }



    static int32_t on_response(const uint8_t* data, size_t size) {
        if (size < 1) {
            msg("bad response");
            return -1;
        }
        switch (data[0]) {
        case SER_NIL:
            std::cout << "(nil)" << std::endl;
            return 1;
        case SER_ERR:
            if (size < 1 + 8) {
                msg("bad response");
                return -1;
            }
            {
                int32_t code = 0;
                uint32_t len = 0;
                memcpy(&code, &data[1], 4);
                memcpy(&len, &data[1 + 4], 4);
                if (size < 1 + 8 + len) {
                    msg("bad response");
                    return -1;
                }
                std::cout << "(err) " << code << " " 
                          << std::string(reinterpret_cast<const char*>(&data[1 + 8]), len) 
                          << std::endl;
                return 1 + 8 + len;
            }
        case SER_STR:
            if (size < 1 + 4) {
                msg("bad response");
                return -1;
            }
            {
                uint32_t len = 0;
                memcpy(&len, &data[1], 4);
                if (size < 1 + 4 + len) {
                    msg("bad response");
                    return -1;
                }
                std::cout << "(str) " 
                          << std::string(reinterpret_cast<const char*>(&data[1 + 4]), len) 
                          << std::endl;
                return 1 + 4 + len;
            }
        case SER_INT:
            if (size < 1 + 8) {
                msg("bad response");
                return -1;
            }
            {
                int64_t val = 0;
                memcpy(&val, &data[1], 8);
                std::cout << "(int) " << val << std::endl;
                return 1 + 8;
            }
        case SER_DBL:
            if (size < 1 + 8) {
                msg("bad response");
                return -1;
            }
            {
                double val = 0;
                memcpy(&val, &data[1], 8);
                std::cout << "(dbl) " << val << std::endl;
                return 1 + 8;
            }
        case SER_ARR:
            if (size < 1 + 4) {
                msg("bad response");
                return -1;
            }
            {
                uint32_t len = 0;
                memcpy(&len, &data[1], 4);
                std::cout << "(arr) len=" << len << std::endl;
                size_t arr_bytes = 1 + 4;
                for (uint32_t i = 0; i < len; ++i) {
                    int32_t rv = on_response(&data[arr_bytes], size - arr_bytes);
                    if (rv < 0) {
                        return rv;
                    }
                    arr_bytes += static_cast<size_t>(rv);
                }
                std::cout << "(arr) end" << std::endl;
                return static_cast<int32_t>(arr_bytes);
            }
        default:
            msg("bad response");
            return -1;
        }
    }




    static int32_t read_res(int fd) {
        std::vector<char> rbuf(4 + k_max_msg + 1);
        errno = 0;
        int32_t err = read_full(fd, rbuf.data(), 4);
        if (err) {
            if (errno == 0) {
                msg("EOF");
            } else {
                msg("read() error");
            }
            return err;
        }

        uint32_t len = 0;
        memcpy(&len, rbuf.data(), 4);  
        if (len > k_max_msg) {
            msg("too long");
            return -1;
        }



        err = read_full(fd, &rbuf[4], len);
        if (err) {
            msg("read() error");
            return err;
        }



        int32_t rv = on_response(reinterpret_cast<uint8_t*>(&rbuf[4]), len);
        if (rv > 0 && static_cast<uint32_t>(rv) != len) {
            msg("bad response");
            rv = -1;
        }
        return rv;
    }

public:


    static int run(int argc, char** argv) {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) {
            die("socket()");
        }



        struct SocketGuard {
            int fd;
            SocketGuard(int socket_fd) : fd(socket_fd) {}
            ~SocketGuard() { close(fd); }
        } socket_guard(fd);

        sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_port = ntohs(1234);
        addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);  // 127.0.0.1
        int rv = connect(fd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
        if (rv) {
            die("connect");
        }

        std::vector<std::string> cmd;
        for (int i = 1; i < argc; ++i) {
            cmd.push_back(argv[i]);
        }
        int32_t err = send_req(fd, cmd);
        if (err) {
            return err;
        }
        err = read_res(fd);
        if (err) {
            return err;
        }

        return 0;
    }
};

int main(int argc, char** argv) {
    return NetworkClient::run(argc, argv);

    
}