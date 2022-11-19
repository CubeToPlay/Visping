
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <stdio.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

#define DISCONNECT_VALUE -1
#define TIME_OUT 1000

int ping(const char* server) {
    // Declare and initialize variables
    HANDLE hIcmpFile;
    unsigned long ipaddr = INADDR_NONE;
    DWORD dwRetVal = 0;
    char SendData[32] = "VISPING BUFFER DATA";
    LPVOID ReplyBuffer = NULL;
    DWORD ReplySize = 0;

    ipaddr = inet_addr(server);
    if (ipaddr == INADDR_NONE) {
        //printf("usage: %s IP address\n", IP_ADRESS);
        return DISCONNECT_VALUE;
    }

    hIcmpFile = IcmpCreateFile();
    if (hIcmpFile == INVALID_HANDLE_VALUE) {
        //printf("\tUnable to open handle.\n");
        //printf("IcmpCreatefile returned error: %ld\n", GetLastError());
        return DISCONNECT_VALUE;
    }

    ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData);
    ReplyBuffer = (VOID*)malloc(ReplySize);
    if (ReplyBuffer == NULL) {
        //printf("\tUnable to allocate memory\n");
        return DISCONNECT_VALUE;
    }


    dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, SendData, sizeof(SendData), NULL, ReplyBuffer, ReplySize, TIME_OUT);
    if (dwRetVal != 0) {
        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
        //struct in_addr ReplyAddr;
        //ReplyAddr.S_un.S_addr = pEchoReply->Address;
        //printf("%ld ms\n", pEchoReply->RoundTripTime);

        return pEchoReply->RoundTripTime;
    }
    else {
        //printf("\tCall to IcmpSendEcho failed.\n");
        //printf("\tIcmpSendEcho returned error: %ld\n", GetLastError());
        return DISCONNECT_VALUE;
    }
}