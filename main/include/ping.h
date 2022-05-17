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

        // std::cout << result << std::endl;

        if (result.find("Average = ") == std::string::npos){
            return 0;
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
        if (ping::list[0] == 0){
            std::cout << "Lost Connection!" << std::endl;
        } else {
            // std::cout << "Ping List: " << std::endl;
            // for(const int &ping : ping::list){
            //     std::cout << ping << " ";
            // }
            // std::cout << std::endl;
            
            // std::cout << "Ping Average: " << std::endl;
            // std::cout << ping::average() << std::endl;


            std::cout << ping::list[0] << std::endl;
            

            // std::cout << "Ping Highest: " << std::endl;
            // std::cout << ping::highest() << std::endl;
        }
    }

    inline void draw(HDC& hdc, int width, int height){
        std::cout << "Pass" << std::endl;

        int center = std::round(width/2);

        // RECT rect;
        // rect.top     = 0;
        // rect.right   = 0;
        // rect.bottom  = height;
        // rect.left    = width;

        // FillRect(hdc, &rect, (HBRUSH) (COLOR_WINDOW));

        TRIVERTEX vertex[2];
        vertex[0].y     = 0;
        vertex[0].x     = 0;
        vertex[0].Red   = 0xffff;
        vertex[0].Blue  = 0;
        vertex[0].Green = 0;
        vertex[0].Alpha = 0;

        vertex[1].y     = height;
        vertex[1].x     = width;
        vertex[1].Red   = 0;
        vertex[1].Blue  = 0;
        vertex[1].Green = 0xffff;
        vertex[1].Alpha = 0;

        GRADIENT_RECT grect;
        grect.UpperLeft = 0;
        grect.LowerRight = 1;

        GradientFill(hdc, vertex, 2, &grect, 1, GRADIENT_FILL_RECT_V);

        MoveToEx(hdc, 0, 0, NULL);
        LineTo(hdc, 10, 100);
    }
};
