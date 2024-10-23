#pragma once

#include "logger.hpp"
#include "parser.hpp"

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <system_error>

#include <vector>
#include <string>
#include <unordered_map>

enum settings {
    POLLMAX = 1000, // maximum clients in poll
    TIMEOUT = -1,  // waiting event from clients
    BUFFER_SIZE = 2048 // maximum buffer for recv
};

struct Socket
{
    int port;
    std::string ip;
    // ...
};

class ProxyServer
{
private:
    constexpr static int INVALID_SOCKET = -1;
    int serv_socket = INVALID_SOCKET;

    Parser parser;
    Logger logger;

    struct sockaddr_in serv_addr, postgres_addr;
    socklen_t addrlen;

    struct pollfd poll_fds[POLLMAX];
    
    // File descriptors and client info 
    std::unordered_map <int, Socket> clients;

    // File descriptors and database info 
    std::unordered_map <int, Socket> databases;

    int initializeSocket();
    void listenSocket();

    void setAddress(struct sockaddr_in& addr, int port, std::string ip);
    int setSocketPoll(int client_socket, int db_socket);
    
    int handleConnection();
    int handleRequestData(int recv_fd, int send_fd);
    int handleResponseData(int recv_fd, int send_fd);

    void handleDisconnection(int& client_socket, int& db_socket);

public:

    ProxyServer();
    ~ProxyServer();

    int Poll();
    
    void bindPostgresAddress(int port, std::string ip);
    void bindServerAddress(int port, std::string ip);
    
    //void Send(int sockfd, std::vector<uint8_t>& buf);
    //std::string Recv(int sockfd, std::vector<uint8_t>& buf);

    bool isSocketOpen(int sockfd) const;
    void closeSocket(int sockfd);
};