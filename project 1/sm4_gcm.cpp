#include "sm4.h"
#include <cstring>
#include <algorithm>

const uint64_t GF128_POLY = 0x87;

static void reverse_bytes(uint8_t *data, size_t len)
{
    for (size_t i = 0; i < len / 2; i++)
    {
        std::swap(data[i], data[len - 1 - i]);
    }
}

static void gf128_lshift(uint8_t *block)
{
    uint64_t *p = (uint64_t *)block;
    uint64_t carry = p[0] >> 63;
    p[0] = (p[0] << 1) | (p[1] >> 63);
    p[1] = (p[1] << 1);

    if (carry)
    {
        p[1] ^= GF128_POLY;
    }
}

static void gf128_mul(const uint8_t *a, const uint8_t *b, uint8_t *result)
{
    uint8_t z[16] = {0};
    uint8_t v[16];
    memcpy(v, b, 16);

    for (int i = 0; i < 128; i++)
    {
        if (a[i / 8] & (0x80 >> (i % 8)))
        {
            for (int j = 0; j < 16; j++)
            {
                z[j] ^= v[j];
            }
        }

        gf128_lshift(v);
    }

    memcpy(result, z, 16);
}

static void ghash(const uint8_t *h, const uint8_t *data, size_t len, uint8_t *result)
{
    uint8_t y[16] = {0};

    for (size_t i = 0; i < len; i += 16)
    {
        for (int j = 0; j < 16; j++)
        {
            if (i + j < len)
            {
                y[j] ^= data[i + j];
            }
        }

        gf128_mul(y, h, y);
    }

    memcpy(result, y, 16);
}

static void inc32(uint8_t *block)
{
    uint32_t *counter = (uint32_t *)(block + 12);
    *counter = __builtin_bswap32(__builtin_bswap32(*counter) + 1);
}

static void gctr(const uint32_t *rk, const uint8_t *icb, const uint8_t *plaintext,
                 size_t len, uint8_t *ciphertext)
{
    uint8_t cb[16];
    uint8_t y[16];
    memcpy(cb, icb, 16);

    for (size_t i = 0; i < len; i += 16)
    {
        sm4_encrypt_block(cb, y, rk);
        size_t block_len = std::min(16UL, len - i);
        for (size_t j = 0; j < block_len; j++)
        {
            ciphertext[i + j] = plaintext[i + j] ^ y[j];
        }
        inc32(cb);
    }
}

void sm4_gcm_encrypt(const uint8_t *key, const uint8_t *iv, size_t iv_len,
                     const uint8_t *aad, size_t aad_len,
                     const uint8_t *plaintext, size_t plaintext_len,
                     uint8_t *ciphertext, uint8_t *tag)
{
    uint32_t rk[32];
    sm4_key_schedule(key, rk);
    uint8_t h[16] = {0};
    sm4_encrypt_block(h, h, rk);
    uint8_t j0[16] = {0};
    if (iv_len == 12)
    {
        memcpy(j0, iv, 12);
        j0[15] = 1;
    }
    else
    {
        size_t total_len = ((iv_len + 15) / 16) * 16 + 16;
        uint8_t *s = new uint8_t[total_len]();
        memcpy(s, iv, iv_len);

        uint64_t bit_len = iv_len * 8;
        s[total_len - 8] = (bit_len >> 56) & 0xff;
        s[total_len - 7] = (bit_len >> 48) & 0xff;
        s[total_len - 6] = (bit_len >> 40) & 0xff;
        s[total_len - 5] = (bit_len >> 32) & 0xff;
        s[total_len - 4] = (bit_len >> 24) & 0xff;
        s[total_len - 3] = (bit_len >> 16) & 0xff;
        s[total_len - 2] = (bit_len >> 8) & 0xff;
        s[total_len - 1] = bit_len & 0xff;

        ghash(h, s, total_len, j0);
        delete[] s;
    }

    uint8_t j0_inc[16];
    memcpy(j0_inc, j0, 16);
    inc32(j0_inc);
    gctr(rk, j0_inc, plaintext, plaintext_len, ciphertext);
    size_t u = (16 - (plaintext_len % 16)) % 16;
    size_t v = (16 - (aad_len % 16)) % 16;
    size_t ghash_len = aad_len + v + plaintext_len + u + 16;
    uint8_t *ghash_input = new uint8_t[ghash_len]();
    size_t pos = 0;
    if (aad_len > 0)
    {
        memcpy(ghash_input + pos, aad, aad_len);
    }
    pos += aad_len + v;

    memcpy(ghash_input + pos, ciphertext, plaintext_len);
    pos += plaintext_len + u;

    uint64_t aad_bit_len = aad_len * 8;
    uint64_t plaintext_bit_len = plaintext_len * 8;

    ghash_input[pos++] = (aad_bit_len >> 56) & 0xff;
    ghash_input[pos++] = (aad_bit_len >> 48) & 0xff;
    ghash_input[pos++] = (aad_bit_len >> 40) & 0xff;
    ghash_input[pos++] = (aad_bit_len >> 32) & 0xff;
    ghash_input[pos++] = (aad_bit_len >> 24) & 0xff;
    ghash_input[pos++] = (aad_bit_len >> 16) & 0xff;
    ghash_input[pos++] = (aad_bit_len >> 8) & 0xff;
    ghash_input[pos++] = aad_bit_len & 0xff;

    ghash_input[pos++] = (plaintext_bit_len >> 56) & 0xff;
    ghash_input[pos++] = (plaintext_bit_len >> 48) & 0xff;
    ghash_input[pos++] = (plaintext_bit_len >> 40) & 0xff;
    ghash_input[pos++] = (plaintext_bit_len >> 32) & 0xff;
    ghash_input[pos++] = (plaintext_bit_len >> 24) & 0xff;
    ghash_input[pos++] = (plaintext_bit_len >> 16) & 0xff;
    ghash_input[pos++] = (plaintext_bit_len >> 8) & 0xff;
    ghash_input[pos++] = plaintext_bit_len & 0xff;

    uint8_t s[16];
    ghash(h, ghash_input, ghash_len, s);
    gctr(rk, j0, s, 16, tag);

    delete[] ghash_input;
}

bool sm4_gcm_decrypt(const uint8_t *key, const uint8_t *iv, size_t iv_len,
                     const uint8_t *aad, size_t aad_len,
                     const uint8_t *ciphertext, size_t ciphertext_len,
                     const uint8_t *tag, uint8_t *plaintext)
{
    uint32_t rk[32];
    sm4_key_schedule(key, rk);

    uint8_t h[16] = {0};
    sm4_encrypt_block(h, h, rk);

    uint8_t j0[16] = {0};
    if (iv_len == 12)
    {
        memcpy(j0, iv, 12);
        j0[15] = 1;
    }
    else
    {
        size_t total_len = ((iv_len + 15) / 16) * 16 + 16;
        uint8_t *s = new uint8_t[total_len]();
        memcpy(s, iv, iv_len);
        uint64_t bit_len = iv_len * 8;
        s[total_len - 8] = (bit_len >> 56) & 0xff;
        s[total_len - 7] = (bit_len >> 48) & 0xff;
        s[total_len - 6] = (bit_len >> 40) & 0xff;
        s[total_len - 5] = (bit_len >> 32) & 0xff;
        s[total_len - 4] = (bit_len >> 24) & 0xff;
        s[total_len - 3] = (bit_len >> 16) & 0xff;
        s[total_len - 2] = (bit_len >> 8) & 0xff;
        s[total_len - 1] = bit_len & 0xff;

        ghash(h, s, total_len, j0);
        delete[] s;
    }

    uint8_t j0_inc[16];
    memcpy(j0_inc, j0, 16);
    inc32(j0_inc);

    size_t u = (16 - (ciphertext_len % 16)) % 16;
    size_t v = (16 - (aad_len % 16)) % 16;
    size_t ghash_len = aad_len + v + ciphertext_len + u + 16;
    uint8_t *ghash_input = new uint8_t[ghash_len]();
    size_t pos = 0;
    if (aad_len > 0)
    {
        memcpy(ghash_input + pos, aad, aad_len);
    }
    pos += aad_len + v;
    memcpy(ghash_input + pos, ciphertext, ciphertext_len);
    pos += ciphertext_len + u;
    uint64_t aad_bit_len = aad_len * 8;
    uint64_t ciphertext_bit_len = ciphertext_len * 8;

    ghash_input[pos++] = (aad_bit_len >> 56) & 0xff;
    ghash_input[pos++] = (aad_bit_len >> 48) & 0xff;
    ghash_input[pos++] = (aad_bit_len >> 40) & 0xff;
    ghash_input[pos++] = (aad_bit_len >> 32) & 0xff;
    ghash_input[pos++] = (aad_bit_len >> 24) & 0xff;
    ghash_input[pos++] = (aad_bit_len >> 16) & 0xff;
    ghash_input[pos++] = (aad_bit_len >> 8) & 0xff;
    ghash_input[pos++] = aad_bit_len & 0xff;

    ghash_input[pos++] = (ciphertext_bit_len >> 56) & 0xff;
    ghash_input[pos++] = (ciphertext_bit_len >> 48) & 0xff;
    ghash_input[pos++] = (ciphertext_bit_len >> 40) & 0xff;
    ghash_input[pos++] = (ciphertext_bit_len >> 32) & 0xff;
    ghash_input[pos++] = (ciphertext_bit_len >> 24) & 0xff;
    ghash_input[pos++] = (ciphertext_bit_len >> 16) & 0xff;
    ghash_input[pos++] = (ciphertext_bit_len >> 8) & 0xff;
    ghash_input[pos++] = ciphertext_bit_len & 0xff;

    uint8_t s[16];
    ghash(h, ghash_input, ghash_len, s);

    uint8_t expected_tag[16];
    gctr(rk, j0, s, 16, expected_tag);

    bool auth_valid = (memcmp(tag, expected_tag, 16) == 0);

    if (auth_valid)
    {
        gctr(rk, j0_inc, ciphertext, ciphertext_len, plaintext);
    }

    delete[] ghash_input;
    return auth_valid;
}

int main()
{
    uint8_t key[16] = {
        0x01, 0x23, 0x45, 0x67,
        0x89, 0xab, 0xcd, 0xef,
        0xfe, 0xdc, 0xba, 0x98,
        0x76, 0x54, 0x32, 0x10};

    uint8_t iv[12] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x01};

    const char *aad_str = "Authenticated";
    uint8_t *aad = (uint8_t *)aad_str;
    size_t aad_len = strlen(aad_str);

    const char *plaintext_str = "Hello, plaintext";
    uint8_t *plaintext = (uint8_t *)plaintext_str;
    size_t plaintext_len = strlen(plaintext_str);

    uint8_t *ciphertext = new uint8_t[plaintext_len];
    uint8_t *decrypted = new uint8_t[plaintext_len];
    uint8_t tag[16];

    std::cout << "Plaintext: " << plaintext_str << std::endl;
    std::cout << "AAD: " << aad_str << std::endl;

    sm4_gcm_encrypt(key, iv, sizeof(iv), aad, aad_len, plaintext, plaintext_len, ciphertext, tag);

    std::cout << "Ciphertext: ";
    for (size_t i = 0; i < plaintext_len; i++)
    {
        printf("%02x ", ciphertext[i]);
    }
    std::cout << std::endl;

    std::cout << "Auth Tag: ";
    for (int i = 0; i < 16; i++)
    {
        printf("%02x ", tag[i]);
    }
    std::cout << std::endl;

    bool auth_result;

    auth_result = sm4_gcm_decrypt(key, iv, sizeof(iv), aad, aad_len, ciphertext, plaintext_len, tag, decrypted);

    std::cout << "Authentication: " << (auth_result ? "PASS" : "FAIL") << std::endl;

    if (auth_result)
    {
        std::cout << "Decrypted: ";
        for (size_t i = 0; i < plaintext_len; i++)
        {
            printf("%c", decrypted[i]);
        }
        std::cout << std::endl;
    }

    delete[] ciphertext;
    delete[] decrypted;
    return 0;
}