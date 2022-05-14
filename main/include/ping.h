#pragma once

#include <cstdio>
#include <stdio.h>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <cstring>
#include <cmath>
#include <array>

#define listLength 10

namespace ping {
    extern int list[listLength];
    inline std::string server(const char* server){
        std::array<char, 128> buffer;
        std::string result;
        std::string cmd = "ping -t ";
        
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(strcat(const_cast<char*>(cmd.c_str()), server), "r"), pclose);
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }

        std::cout << result << std::endl;

        return result;
    }

    inline int average(){
        int average = 0;

        for (int i = 0; i < listLength; i++){
            average += list[i];
        }

        average = round(average/listLength);

        return average;
    }

    inline int highest(){
        int highest = 0;

        for (int i = 0; i < listLength; i++){
            if (list[i] > highest){highest = list[i];}
        }

        return highest;
    }
};
