
#include "sm3.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>

node *root;
std::vector<std::string> data_blocks;

void sm3_hash_bytes(uint8_t *output, const uint8_t *input, size_t len)
{
    std::string input_str((char *)input, len);
    std::string hash_str = sm3_hash(input_str);
    for (int i = 0; i < 32; i++)
    {
        sscanf(hash_str.substr(i * 2, 2).c_str(), "%02x", (unsigned int *)&output[i]);
    }
}

void init(node *now, uint32_t left, uint32_t right)
{
    now->left = left;
    now->right = right;
    now->fa = nullptr;

    if (left == right)
    {
        std::string data = data_blocks[left];
        uint8_t tmp[256];
        tmp[0] = 0;
        memcpy(tmp + 1, data.c_str(), data.length());
        sm3_hash_bytes(now->hash, tmp, data.length() + 1);
        now->lson = nullptr;
        now->rson = nullptr;
        return;
    }

    now->lson = (node *)malloc(sizeof(node));
    now->rson = (node *)malloc(sizeof(node));
    now->lson->fa = now;
    now->rson->fa = now;

    init(now->lson, left, (left + right) / 2);
    init(now->rson, ((left + right) / 2) + 1, right);

    uint8_t tmp[65];
    tmp[0] = 1;
    memcpy(tmp + 1, now->lson->hash, 32);
    memcpy(tmp + 33, now->rson->hash, 32);
    sm3_hash_bytes(now->hash, tmp, 65);
}

bool check_byte(uint8_t *p, uint8_t *q, uint32_t len)
{
    for (int i = 0; i < len; i++)
    {
        if (p[i] != q[i])
            return false;
    }
    return true;
}

void compute(node *now, uint32_t tar, const std::string &data_str, uint8_t *hash)
{
    if (!now)
        return;

    if (now->left == now->right)
    {
        uint8_t tmp[256];
        tmp[0] = 0;
        memcpy(tmp + 1, data_str.c_str(), data_str.length());
        sm3_hash_bytes(hash, tmp, data_str.length() + 1);
        return;
    }
    uint32_t mid = (now->left + now->right) / 2;
    uint8_t left_hash[32], right_hash[32];

    if (tar <= mid)
    {
        compute(now->lson, tar, data_str, left_hash);
        memcpy(right_hash, now->rson->hash, 32);
    }
    else
    {
        memcpy(left_hash, now->lson->hash, 32);
        compute(now->rson, tar, data_str, right_hash);
    }

    uint8_t tmp[65];
    tmp[0] = 1;
    memcpy(tmp + 1, left_hash, 32);
    memcpy(tmp + 33, right_hash, 32);
    sm3_hash_bytes(hash, tmp, 65);
}

bool proof(const std::string &data_str)
{
    uint8_t hash[32];
    for (int i = 0; i < data_blocks.size(); i++)
    {
        if (data_blocks[i] == data_str)
        {
            compute(root, i, data_str, hash);
            bool result = check_byte(root->hash, hash, 32);
            printf("哈希比较结果: %s\n", result ? "匹配" : "不匹配");
            return result;
        }
    }
    return false;
}

bool proof_pos(const std::string &data_str, uint32_t pos)
{
    if (!root || pos >= data_blocks.size())
        return false;
    return (data_blocks[pos] == data_str);
}

void delet(node *now)
{
    if (!now)
        return;

    if (now->left == now->right)
    {
        free(now);
        return;
    }
    if (now->lson)
        delet(now->lson);
    if (now->rson)
        delet(now->rson);
    free(now);
}

void build_tree(const std::vector<std::string> &data)
{
    data_blocks = data;
    root = (node *)malloc(sizeof(node));
    init(root, 0, data_blocks.size() - 1);
}
