#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <thread>
#include <atomic>
#include "portaudio.h"
#include "config.h"

std::atomic<bool> running{ true };

void recive_and_read(SOCKET socket);
void write_and_send(SOCKET sock, sockaddr_in addr);
int check_device();

int main() 
{
    PaError err;

    // Initaialize PortAudio
    err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "Could not initialize PortAudio.\n";
        return -1;
    }

    // Start winsock librery
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        std::cerr << "WSAStartup failded\n.";
        Pa_Terminate();
        return -1;
    }

    // Init socket
    SOCKET my_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (my_sock == INVALID_SOCKET)
    {
        std::cerr << "Socket error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        Pa_Terminate();
        return -1;
    }

    // Init addr
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(LOCALHOST);
    addr.sin_port = htons(PORT);

    if (bind(my_sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "bind failed: " << WSAGetLastError() << "\n";
        return -1;
    }

    //check_device();

    
    std::thread tSend(write_and_send, my_sock, addr);
    std::thread tRecv(recive_and_read, my_sock);

    std::cout << "Press Enter to stop.\n";
    std::cin.get();
    running = false;

    tSend.join();
    tRecv.join();
    
    closesocket(my_sock);
    WSACleanup();
    Pa_Terminate();

    return 0;
    
}