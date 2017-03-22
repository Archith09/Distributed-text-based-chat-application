

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdexcept>

class Server
    {
    public:
        Server(const char* ipAddress, int port);
       ~Server();
        int get_socket() const;
        int get_port() const;
        std::string get_addr() const;
		int send(struct sockaddr_in clientAddress,char *message, size_t max_size);
        int recv(struct sockaddr_in clientAddress,char *message, size_t max_size);

    private:
        int udp_socket;
        int udp_port;
        std::string udp_addr;
        struct sockaddr_in udp_addrinfo;
    };

