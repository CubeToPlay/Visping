#pragma once

#include "resource.h"
#include "ping.h"

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
