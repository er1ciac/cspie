import os
from struct import pack
from gmssl.sm3 import sm3_hash as _sm3_hash_hex

def sm3_hash(msg: bytes):
    msg_list = list(msg)
    hexstr = _sm3_hash_hex(msg_list)
    return bytes.fromhex(hexstr)

p = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF
a = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC
b = 0x28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93
n = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123
Gx = 0x32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7
Gy = 0xBC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0
G = (Gx, Gy)

def inv_mod(x: int, m: int) :
    return pow(x, m-2, m)

def point_add(P: tuple, Q: tuple):
    if P is None: return Q
    if Q is None: return P
    x1, y1 = P; x2, y2 = Q
    if x1 == x2 and (y1 + y2) % p == 0:
        return None
    if P != Q:
        l = ((y2 - y1) * inv_mod(x2 - x1, p)) % p
    else:
        l = ((3 * x1 * x1 + a) * inv_mod(2 * y1, p)) % p
    x3 = (l*l - x1 - x2) % p
    y3 = (l*(x1 - x3) - y1) % p
    return (x3, y3)

def scalar_mult(k: int, P: tuple):
    Q = None
    N = P
    while k:
        if k & 1:
            Q = point_add(Q, N)
        N = point_add(N, N)
        k >>= 1
    return Q

def kdf(Z: bytes, klen: int) :
    ct = 1
    t = b''
    for _ in range((klen + 31) // 32):
        t += sm3_hash(Z + pack('>I', ct))
        ct += 1
    return t[:klen]

def sm2_encrypt(message: bytes, PA: tuple):
    mlen = len(message)
    while True:
        k = int.from_bytes(os.urandom(32), 'big') % (n-1) + 1
        x1, y1 = scalar_mult(k, G)
        x2, y2 = scalar_mult(k, PA)
        Z = x2.to_bytes(32,'big') + y2.to_bytes(32,'big')
        t = kdf(Z, mlen)
        if any(t): 
            break
    C2 = bytes(a ^ b for a, b in zip(message, t))
    C3 = sm3_hash(x2.to_bytes(32,'big') + message + y2.to_bytes(32,'big'))
    C1 = b'\x04' + x1.to_bytes(32,'big') + y1.to_bytes(32,'big')  # 非压缩点格式
    return C1 + C3 + C2

def sm2_decrypt(cipher: bytes, dA: int):
    assert cipher[0] == 0x04
    x1 = int.from_bytes(cipher[1:33], 'big')
    y1 = int.from_bytes(cipher[33:65], 'big')
    C1 = (x1, y1)
    C3 = cipher[65:97]
    C2 = cipher[97:]
    mlen = len(C2)
    x2, y2 = scalar_mult(dA, C1)
    Z = x2.to_bytes(32,'big') + y2.to_bytes(32,'big')
    t = kdf(Z, mlen)
    M = bytes(a ^ b for a, b in zip(C2, t))
    u = sm3_hash(x2.to_bytes(32,'big') + M + y2.to_bytes(32,'big'))
    if u != C3:
        raise ValueError("Invalid cipher text or wrong private key")
    return M


dA = int.from_bytes(os.urandom(32), 'big') % (n-1) + 1
PA = scalar_mult(dA, G)

msg = b'SM2 encryption'
cipher = sm2_encrypt(msg, PA)
print("Cipher (hex):", cipher.hex())

plain = sm2_decrypt(cipher, dA)
print("Decrypted:", plain)