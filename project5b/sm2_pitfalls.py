from sm2 import sm2_sign, sm2_verify, scalar_mult, point_add, inv_mod, n,sm3_hash,G
from struct import pack
import hashlib
def calculate_signature(message, key, k):
    """a flawed implementation of SM2 signature using a fixed k value"""
    ENTL = len(b'ALICE123@YH') * 8
    PA = scalar_mult(key, G)
    Za_keyta = (
        pack('>H', ENTL) + b'ALICE123@YH' +
        (0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC).to_bytes(32, 'big') +
        (0x28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93).to_bytes(32, 'big') +
        (0x32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7).to_bytes(32, 'big') +
        (0xBC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0).to_bytes(32, 'big') +
        PA[0].to_bytes(32, 'big') +
        PA[1].to_bytes(32, 'big')
    )
    ZA = sm3_hash(Za_keyta)
    e = int.from_bytes(sm3_hash(ZA + message), 'big')
    x1, y1 = scalar_mult(k, G)
    r = (e + x1) % n
    s = (inv_mod(1 + key, n) * (k - r * key)) % n
    return r, s, e

def leak_k():
    print("===开始k值泄露攻击===")
    key = 0x12345678901234567890123456789012345678901234567890123456789012
    print(f"受害者的私钥: {key}")
    message = b"Test message for SM2 signature"
    r, s = sm2_sign(message, key)
    leaked_k = (s * (1 + key) % n + r * key % n) % n
    recovered_key = ((leaked_k - s) * inv_mod(s + r, n)) % n
    print(f"算出来的私钥: {recovered_key}")
    if recovered_key == key:
        print("k值泄露攻击成功")
    else:
        print("k值泄露攻击失败")

def reuse_k():
    print("===开始k值重用攻击===")
    key = 0x23456789012345678901234567890123456789012345678901234567890123
    message1 = b"First message"
    message2 = b"Second message"
    fixed_k = 0x87654321876543218765432187654321876543218765432187654321876543
    r1, s1, _ = calculate_signature(message1, key, fixed_k)
    r2, s2, _ = calculate_signature(message2, key, fixed_k)
    denominator = (r2 - r1 - s1 + s2) % n
    recovered_key = ((s1 - s2) * inv_mod(denominator, n)) % n
    print(f"受害者的私钥: {key}")
    print(f"算出来的私钥: {recovered_key}")
    if recovered_key == key:
        print("k值重用攻击成功 ")
    else:
        print("k值重用攻击失败")


def reuse_k_two_user():
    print("===开始两用户k值重用攻击===")
    key1 = 0x34567890123456789012345678901234567890123456789012345678901234
    key2 = 0x56789012345678901234567890123456789012345678901234567890123456
    message1 = b"User 1 message"
    message2 = b"User 2 message"
    same_k = 0x98765432109876543210987654321098765432109876543210987654321098
    r1, s1, _ = calculate_signature(message1, key1, same_k)
    r2, s2, _ = calculate_signature(message2, key2, same_k)
    print(f"用户1私钥: {key1}")
    print(f"用户2私钥: {key2}")
    recovered_key2 = ((key1 * (s1 + r1) - s2 + s1) * inv_mod(s2 + r2, n)) % n
    print(f"已知用户1私钥,推导用户2私钥: {recovered_key2}")
    recovered_key1 = ((key2 * (s2 + r2) + s2 - s1) * inv_mod(s1 + r1, n)) % n
    print(f"已知用户2私钥,推导用户1私钥: {recovered_key1}")
    if recovered_key1 == key1 and recovered_key2 == key2:
        print("攻击成功")


#forge signature if the verification does not check message is implemented in project5c
#malleability attack is not feasible  as it is in ECDSA
def same_d_k_with_ecdsa():
    """
    演示在 SM2 和 ECDSA 中使用相同的 d 和 k 导致私钥泄露的安全缺陷
    """
    print("===开始SM2与ECDSA相同d,k攻击===")
    key = 0x12345678901234567890123456789012345678901234567890123456789012
    message = b"Cross-algorithm message"
    same_k = 0xabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefab
    x, y = scalar_mult(same_k, G)
    e1 = int.from_bytes(hashlib.sha256(message).digest(), 'big') % n
    r1 = x % n
    s1 = ((e1 + r1 * key) * inv_mod(same_k, n)) % n
    r2, s2, e2 = calculate_signature(message, key, same_k)
    numerator = (s1 * s2 - e1) % n
    denominator = (r1 - s1 * s2 - s1 * r2) % n
    recovered_key = (numerator * inv_mod(denominator, n)) % n
    print(f"受害者的私钥: {key}")
    print(f"算出来的私钥: {recovered_key}")
    if recovered_key == key:
        print("相同d,k攻击成功 ")
    else:
        print("相同d,k攻击失败")

leak_k()
reuse_k()
reuse_k_two_user()
same_d_k_with_ecdsa()