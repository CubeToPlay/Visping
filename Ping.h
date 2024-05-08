#pragma once

#include "framework.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <stdio.h>

#define DISCONNECT_VALUE -1
#define PING_TIME_OUT 1000
#define BUFFER_LENGTH 32
#define PING_LOOP_INTERVAL 50

class Ping {
public:
    Ping(wchar_t* server, int pingLength);

    ~Ping();

    void setAddress(wchar_t* server);

    void start();
    void stop();

    double getAverage();
    int getPing();
    int getPing(int index);
    int getMax();
    int getMin();
    double getInstability();

private:
    static DWORD WINAPI ThreadLoop(LPVOID lpParam);
    
    int ping();

    void insert(int pingValue);

    HANDLE hThread;

    char* sendData[BUFFER_LENGTH];
    DWORD dwRetVal;
    DWORD replySize;
    HANDLE hIcmpFile;

    ULONG ipAddress;
    addrinfoW hints, *addressInfo;

    bool run;
    double average, instability;
    int minimum, maximum;
    
    std::vector<int>* pingVector;
};