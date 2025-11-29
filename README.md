Тестовое задание (NDM_test_C++) для компании:
## Сборка проекта
Для сборки проекта в директории с Makefile выполнить:
```shell
make all
```

## Запуск проекта
После сборки, запуск сервера в созданной директории bin:

```shell
cd bin
./server-MNV
./client-MNV
```

### Описание аргументов командной строки для сервера:
```shell
./server-MNV [port]
```

***Если не указывать аргументы, то по дефолту будет использоваться порт 8080***

Пример использования порта 7777:
```shell
./server-MNV 7777
```
***Вывод сервера:***
```shell
Server started at localhost:7777
Waiting for connections...
```
### Описание  аргументов командной строки для клиента:

```shell
./client-MNV [server_ip] [port]
```
- ***ip*** - адрес сервера
- ***port*** - порт сервера

По дефолту без параметров указывается адрес: 127.0.0.1:8080

***Пример с параметрами:*** 
```
./client-MNV 192.168.31.164 7777
```
***Вывод клиента:***
```
connecting to: 192.168.31.164:7777...
Successful connection to server!
Server: Welcome to server. Start typing...
```

### Команды

- ***/stats*** - выводит количество ***всех подключившихся пользователей*** и ***активных на данный момент*** 
  Пример вывода (открыто 4 соединения, 1 закрыто):
  <img width="733" height="45" alt="Pasted image 20251129134554" src="https://github.com/user-attachments/assets/8228f578-94bd-4458-aa57-54d63add4e1e" />
  - **Total clients** - всего подключенных клиентов
  - **Active clients** - активных клиентов сейчас
- ***/time*** - возвращает текущее время <img width="733" height="48" alt="Pasted image 20251129134717" src="https://github.com/user-attachments/assets/8d10d868-5aea-4852-8604-26b69d1de3c1" />

- ***/shutdown*** - завершает работу сервера, закрываются все активные соединения сервером<img width="723" height="134" alt="Pasted image 20251129134817" src="https://github.com/user-attachments/assets/ccbbaaff-ef32-4471-bae1-eaae3678a644" />


Для выхода отправить `exit` или по классике `CTRL + C`


