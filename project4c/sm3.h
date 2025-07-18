#ifndef SM3_H
#define SM3_H

#include <cstdint>
#include <cstddef>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdio>
void sm3_init(uint32_t digest[8]);
void sm3_update(uint32_t digest[8], const uint8_t *data, size_t len);
void sm3_final(uint32_t digest[8], const uint8_t *data, size_t len, uint8_t hash[32]);
std::string sm3_hash(const std::string &input);
struct node
{
    uint8_t hash[32];
    uint32_t left, right;
    node *lson, *rson, *fa;
};
extern node *root;
void sm3_hash_bytes(uint8_t *output, const uint8_t *input, size_t len);
void init(node *now, uint32_t left, uint32_t right);
bool check_byte(uint8_t *p, uint8_t *q, uint32_t len);
void compute(node *now, uint32_t tar, const std::string &data_str, uint8_t *hash);
bool proof(const std::string &data_str);
bool proof_pos(const std::string &data_str, uint32_t pos);
void delet(node *now);
void build_tree(const std::vector<std::string> &data);
#endif