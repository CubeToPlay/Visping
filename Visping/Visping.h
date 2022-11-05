#pragma once

#include "resource.h"

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

#include <array>
#include <string>
#include <stdio.h>

#define PING_GOOD 60
#define PING_BAD 90

#define ARRAY_LENGTH 20
#define DISCONNECT_VALUE -1

// Define Array
std::array <int, ARRAY_LENGTH> ping_array;

// Insert Into Array
void insert(int value) {
    size_t ping_array_size = ping_array.size();

    if (ping_array_size < ARRAY_LENGTH)
    {
        ping_array[ping_array_size] = value;
    }
    else
    {
        for (int i(ARRAY_LENGTH - 1); i > 0; i--)
        {
            ping_array[i] = ping_array[i - 1];
        }

        ping_array[0] = value;
    }
}

// Returns the statistics of the array
void ping_stats(int& highest, int& lowest, int& average, int& instability) {
    int sum(0);

    highest = ping_array[0];
    lowest = ping_array[0];

    for (const int& indexed_ping : ping_array) {
        if (indexed_ping > highest) highest = indexed_ping;
        if (indexed_ping < lowest) lowest = indexed_ping;

        sum += indexed_ping;
    }

    instability = highest - lowest;

    average = sum / ARRAY_LENGTH;
}

// Ping the server once
void ping_server(std::string server) {
    // Gets ping data
    std::string cmd_ping_output, cmd_command("ping -n 1 " + server);
    std::array<char, 128> buffer;

    FILE* pipe(nullptr);
    pipe = _popen(cmd_command.c_str(), "r");

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        cmd_ping_output += buffer.data();
    }

    _pclose(pipe);

    // Analyze output
    size_t index_begin, index_end;
    const std::string search_text_begin("Average = "),
        search_text_end("ms");

    if (cmd_ping_output.find(search_text_begin) == std::string::npos) {
        insert(DISCONNECT_VALUE);
    }
    else
    {
        index_begin = cmd_ping_output.find(search_text_begin);
        cmd_ping_output = cmd_ping_output.erase(0, index_begin + search_text_begin.length());

        index_end = cmd_ping_output.find(search_text_end);
        cmd_ping_output = cmd_ping_output.erase(index_end, index_end + search_text_end.length());

        insert(std::stoi(cmd_ping_output));
    }
}
