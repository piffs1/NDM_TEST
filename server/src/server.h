#include <iostream>
#include <vector>

class EpollServer {
private:
    int server_fd;
    int epoll_fd;
    static const int MAX_EVENTS = 64;
    static const int BUFFER_SIZE = 1024;

    /*вектор для отслеживания активных соединений*/
    std::vector<int> active_clients;  // typedef int SOCKET?
    uint32_t total_clients; // всего клиентов
    /* переменная работы сервера */
    bool isRunning = false; 
    //Константы запросов для сервера
    const std::string SCMD_STATS = "/stats";
    const std::string SCMD_SHUTDOWN = "/shutdown";
    const std::string SCMD_TIME = "/time";
public:
    EpollServer(int port);
    ~EpollServer();
    /*старт сервера*/
    void run();
    /*остановка сервера*/
    void stop();
private:
    /*обработка нового подключения*/
    void handle_new_connection();
    /*обрабокта клиентского подключения*/
    void handle_client_data(int client_fd, uint32_t events);
    /*обработка поступаемых сообщений/команд*/
    std::string processCommand(const std::string a_msg);
    /*  */
    void read_from_client(int client_fd);
    void reply_to_client(int client_fd, const char* data, size_t len);
    void close_client(int client_fd);
    std::string get_time();

};