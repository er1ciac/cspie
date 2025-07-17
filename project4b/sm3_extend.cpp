#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <cstring>
#include <cstdint>
#include <sstream>

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

static uint32_t T[64] = {
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

std::string sm3_hash_with_iv(const std::vector<uint8_t> &msg, const uint32_t *iv)
{
    std::vector<uint8_t> padded_msg = msg;
    uint64_t len_in_bits = msg.size() * 8;
    padded_msg.push_back(0x80);
    while ((padded_msg.size() * 8) % 512 != 448)
    {
        padded_msg.push_back(0x00);
    }

    for (int i = 7; i >= 0; i--)
    {
        padded_msg.push_back((len_in_bits >> (i * 8)) & 0xFF);
    }

    uint32_t digest[8];
    for (int i = 0; i < 8; i++)
    {
        digest[i] = iv[i];
    }

    for (size_t i = 0; i < padded_msg.size(); i += 64)
    {
        sm3_compress(digest, &padded_msg[i]);
    }

    std::stringstream ss;
    for (int i = 0; i < 8; i++)
    {
        ss << std::hex << std::setfill('0') << std::setw(8) << digest[i];
    }
    return ss.str();
}

std::vector<uint8_t> sm3_padding(size_t msg_len_bytes)
{
    uint64_t ml = msg_len_bytes * 8;
    std::vector<uint8_t> pad;

    pad.push_back(0x80);
    while ((msg_len_bytes + pad.size()) % 64 != 56)
    {
        pad.push_back(0x00);
    }
    for (int i = 7; i >= 0; i--)
    {
        pad.push_back((ml >> (i * 8)) & 0xFF);
    }

    return pad;
}

void split_digest_to_iv(const std::string &digest_hex, uint32_t *iv)
{
    for (int i = 0; i < 8; i++)
    {
        std::string hex_part = digest_hex.substr(i * 8, 8);
        iv[i] = std::stoul(hex_part, nullptr, 16);
    }
}

int main()
{
    std::string orig_digest = "0ca841e92a5bb9d9f7b0e16e4ce28a0e954b36c04cc4bd0a0dbc3f167d6ea147";
    std::string public_msg = "message";
    std::string secret_prefix = "sm3 example ";
    std::string append_data = "1234";
    size_t orig_full_len = secret_prefix.length() + public_msg.length();
    std::vector<uint8_t> padding = sm3_padding(orig_full_len);
    std::vector<uint8_t> forged_msg_public;
    for (char c : public_msg)
        forged_msg_public.push_back(c);
    forged_msg_public.insert(forged_msg_public.end(), padding.begin(), padding.end());
    for (char c : append_data)
        forged_msg_public.push_back(c);
    std::vector<uint8_t> forged_msg_full;
    for (char c : secret_prefix)
        forged_msg_full.push_back(c);
    forged_msg_full.insert(forged_msg_full.end(), forged_msg_public.begin(), forged_msg_public.end());
    uint32_t iv[8];
    split_digest_to_iv(orig_digest, iv);
    uint32_t current_state[8];
    for (int i = 0; i < 8; i++)
    {
        current_state[i] = iv[i];
    }

    std::vector<uint8_t> append_bytes;
    for (char c : append_data)
        append_bytes.push_back(c);
    uint64_t total_new_len = forged_msg_full.size() * 8;
    std::vector<uint8_t> padded_append = append_bytes;
    padded_append.push_back(0x80);
    while (padded_append.size() % 64 != 56)
    {
        padded_append.push_back(0x00);
    }
    for (int i = 7; i >= 0; i--)
    {
        padded_append.push_back((total_new_len >> (i * 8)) & 0xFF);
    }
    sm3_compress(current_state, &padded_append[0]);
    std::stringstream forged_digest_ss;
    for (int i = 0; i < 8; i++)
    {
        forged_digest_ss << std::hex << std::setfill('0') << std::setw(8) << current_state[i];
    }
    std::string forged_digest = forged_digest_ss.str();
    std::string verification_hash = sm3_hash_with_iv(forged_msg_full, IV);
    bool attack_success = (verification_hash == forged_digest);
    std::cout << "伪造摘要: " << forged_digest << std::endl;
    std::cout << "真实摘要: " << verification_hash << std::endl;
    std::cout << (attack_success ? "攻击成功" : "攻击失败") << std::endl;
    return 0;
}