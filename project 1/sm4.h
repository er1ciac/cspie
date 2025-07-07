#ifndef SM4_H
#define SM4_H

#include <cstdint>
#include <vector>

void sm4_key_schedule(const uint8_t key[16], uint32_t rk[32]);
void sm4_key_schedule_decrypt(const uint32_t rk[32], uint32_t drk[32]);
void sm4_encrypt_block(const uint8_t input[16], uint8_t output[16], const uint32_t rk[32]);
void sm4_decrypt_block(const uint8_t input[16], uint8_t output[16], const uint32_t rk[32]);

#endif