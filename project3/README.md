# circom实现poseidon2哈希算法的电路，并用groth16证明
## 依赖库
安装 Circom:
 ```
sudo apt update
sudo apt install -y build-essential libgmp-dev nasm
git clone https://github.com/iden3/circom.git
cd circom
cargo build --release
sudo cp target/release/circom /usr/local/bin/
 ```
 安装 SnarkJS

 `npm install -g snarkjs`
## 设计电路
在bn128上设计一个(256,3,5)的电路。
## 运行流程
```
circom main.circom --r1cs --wasm -o build/
snarkjs powersoftau new bn128 14 build/pot14_0000.ptau
snarkjs powersoftau contribute build/pot14_0000.ptau build/pot14_0001.ptau --name="First contributor"
snarkjs powersoftau prepare phase2 build/pot14_0001.ptau build/pot14_final.ptau
snarkjs groth16 setup build/main.r1cs build/pot14_final.ptau build/poseidon2_0000.zkey
snarkjs zkey contribute build/poseidon2_0000.zkey build/poseidon2.zkey --name="1st Contributor"
snarkjs zkey export verificationkey build/poseidon2.zkey verification_key.json
node build/main_js/generate_witness.js build/main_js/main.wasm input.json build/witness.wtns
snarkjs groth16 prove build/poseidon2.zkey build/witness.wtns proof.json public.json
snarkjs groth16 verify verification_key.json public.json proof.json
```
下面是运行的结果，最后输出OK说明运行成功了：
![](./l1.png)


![](./l2.png)