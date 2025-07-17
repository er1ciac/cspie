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

std::string sm3_hash(const std::string &input);
#endif