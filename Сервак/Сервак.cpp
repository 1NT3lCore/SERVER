#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <ws2tcpip.h>
#include <string>  //Включение необходимых заголовочных файлов для работы с Winsock, 
//Windows API, стандартным вводом/выводом, файлами, сокетами и строками.

#pragma comment(lib, "Ws2_32.lib")
//Директива #pragma для автоматической ссылки на библиотеку Ws2_32.lib, 
// необходимую для работы с Winsock.

#define DEFAULT_PORT 27015
//Определение константы DEFAULT_PORT , которая хранит порт, на котором будет работать сервер.
int main() {
    WSADATA wsaData;
    int iResult;
    //Объявление переменных для инициализации Winsock и хранения результата операций.
    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;
    //Объявление сокетов ListenSocket для прослушивания подключений 
    // и ClientSocket для коммуникации с клиентом.
    struct addrinfo* result = NULL;
    struct addrinfo hints;
    //Объявление указателя на структуру addrinfo для обработки информации об адресе сервера 
    // и самой структуры hints для настройки параметров создания сокета.
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cout << "WSAStartup failed with error: " << iResult << std::endl;
        return 1;
    }
    //Инициализация Winsock с указанием версии 2.2.
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    //Установка параметров для создания сокета: семейство адресов, тип сокета, 
    // протокол TCP и флаг AI_PASSIVE для серверного сокета.
    iResult = getaddrinfo(nullptr, std::to_string(DEFAULT_PORT).c_str(), &hints, &result);
    if (iResult != 0) {
        std::cout << "getaddrinfo failed with error: " << iResult << std::endl;
        WSACleanup();
        return 1;
    }
    //Получение информации об адресе сервера на указанном порту.
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        std::cout << "socket failed with error: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    //Создание сокета для прослушивания подключений на заданном адресе и порту.
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        std::cout << "bind failed with error: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    //Привязка сокета к адресу и порту.
    freeaddrinfo(result);
    //Очистка памяти после использования информации об адресе.
    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        std::cout << "listen failed with error: " << WSAGetLastError() << std::endl;
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    //Начало прослушивания входящих соединений.
    std::cout << "Server started, waiting for clients..." << std::endl;
    //Вывод сообщения о запуске сервера и ожидании клиентов.
    while (true) {
        //Начало бесконечного цикла ожидания клиентских подключений.
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            std::cout << "accept failed with error: " << WSAGetLastError() << std::endl;
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }
        //Принятие входящего подключения от клиента.
        std::cout << "Client connected!" << std::endl;
        //Вывод сообщения о подключении клиента.
        char recvbuf[1024];
        int recvbuflen = sizeof(recvbuf);
        //Объявление буфера для приема данных от клиента.
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            std::string filename(recvbuf, iResult);

            std::ifstream file(filename, std::ios::binary);
            if (file) {
                // Отправка файла

                // Установка флага успешной отправки файла
                bool fileSentSuccessfully = true;

                if (fileSentSuccessfully) {
                    std::cout << "File received successfully!" << std::endl;
                }
            }
            else {
                std::cout << "Failed to open file: " << filename << std::endl;
            }
        }

        closesocket(ClientSocket);
    }
    //Закрытие клиентского сокета после завершения операций.
    closesocket(ListenSocket);
    WSACleanup();
    //Закрытие серверного сокета и очистка ресурсов после завершения программы.
    return 0;
}
