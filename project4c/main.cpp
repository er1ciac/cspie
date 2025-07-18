
#include "sm3.h"
#include <iostream>
#include <vector>
#include <string>

int main()
{
    std::vector<std::string> data;
    for (int i = 0; i < 5; ++i)
    {
        data.push_back("block_" + std::to_string(i));
    }

    build_tree(data);
    std::cout << "Merkle Tree built successfully." << std::endl;

    int index = 1;
    std::string test_data = "block_" + std::to_string(index);
    std::cout << "测试数据: " << test_data << std::endl;
    bool is_valid = proof_pos(test_data, index);
    std::cout << "存在性证明结果: " << (is_valid ? "存在" : "不存在") << std::endl;

    std::string fake_data = "block_199";
    std::cout << "测试数据: " << fake_data << std::endl;
    bool fake_is_valid = proof(fake_data);
    std::cout << "不存在性证明结果: " << (fake_is_valid ? "存在!" : "不存在") << std::endl;
    delet(root);
    return 0;
}