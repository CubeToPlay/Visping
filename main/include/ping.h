#pragma once

#include <cstdio>
#include <stdio.h>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <array>
#include <vector>

#define arrayLength 10

namespace ping {
    static std::array<int, arrayLength> list;

    inline int get(int value){
        return ping::list.at(value);
    }

    inline void insert(int value){
        if (ping::list.size() < ping::list.max_size()){
            ping::list[ping::list.size()] = value;
            // std::cout << ping::list.size() << " " << ping::list.max_size() << std::endl;
        } else {
            for(int i = arrayLength; i >= 1; i--){
                ping::list[i] = ping::list[i-1];
            }
            ping::list[0] = value;
        }
    }

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

        return result;
    }

    inline int average(){
        int average = 0;

        for(const int &ping : ping::list){
            average += ping;
        }

        average = round(average/arrayLength);

        return average;
    }

    inline int highest(){
        int highest = 0;

        for(const int &ping : ping::list){
            if (ping > highest){highest = ping;}
        }

        return highest;
    }

    inline int once(const char* server){
        std::string result = ping::server(server);

        const std::string searchBegin = "Average = ";
        int locationBegin = result.find(searchBegin);

        result = result.erase(0, locationBegin + searchBegin.length());

        const std::string searchEnd = "ms";
        int locationEnd = result.find(searchEnd);

        result = result.erase(locationEnd, locationEnd + searchEnd.length());

        return stoi(result);
    }

    inline void display(){
        std::cout << "Ping List: " << std::endl;
        for(const int &ping : ping::list){
            std::cout << ping << " ";
        }
        std::cout << std::endl;

        std::cout << "Ping Average: " << std::endl;
        std::cout << ping::average() << std::endl;
         
        std::cout << "Ping Highest: " << std::endl;
        std::cout << ping::highest() << std::endl;
    }
};
