#include "../include/proxy_server.hpp"

ProxyServer::ProxyServer()
{
    serv_socket = initializeSocket();

    // Insert server descriptor to poll 
    // for listen and connect new clients
    poll_fds[0].fd = serv_socket;
    poll_fds[0].events = POLLIN;

    for (size_t i = 1; i < POLLMAX; ++i) {
        poll_fds[i].fd = -1;
    }
}

ProxyServer::~ProxyServer()
{
    closeSocket(serv_socket);
    std::cout << "DELETE SERVER" << std::endl;
}

int ProxyServer::initializeSocket()
{
    int new_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (new_socket == INVALID_SOCKET) {
        throw std::system_error(errno, std::generic_category());
    }

    return new_socket;
}

void ProxyServer::setAddress(struct sockaddr_in& addr, int port, std::string ip)
{
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.data());
};

void ProxyServer::bindServerAddress(int port, std::string ip)
{
    setAddress(serv_addr, port, ip);

    addrlen = sizeof(serv_addr);

    if (bind(serv_socket, (sockaddr*) &serv_addr, addrlen) == -1) {
        throw std::system_error(errno, std::generic_category());
    }

    listenSocket();
}

void ProxyServer::bindPostgresAddress(int port, std::string ip)
{
    setAddress(postgres_addr, port, ip);
}

void ProxyServer::listenSocket()
{
    if (listen(serv_socket, SOMAXCONN) == -1) {
        throw std::system_error(errno, std::system_category());
    }
}

int ProxyServer::Poll()
{
    static size_t client_index = 0, max_poll_index = 0;

    int nready = poll(poll_fds, max_poll_index + 1, TIMEOUT);
    
    if (nready == -1) {
        throw std::system_error(errno, std::system_category());
    }

    // Accept new client
    if (poll_fds[0].revents & POLLIN) {  
        client_index = handleConnection();

        if (client_index + 1 > max_poll_index)
            max_poll_index = client_index + 1;

        if (--nready <= 0)
            return 1;
    }

    // Checking all client sockets
    for (size_t i = 1; i <= max_poll_index; i += 2) {

        if (poll_fds[i].fd < 0)
            continue;
        
        if (poll_fds[i].revents & POLLIN) {
            if (handleRequestData(poll_fds[i].fd, poll_fds[i + 1].fd) == -1) {
                handleDisconnection(poll_fds[i].fd, poll_fds[i + 1].fd);
                continue;
            }
        }
    }

    // Checking all database sockets
    for (size_t i = 2; i <= max_poll_index; i += 2) {

        if (poll_fds[i].fd < 0)
            continue;

        if (poll_fds[i].revents & POLLIN) {
            if (handleResponseData(poll_fds[i].fd, poll_fds[i - 1].fd) == -1) {
                handleDisconnection(poll_fds[i].fd, poll_fds[i - 1].fd);
                continue;
            }
        }
    }

    return 1;
}

// Create pair client and database socket
int ProxyServer::handleConnection()
{
    // Create new client socket
    sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    int client_socket = accept(serv_socket, (sockaddr*)&client_addr, &addrlen);
    
    if (client_socket  == INVALID_SOCKET) {
        throw std::runtime_error("accept error");
    }

    // Create new database socket
    int db_socket = initializeSocket();
    addrlen = sizeof(postgres_addr);

    if (connect(db_socket, (sockaddr *) &postgres_addr, addrlen) == -1) {
        throw std::system_error(errno, std::generic_category());
    }

    std::string client_ip = inet_ntoa(client_addr.sin_addr);
    std::string db_ip = inet_ntoa(postgres_addr.sin_addr);

    clients[client_socket].ip = client_ip;
    clients[client_socket].port = ntohs(client_addr.sin_port);

    databases[db_socket].ip = db_ip;
    databases[db_socket].port = ntohs(postgres_addr.sin_port);
    
    std::cout << "NEW CONNECT: port " << clients[client_socket].port 
              << " ip: " << client_ip << std::endl;
    
    return setSocketPoll(client_socket, db_socket);
}

// Set file descriptor for pair client and database
// return: client index
int ProxyServer::setSocketPoll(int client_socket, int db_socket)
{

    size_t i;
    for (i = 1; i < POLLMAX - 1; i += 2)
    {
        if (poll_fds[i].fd < 0 && poll_fds[i + 1].fd < 0) {
            poll_fds[i].fd = client_socket;
            poll_fds[i + 1].fd = db_socket;
            break;
        }
    }

    if (i >= POLLMAX - 1) {
        throw std::runtime_error("poll error (too many clients)");
    }

    poll_fds[i].events = POLLIN;
    poll_fds[i + 1].events = POLLIN;
    return i;
}

void ProxyServer::handleDisconnection(int& client_socket, int& db_socket)
{
    closeSocket(client_socket);
    closeSocket(db_socket);
    client_socket = INVALID_SOCKET;
    db_socket = INVALID_SOCKET;
}

int ProxyServer::handleRequestData(int recv_fd, int send_fd)
{
    std::vector<uint8_t> buf(BUFFER_SIZE);
    
    int n = 0;
    if ((n = recv(recv_fd, buf.data(), buf.size(), 0)) <= 0) {
        clients.erase(recv_fd);
        databases.erase(send_fd);
        return -1;
    }
    else {
        logger.log(INFO, clients[recv_fd].ip, clients[recv_fd].port, 
                   parser.parse(buf));
    
        send(send_fd, buf.data(), n, 0);
    }

    return 0;
}

int ProxyServer::handleResponseData(int recv_fd, int send_fd)
{
    std::vector<uint8_t> buf(BUFFER_SIZE);
    
    int n = 0;
    if ((n = recv(recv_fd, buf.data(), buf.size(), 0)) <= 0) {
        clients.erase(recv_fd);
        databases.erase(send_fd);
        return -1;
    }
    else {
        logger.log(INFO, databases[recv_fd].ip, databases[recv_fd].port, 
                   parser.parse(buf));
        send(send_fd, buf.data(), n, 0);
    }

    return 0;
}
// void ProxyServer::Send(int sockfd, std::vector<uint8_t>& buf)
// {
//     if (!isSocketOpen(sockfd)) {
//         throw std::runtime_error("send error (socket is closed)");
//     }

//     // Error list in manual
//     // https://man7.org/linux/man-pages/man2/send.2.html
//     // Connection reset by peer
//     ssize_t bytes_write = send(sockfd, buf.data(), buf.size(), 0);

//     if (bytes_write < 0) {
//         throw std::system_error(errno, std::generic_category());
//     }
// }

// void ProxyServer::Recv(int sockfd, std::vector<uint8_t>& buf)
// {
//     if (!isSocketOpen(sockfd)) {
//         throw std::runtime_error("recv error (socket is closed)");
//     }
    
//     // Error list in manual
//     // https://man7.org/linux/man-pages/man2/recvfrom.2.html
//     // Connection reset by peer
//     ssize_t bytes_read = recv(sockfd, buf.data(), sizeof(buf), 0);

//     if (bytes_read == 0) {
//         close(sockfd);
//     }
//     else if (bytes_read == -1) {
//         throw std::system_error(errno, std::generic_category());
//     }

//     return bytes_read;
// }

bool ProxyServer::isSocketOpen(int sockfd) const 
{
    if (sockfd == INVALID_SOCKET) {
        return false;
    }

    return true;
}

void ProxyServer::closeSocket(int sockfd) 
{
    if (isSocketOpen(sockfd)) {
        close(sockfd);
    }
}