#include "sm3.h"
#include <iostream>
#include <cstdio>
#include <cstring>

int main()
{
    const char *msg = "sm3 example message";
    uint32_t digest[8];
    uint8_t hash[32];
    {
        Timer t("SM3 start");
        for (int i = 0; i < 10000; ++i)
        {
            sm3_init(digest);
            sm3_update(digest, (const uint8_t *)msg, strlen(msg));
            sm3_final(digest, (const uint8_t *)msg, strlen(msg), hash);
        }
    }

    std::cout << "SM3(\"" << msg << "\") = ";
    for (int i = 0; i < 32; ++i)
    {
        printf("%02x", hash[i]);
    }
    std::cout << std::endl;

    return 0;
}
