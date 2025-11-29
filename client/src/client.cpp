#include "client.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

EpollClient_hh::EpollClient_hh(const std::string& ip, int port) 
    : server_ip(ip), server_port(port){}

EpollClient_hh::~EpollClient_hh() {
    disconnect();
}

bool EpollClient_hh::connect_to_server() {
    /*создаем соедиение на базе ТСР*/
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        std::cout << "Error of creation client.socket: " << strerror(errno) << std::endl;
        return false;
    }
    
    
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr) <= 0) {
        std::cout << "Error of address server: " << server_ip << std::endl;
        close(sock_fd);
        return false;
    }

    std::cout << "connecting to: " << server_ip << ":" << server_port << "..." << std::endl;

    if (connect(sock_fd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "Error connection: " << strerror(errno) << std::endl;
        close(sock_fd);
        return false;
    }

    std::cout << "Successful connection to server!" << std::endl;
    /* приветственный блок от сервера.
    "Server: Welcome to server. Start typing..."
    Нужен ли? В задании не указан. 
    */
    char welcome_buffer[1024];
    ssize_t welcome_bytes = recv(sock_fd, welcome_buffer, sizeof(welcome_buffer),0);
    
    if (welcome_bytes > 0) {
        welcome_buffer[welcome_bytes] = '\0';
        std::cout << "Server: " << welcome_buffer;
    } 
    else if (welcome_bytes == 0) {
        std::cout << "[Server closed connection...]" << std::endl;
        return false;
    } else {
        std::cout << "Error reading..." << strerror(errno) << std::endl;
        return false;
    }

    return true;
}

void EpollClient_hh::disconnect() {
    if (sock_fd != -1) {
        std::cout << "Disconnect from server..." << std::endl;
        close(sock_fd);
        sock_fd = -1;
    }
}

void EpollClient_hh::send_message(const std::string& message) {
    if (sock_fd == -1) {
        std::cout << "No connection to server..." << std::endl;
        return;
    }

    ssize_t bytes_sent = send(sock_fd, message.c_str(), message.length(), 0);
    if (bytes_sent <= 0) {
        std::cout << "Error sending: " << strerror(errno) << std::endl;
        return;
    }
    /*ожидаем ответ от сервера после отправки*/
    read_server_response();
}

void EpollClient_hh::read_server_response() {
    
    char buffer[1024];
    ssize_t bytes_read = read(sock_fd, buffer, sizeof(buffer) - 1);    
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        std::cout << "[Server]: " << buffer << std::endl;
        ///Костыль. Пока не стал заморачиваться делать по другому. Если так работает, то
        ///todo: Вынести в отдельный общий constants.h для сервера и клиента и забить константы в хидер
        if (std::string(buffer)=="Shutdown server...") {
            std::cout << "[Server closed connection...]" << std::endl;
            disconnect();
        }
    }
    else if (bytes_read == 0) {
        std::cout << "[Server closed connection...]" << std::endl;
        disconnect();
    } else {
        std::cout << "Error reading..." << strerror(errno) << std::endl;
    }
}

