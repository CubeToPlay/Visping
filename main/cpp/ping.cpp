#include "ping.h"

#include <iostream>

void ping::server(const std::string& server){
    const std::string program = "ping -t " + server;
    system(const_cast<char*>(program.c_str()));
    // printf(const_cast<char*>(program.c_str()));
}

void ping::test(int number){
    std::cout << number << std::endl;
}
