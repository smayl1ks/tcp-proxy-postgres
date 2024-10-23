#include "../include/proxy_server.hpp"
#include <csignal>
#include <iostream>

int main()
{
    Logger errorlog("errors.txt");
    //Logger log("log.txt");

    int server_port = 8080; 
    int db_port = 5432; 

    std::string server_ip = "127.0.0.1";
    std::string db_ip = "127.0.0.1";

    ProxyServer server;

    try
    {
        server.bindServerAddress(server_port, server_ip);
        server.bindPostgresAddress(db_port, db_ip);
        while (server.Poll());

    } catch (const std::system_error& ex)
    {
        std::cout << "SYSTEM ERROR: " << ex.what() << std::endl;
        errorlog.log(ERROR, server_ip, server_port, ex.what());
    } 
    catch (const std::runtime_error& ex)
    {
        std::cout << "RUNTIME ERROR: " << ex.what() << std::endl;
        errorlog.log(ERROR, server_ip, server_port, ex.what());
    }
    catch (const std::exception& ex)
    {
        std::cout << "OTHER ERROR: " << ex.what() << std::endl;
        errorlog.log(ERROR, server_ip, server_port, ex.what());
    }

    return 0;
}