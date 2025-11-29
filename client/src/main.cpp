#include "client.h"

/*
    @todo Сделать флаг -d для включения/отключения вывода дебага в консоль
    @todo Создать общий файл констант constants.h для сервера и клиента. Для команд сервера (/shutdown,/time,/stats)
    и ответов от сервера)
*/

/* функция для интерактивного ввода в консоль */
void interactive_mode(EpollClient_hh& client) {
    
    std::string input;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);
        
        if (input == "exit") {
            break;
        }
        
        if (!input.empty()) {
            client.send_message(input);
        }
    }
}


int main(int argc, char* argv[]) {
    /* создаем дефолтные параметры localhost:8080*/
    std::string server_ip = "127.0.0.1";
    int server_port = 8080;
    
    /* config file. overhead? */
    if (argc >= 2) {
        server_ip = argv[1];
    }
    if (argc >= 3) {
        server_port = std::stoi(argv[2]);
    }
    
    EpollClient_hh client(server_ip, server_port);

    if (!client.connect_to_server()) {
        return EXIT_FAILURE;
    }

    interactive_mode(client);
    std::cout << "Client shutting down..." << std::endl;
    return 0;
}