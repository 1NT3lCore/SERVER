#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"
#define DEFAULT_SERVER "127.0.0.1"
//Определение констант для адреса сервера и порта, к которому будет осуществляться подключение.
void showProgress(int current, int total) {
    float progress = (float)current / total * 100;
    printf("Copying file: %.2f%%\r", progress);
}
//Функция для отображения прогресса копирования файла.
void copyFile(SOCKET ConnectSocket, const char* filename, bool* fileReceived) {
    int fileSize;
    recv(ConnectSocket, reinterpret_cast<char*>(&fileSize), sizeof(fileSize), 0);
    std::ofstream file(filename, std::ios::binary);
    char buffer[1024];
    int bytesReceived;
    int totalBytesReceived = 0;

    while (fileSize > 0) {
        bytesReceived = recv(ConnectSocket, buffer, sizeof(buffer), 0);
        file.write(buffer, bytesReceived);

        fileSize -= bytesReceived;
        totalBytesReceived += bytesReceived;

        showProgress(totalBytesReceived, fileSize + totalBytesReceived);
    }

    printf("\n");

    if (totalBytesReceived > 0) {
        *fileReceived = true;
    }
}
//Функция для копирования файла с сервера на клиент. Она принимает сокет для связи, имя файла, 
    // которое нужно скопировать, и указатель на флаг успешного получения файла.
int main() {
    WSADATA wsaData;
    int iResult;

    SOCKET ConnectSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;
    //Объявление сокета для клиентского соединения и указателя 
    // на структуру addrinfo для хранения информации об адресе сервера.
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
//Установка параметров для создания сокета: любое семейство адресов, 
// тип сокета потока и TCP протокол.
    iResult = getaddrinfo(DEFAULT_SERVER, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }
//Получение информации об адресе сервера.
    ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    //Создание сокета для клиентского соединения.
    iResult = connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
    }
//Подключение к серверу.
    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    printf("Connected to server!\n");
    //Проверка успешного соединения с сервером и вывод соответствующего сообщения.
    char filename[256];
    printf("Enter the name of the file to request: ");
    fgets(filename, sizeof(filename), stdin);

    // Удаление символа новой строки из имени файла
    filename[strcspn(filename, "\n")] = 0;
    //Получение имени файла, который необходимо запросить у сервера.
    send(ConnectSocket, filename, strlen(filename), 0);

    bool fileReceived = false;

    // Создание отдельного потока для копирования файла
    std::thread copyThread(copyFile, ConnectSocket, filename, &fileReceived);

    copyThread.join(); // Ожидание завершения потока

    if (fileReceived) {
        printf("File received successfully!\n");
    }

    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}
//Отправка имени файла, создание потока для копирования файла, 
// ожидание завершения потока и вывод сообщения об успешном получении файла.
//  Закрытие сокета и очистка ресурсов после завершения программы.
