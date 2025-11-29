#include "server.h"

/*
@todo Сделать бинд сервера на разные адреса и добавить этот параметр в парсинг командной строки пример: ./server-maximovhh [ip] [port] 
./server-maximovhh 127.0.0.1 8080 
@todo сделать флаг -d для включения/отключения отладки в консоль. -d отладка включена
@todo Вынести константы в отдельный общий файл для клиента/сервера constants.h для переменных SCMD_* в server.h
*/


int main(int argc, char* argv[]) {
    //using default 8080 port
    int port = 8080;
    
    if (argc == 2) {
        port = std::stoi(argv[1]);
    }
    
    
    try {
        EpollServer server(port);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return 0;
}
