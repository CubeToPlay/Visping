#include "Ping.h"

Ping::Ping(wchar_t* server, int length) {
    run = false;
    hIcmpFile = NULL;
    dwRetVal = 0;
    replySize = 0;

    //memset(&hints, 0, sizeof(hints)); // Initialize hints to 0
    hints.ai_family = AF_UNSPEC;     // Any address family (IPv4 or IPv6)
    hints.ai_socktype = SOCK_STREAM; // TCP sockets

    pingVector.reset(new std::vector<int>(length));

    setAddress(server);
}

Ping::~Ping() {
    stop();
    WSACleanup();
}

void Ping::start() {
    if (!run.load()) {
        run.store(true);
        hIcmpFile = IcmpCreateFile();
        hThread = CreateThread(NULL, 0, ThreadLoop, this, 0, NULL);
    }
}

void Ping::stop() {
    if (run.load()){
        run.store(false);
        IcmpCloseHandle(hIcmpFile);
        CloseHandle(hThread);
    }
}

DWORD WINAPI Ping::ThreadLoop(LPVOID lpParam) {
    Ping* ping = static_cast<Ping*>(lpParam);
    while (ping->run.load()) {
        ping->insert(ping->ping());
        
        int sum = 0;

        ping->maximum = *std::max_element(ping->pingVector->begin(), ping->pingVector->end());
        ping->minimum = *std::min_element(ping->pingVector->begin(), ping->pingVector->end());
        
        for (int const& p : *ping->pingVector) sum += p;
        ping->average = (double)sum / (ping->pingVector->size());

        sum = 0;
        for (int const& p : *ping->pingVector) sum += (int)pow(p - ping->average, 2);
        ping->instability = sqrt((double)sum / (ping->pingVector->size() - 1));

        int delay = PING_LOOP_INTERVAL - ping->getPing();

        if (delay > 0) std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }

    return 0;
}

void Ping::setAddress(wchar_t* server) {
    this->server = server;

    ipAddress = INADDR_NONE;

    if (InetPton(AF_INET, server, &ipAddress) != 1)
    {
        int status = GetAddrInfoW(server, L"80", &hints, &addressInfo);

        if (status != 0)
        {
            OutputDebugStringA("Error resolving address: ");
            OutputDebugString(gai_strerror(status));
            OutputDebugStringA("\n");
            return;
        }

        // Loop through the linked list of addresses
        for (addrinfoW* info = addressInfo; info != nullptr; info = info->ai_next) {
            wchar_t addressString[INET6_ADDRSTRLEN];
            void* address;

            // Get address string based on family
            if (info->ai_family == AF_INET) 
                address = &((sockaddr_in*)info->ai_addr)->sin_addr;
            else 
                address = &((sockaddr_in6*)info->ai_addr)->sin6_addr;
            
            InetNtop(info->ai_family, address, addressString, sizeof(addressString));
            InetPton(info->ai_family, addressString, &ipAddress);

            OutputDebugStringA("Addresses: ");
            OutputDebugStringW(addressString);
            OutputDebugStringA("\n");
        }

        FreeAddrInfoW(addressInfo);
    }
    else {
        OutputDebugStringA("Addresses: ");
        OutputDebugStringW(server);
        OutputDebugStringA("\n");
    }
}

int Ping::ping() {

    if (hIcmpFile == INVALID_HANDLE_VALUE) return DISCONNECT_VALUE;

    if (ipAddress == INADDR_NONE) {
        setAddress(this->server);
        return  DISCONNECT_VALUE;
    }

    replySize = sizeof(ICMP_ECHO_REPLY) + sizeof(sendData) + 8;
    void* replyBuffer = (VOID*)malloc(replySize);

    if (replyBuffer == NULL) return DISCONNECT_VALUE;


    int tripTime = DISCONNECT_VALUE;
    dwRetVal = IcmpSendEcho(hIcmpFile, ipAddress, sendData, sizeof(sendData), NULL, replyBuffer, replySize, PING_TIME_OUT);
    
    if (dwRetVal != 0) tripTime = ((PICMP_ECHO_REPLY)replyBuffer)->RoundTripTime;

    free(replyBuffer);

    return tripTime;
}

void Ping::insert(int pingValue) {
    pingVector->insert(pingVector->begin(), pingValue);
    pingVector->pop_back();
}

int Ping::getPing() { return pingVector->front(); }
int Ping::getPing(int index) { return (index < 0 || index >= pingVector->size()) ? DISCONNECT_VALUE : pingVector->at(index); }
int Ping::getMax() { return maximum; }
int Ping::getMin() { return minimum; }
double Ping::getInstability() { return instability; }
double Ping::getAverage() { return average; }
bool Ping::isRunning() { return run; }