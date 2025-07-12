#ifndef SM3_H
#define SM3_H

#include <cstdint>
#include <cstddef>
#include <chrono>
#include <iostream>
#include <string>
void sm3_init(uint32_t digest[8]);
void sm3_update(uint32_t digest[8], const uint8_t *data, size_t len);
void sm3_final(uint32_t digest[8], const uint8_t *data, size_t len, uint8_t hash[32]);

class Timer
{
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    std::string name;

public:
    Timer(const std::string &name) : name(name)
    {
        start_time = std::chrono::high_resolution_clock::now();
    }
    ~Timer()
    {
        auto end_time = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        std::cout << name << " took " << duration << " ms" << std::endl;
    }
};
#endif