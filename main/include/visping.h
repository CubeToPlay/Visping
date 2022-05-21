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
#include <ctime>

#define goodPing 60
#define slowPing 90

#define arrayLength 20
#define disconnected -1

namespace vpg {
    static std::array<int, arrayLength> list;

    inline int get(int value){
        return vpg::list.at(value);
    }

    inline void insert(int value){
        if (vpg::list.size() < vpg::list.max_size()){
            vpg::list[vpg::list.size()] = value;
            // std::cout << vpg::list.size() << " " << vpg::list.max_size() << std::endl;
        } else {
            for(int i = arrayLength; i >= 1; i--){
                vpg::list[i] = vpg::list[i-1];
            }
            vpg::list[0] = value;
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

        for(const int &ping : vpg::list){
            average += ping;
        }

        average = round(average/arrayLength);

        return average;
    }

    inline int highest(){
        int highest = 0;

        for(const int &ping : vpg::list){
            if (ping > highest){highest = ping;}
        }

        return highest;
    }

    inline int lowest(){
        int lowest = vpg::highest();

        for(const int &ping : vpg::list){
            if (ping < lowest){lowest = ping;}
        }

        return lowest;
    }

    inline int stablitiy(){
        return vpg::average() - vpg::lowest();
    }

    inline int once(const char* server){
        std::string result = vpg::server(server);

        // std::cout << result << std::endl;

        if (result.find("Average = ") == std::string::npos){
            return disconnected;
        }

        const std::string searchBegin = "Average = ";
        int locationBegin = result.find(searchBegin);

        result = result.erase(0, locationBegin + searchBegin.length());

        const std::string searchEnd = "ms";
        int locationEnd = result.find(searchEnd);

        result = result.erase(locationEnd, locationEnd + searchEnd.length());

        return stoi(result);
    }

    inline void display(){
        if (vpg::list[0] == disconnected){
            std::cout << "Lost Connection!" << std::endl;
        } else {
            // std::cout << vpg::list[0] << std::endl;
            std::cout << vpg::average() << std::endl;
            // std::cout << vpg::stablitiy() << std::endl;
        }
    }

    inline std::string string(){
        std::string fullString;
        std::string insertString;

        // for(const int &ping : vpg::list){
        //     insertString = std::to_string(ping) + " ";
        //     fullString.append(insertString);
        //     insertString = "";
        // }

        fullString.append(" Average: " + std::to_string(vpg::average()));
        fullString.append("\n Highest: " + std::to_string(vpg::highest()));
        fullString.append("\n Stability: " + std::to_string(vpg::stablitiy()));

        return fullString;
    }
};
