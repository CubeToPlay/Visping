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
        std::string cmd = "ping -n 1 ";
        
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(strcat(const_cast<char*>(cmd.c_str()), server), "r"), pclose);
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }

        const std::string searchBegin = "Average = ";
        int locationBegin = result.find(searchBegin);

        result = result.erase(0, locationBegin + searchBegin.length());

        const std::string searchEnd = "ms";
        int locationEnd = result.find(searchEnd);

        result = result.erase(locationEnd, locationEnd + searchEnd.length());

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
