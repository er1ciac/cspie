#include "sm3.h"
#include <cstring>

static const uint32_t IV[8] = {
    0x7380166f, 0x4914b2b9,
    0x172442d7, 0xda8a0600,
    0xa96f30bc, 0x163138aa,
    0xe38dee4d, 0xb0fb0e4e};

static uint32_t ROTL(uint32_t x, int n)
{
    return (x << n) | (x >> (32 - n));
}

static uint32_t P0(uint32_t x)
{
    return x ^ ROTL(x, 9) ^ ROTL(x, 17);
}

static uint32_t P1(uint32_t x)
{
    return x ^ ROTL(x, 15) ^ ROTL(x, 23);
}

static uint32_t FF(uint32_t x, uint32_t y, uint32_t z, int j)
{
    return (j < 16) ? (x ^ y ^ z) : ((x & y) | (x & z) | (y & z));
}

static uint32_t GG(uint32_t x, uint32_t y, uint32_t z, int j)
{
    return (j < 16) ? (x ^ y ^ z) : ((x & y) | (~x & z));
}

static const uint32_t T[64] = {
    0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519,
    0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519,
    0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519,
    0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519,
    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a,
    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a,
    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a,
    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a,
    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a,
    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a,
    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a,
    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a,
    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a,
    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a,
    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a};

static void sm3_compress(uint32_t V[8], const uint8_t block[64])
{
    uint32_t W[68], W1[64];
    for (int i = 0; i < 16; ++i)
    {
        W[i] = ((uint32_t)block[4 * i] << 24) | ((uint32_t)block[4 * i + 1] << 16) |
               ((uint32_t)block[4 * i + 2] << 8) | (uint32_t)block[4 * i + 3];
    }
    for (int i = 16; i < 68; ++i)
    {
        W[i] = P1(W[i - 16] ^ W[i - 9] ^ ROTL(W[i - 3], 15)) ^ ROTL(W[i - 13], 7) ^ W[i - 6];
    }
    for (int i = 0; i < 64; ++i)
    {
        W1[i] = W[i] ^ W[i + 4];
    }

    uint32_t A = V[0], B = V[1], C = V[2], D = V[3];
    uint32_t E = V[4], F = V[5], G = V[6], H = V[7];

    for (int j = 0; j < 64; ++j)
    {
        uint32_t SS1 = ROTL((ROTL(A, 12) + E + ROTL(T[j], j)) & 0xFFFFFFFF, 7);
        uint32_t SS2 = SS1 ^ ROTL(A, 12);
        uint32_t TT1 = (FF(A, B, C, j) + D + SS2 + W1[j]) & 0xFFFFFFFF;
        uint32_t TT2 = (GG(E, F, G, j) + H + SS1 + W[j]) & 0xFFFFFFFF;
        D = C;
        C = ROTL(B, 9);
        B = A;
        A = TT1;
        H = G;
        G = ROTL(F, 19);
        F = E;
        E = P0(TT2);
    }

    V[0] ^= A;
    V[1] ^= B;
    V[2] ^= C;
    V[3] ^= D;
    V[4] ^= E;
    V[5] ^= F;
    V[6] ^= G;
    V[7] ^= H;
}

void sm3_init(uint32_t digest[8])
{
    for (int i = 0; i < 8; ++i)
        digest[i] = IV[i];
}

void sm3_update(uint32_t digest[8], const uint8_t *data, size_t len)
{
    while (len >= 64)
    {
        sm3_compress(digest, data);
        data += 64;
        len -= 64;
    }
}

void sm3_final(uint32_t digest[8], const uint8_t *data, size_t len, uint8_t hash[32])
{
    uint8_t buffer[64] = {0};
    memcpy(buffer, data, len);

    buffer[len] = 0x80;
    size_t pad_len = (len < 56) ? 56 : 120;
    uint64_t total_bits = (len * 8);
    buffer[pad_len++] = (total_bits >> 56) & 0xFF;
    buffer[pad_len++] = (total_bits >> 48) & 0xFF;
    buffer[pad_len++] = (total_bits >> 40) & 0xFF;
    buffer[pad_len++] = (total_bits >> 32) & 0xFF;
    buffer[pad_len++] = (total_bits >> 24) & 0xFF;
    buffer[pad_len++] = (total_bits >> 16) & 0xFF;
    buffer[pad_len++] = (total_bits >> 8) & 0xFF;
    buffer[pad_len++] = total_bits & 0xFF;

    sm3_compress(digest, buffer);
    if (len >= 56)
        sm3_compress(digest, buffer + 64);

    for (int i = 0; i < 8; ++i)
    {
        hash[i * 4] = (digest[i] >> 24) & 0xFF;
        hash[i * 4 + 1] = (digest[i] >> 16) & 0xFF;
        hash[i * 4 + 2] = (digest[i] >> 8) & 0xFF;
        hash[i * 4 + 3] = digest[i] & 0xFF;
    }
}
