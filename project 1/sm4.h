#ifndef SM4_H
#define SM4_H
#include <cstdint>
#include <vector>
#include <chrono>
#include <iostream>
#include <string>
void sm4_key_schedule(const uint8_t key[16], uint32_t rk[32]);
void sm4_key_schedule_decrypt(const uint32_t rk[32], uint32_t drk[32]);
void sm4_encrypt_block(const uint8_t input[16], uint8_t output[16], const uint32_t rk[32]);
void sm4_decrypt_block(const uint8_t input[16], uint8_t output[16], const uint32_t rk[32]);
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