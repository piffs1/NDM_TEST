#include <iostream>

class EpollClient_hh {
private:
    int sock_fd=-1; // если определять в EpollClient_hh() : sock_fd(-1), то ворнинги появляются :( -Wreorder
    std::string server_ip;
    int server_port;
public:
    EpollClient_hh(const std::string& ip, int port);
    ~EpollClient_hh();
    /* Подключение к серверу */
    bool connect_to_server();
    /*отключение от севера*/
    void disconnect();
    /* функция отправки сообщения на сервер */
    void send_message(const std::string& message);
private:
    /* функция получения ответа от сервера*/
    void read_server_response();
};
