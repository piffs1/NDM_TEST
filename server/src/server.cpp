#include "server.h"
#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <ctime>

EpollServer::EpollServer(int port) : server_fd(-1), epoll_fd(-1), total_clients(0) {

        server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (server_fd == -1) {
            throw std::runtime_error("Error socket creation: " + std::string(strerror(errno)));
        }

        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
            close(server_fd);
            throw std::runtime_error("Error setsockopt: " + std::string(strerror(errno)));
        }

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        //localhost. Может inet_pton? в задании не сказано где биндить сервер, оставим локалхост, да и линуксы отдельной нет проверить. Виртуалка?
        server_addr.sin_addr.s_addr = INADDR_ANY; 
        server_addr.sin_port = htons(port);
        
        if (bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
            close(server_fd);
            throw std::runtime_error("Error binding server:" + std::string(strerror(errno)));
        }

        if (listen(server_fd, SOMAXCONN) == -1) {
            close(server_fd);
            throw std::runtime_error("Error listening server:" + std::string(strerror(errno)));
        }

        epoll_fd = epoll_create1(0);
        if (epoll_fd == -1) {
            close(server_fd);
            throw std::runtime_error("Error epoll_create1: " + std::string(strerror(errno)));
        }

        epoll_event event{};
        event.events = EPOLLIN | EPOLLET;  // edge-triggered
        event.data.fd = server_fd;
        
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1) {
            close(server_fd);
            close(epoll_fd);
            throw std::runtime_error("Error epoll_ctl server: " + std::string(strerror(errno)));
        }

        std::cout << "Server started at localhost:" << port << std::endl;
        std::cout << "Waiting for connections..." << std::endl;
}

EpollServer::~EpollServer() {
        stop();
    }

void EpollServer::run() {
    std::vector<epoll_event> events(MAX_EVENTS);
    isRunning = true;
    while (isRunning) {
        // 7. Ожидание событий (блокирующий вызов)
        int num_events = epoll_wait(epoll_fd, events.data(), MAX_EVENTS, -1);
        if (num_events == -1) {
            if (errno == EINTR) {
                    // Вызов был прерван сигналом - продолжаем
                    continue;
                }
            /*иная ошибка. кидаем exception*/
            throw std::runtime_error("Error epoll_wait: " + std::string(strerror(errno)));
        }

        std::cout << "\nReceived events: " << num_events << std::endl;

        for (int i = 0; i < num_events; ++i) {
            uint32_t event_flags = events[i].events;
            int event_fd = events[i].data.fd;

            if (event_flags & (EPOLLERR | EPOLLHUP)) {
                std::cerr << "Error in socket: " << event_fd << std::endl;
                if (event_fd != server_fd) {
                    close_client(event_fd);
                }
                continue;
            }

            if (event_fd == server_fd) {
                /*обрабатываем новое подключение на сервере*/
                handle_new_connection();
            } else {
            // Данные на клиентском сокете
            handle_client_data(event_fd, event_flags);
            }
        }
    }
}

void EpollServer::stop()
{
    // Закрытие всех клиентских сокетов
    for (int client_fd : active_clients) {
        close(client_fd);
    }
        
    if (server_fd != -1) {
        close(server_fd);
        std::cout << "Server's socket closed" << std::endl;
    }
    if (epoll_fd != -1) {
        close(epoll_fd);
        std::cout << "Epoll instanse closed" << std::endl;
    }
    isRunning = false;
}

void EpollServer::handle_new_connection() {
    std::cout << "\n=== NEW CONNECTION ===" << std::endl;

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
            
        int client_fd = accept4(server_fd, (sockaddr*)&client_addr, &client_len, SOCK_NONBLOCK);
            
        if (client_fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            } else {
                std::cout << "Error accept: " << strerror(errno) << std::endl;
                break;
            }
        }

        /* добавляем клиента в epoll. Берем дополнительный флаг EPOLLRDHUP для обнаружения закрытия у клиента(для отслеживания активных сессий /stats)*/
        epoll_event client_event{};
        client_event.events = EPOLLIN | EPOLLET | EPOLLRDHUP; 
        client_event.data.fd = client_fd;
            
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) == -1) {
            std::cout << "Error epoll_ctl for client: " << strerror(errno) << std::endl;
            close(client_fd);
            continue;
        }
        /* добавляем в отслеживание клиента */
        active_clients.push_back(client_fd);
        total_clients++;
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            
        std::cout << "Client connected: " << client_ip << ":" << ntohs(client_addr.sin_port) 
            << " (fd: " << client_fd << ")" << std::endl;
        "Total clients: " + std::to_string(total_clients) + "\tActive clients: " + std::to_string(active_clients.size());

        /*приветственное сообщение клиенту от сервера, в задании не указано, но пусть будет*/
        const char* welcome_msg = "Welcome to server. Start typing...\n";
        send(client_fd, welcome_msg, strlen(welcome_msg), MSG_DONTWAIT);
    }
}

void EpollServer::handle_client_data(int client_fd, uint32_t events) {
    /*закрыл ли сокет соединение*/
    if (events & EPOLLRDHUP) {
        std::cout << "Client: " << client_fd << " closed connection (EPOLLRDHUP)" << std::endl;
        close_client(client_fd);
        return;
    }

    /*данные от клиента*/
    if (events & EPOLLIN) {
        read_from_client(client_fd);
    }
}
/* функция получения текущего времени */
std::string EpollServer::get_time()
{
    std::time_t t = std::time(nullptr);
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&t));

    return std::string(buffer);
}
/*обработка команды сервером. Нужно ли обрабатывать unknown command? В задании не указано :)*/
std::string EpollServer::processCommand(const std::string a_msg) 
{
    if (a_msg == SCMD_TIME) {
        return get_time();
    }
        
    if (a_msg == SCMD_STATS) {
        return "Total clients: " + std::to_string(total_clients) + "\tActive clients: " + std::to_string(active_clients.size());
    }

    if (a_msg == SCMD_SHUTDOWN) {
        return "Shutdown server...";
    }

    return a_msg;
}
/* чтение от клиента*/
void EpollServer::read_from_client(int client_fd) {
    std::cout << "\n--- Reading from client " << client_fd << " ---" << std::endl;
        
    char buffer[BUFFER_SIZE];
    ssize_t total_bytes = 0;
        
    while (true) {
        ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
            
        if (bytes_read > 0) {
            total_bytes += bytes_read;
            buffer[bytes_read] = '\0';
                
            std::cout << "Data from client " << client_fd << " (" 
                << bytes_read << " byte): " << buffer << std::endl;
                
            std::string msg = std::string(buffer);
            std::string res = processCommand(msg);
            reply_to_client(client_fd, res.c_str(), res.size());
            if (msg == SCMD_SHUTDOWN){
                stop();
                break;
            }
        } 
        else if (bytes_read == 0) {
            std::cout << "Client " << client_fd << " closed connection" << std::endl;
            close_client(client_fd);
            break;
        } else {
            /*полная вычитка*/
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cout << "[all]Read: " << total_bytes << " bytes from client " 
                    << client_fd << std::endl;
                break;
            } else {
                std::cout << "Error reading from client " << client_fd << ": " << strerror(errno) << std::endl;
                close_client(client_fd);
            break;
            }
        }
    }
}

void EpollServer::reply_to_client(int client_fd, const char* data, size_t len) {
    std::cout << "Sending client " << client_fd << "..." << std::endl;
        
    ssize_t bytes_sent = send(client_fd, data, len, MSG_DONTWAIT);
        
    if (bytes_sent == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            std::cout << "The buffer is full for client " << client_fd << std::endl;
        } else {
            std::cout << "Error sending to client " << client_fd << ": " << strerror(errno) << std::endl;
            close_client(client_fd);
        }
    } else {
        std::cout << "Transmitted " << bytes_sent << " bytes for client " << client_fd << std::endl;
    }
}

void EpollServer::close_client(int client_fd) {
    std::cout << "closing client...: " << client_fd << std::endl;
        
    /*удаляем из отслеживания epoll*/
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);

    close(client_fd);
        
    /*удаляем из активных клиентов*/
    auto it = std::find(active_clients.begin(), active_clients.end(), client_fd);
    if (it != active_clients.end()) {
        active_clients.erase(it);
    }
    std::cout << "Remain clients: " << active_clients.size() << std::endl;
}


