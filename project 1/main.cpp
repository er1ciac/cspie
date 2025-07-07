#include <iostream>
#include "sm4.h"
int main()
{
    uint8_t key[16] = {
        0x01, 0x23, 0x45, 0x67,
        0x89, 0xab, 0xcd, 0xef,
        0xfe, 0xdc, 0xba, 0x98,
        0x76, 0x54, 0x32, 0x10};
    uint8_t input[16] = {
        0x01, 0x23, 0x45, 0x67,
        0x89, 0xab, 0xcd, 0xef,
        0xfe, 0xdc, 0xba, 0x98,
        0x76, 0x54, 0x32, 0x10};
    uint8_t encrypted[16];
    uint8_t decrypted[16];
    uint32_t rk[32], drk[32];

    std::cout << "original : ";
    for (auto c : input)
    {
        printf("%02x ", c);
    }
    std::cout << std::endl;

    sm4_key_schedule(key, rk);
    sm4_key_schedule_decrypt(rk, drk);

    {
        Timer t("SM4 Encryption");
        for (int i = 0; i < 1000; i++)
            sm4_encrypt_block(input, encrypted, rk);
    }

    std::cout << "Ciphertext: ";
    for (auto c : encrypted)
    {
        printf("%02x ", c);
    }
    std::cout << std::endl;

    {
        Timer t1("SM4 Decryption");
        sm4_decrypt_block(encrypted, decrypted, drk);
    }

    std::cout << "Decrypted : ";
    for (auto c : decrypted)
    {
        printf("%02x ", c);
    }
    std::cout << std::endl;
    return 0;
}