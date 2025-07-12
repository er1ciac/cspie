import os
from struct import pack
from gmssl.sm3 import sm3_hash as _sm3_hash_hex

def sm3_hash(msg: bytes):
    msg_list = list(msg)
    hexstr = _sm3_hash_hex(msg_list)
    return bytes.fromhex(hexstr)

def kdf(Z: bytes, klen: int):
    ct = 1
    t = b''
    for _ in range((klen + 31) // 32):
        t += sm3_hash(Z + pack('>I', ct))
        ct += 1
    return t[:klen]

p = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF
a = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC
b = 0x28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93
n = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123
Gx = 0x32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7
Gy = 0xBC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0
G = (Gx, Gy)
h_cofactor = 1 

def inv_mod(x: int, m: int):
    return pow(x, m-2, m)

def point_add(P: tuple, Q: tuple) :
    if P is None: return Q
    if Q is None: return P
    x1, y1 = P; x2, y2 = Q
    if x1 == x2 and (y1 + y2) % p == 0:
        return None
    if P != Q:
        lam = ((y2 - y1) * inv_mod(x2 - x1, p)) % p
    else:
        lam = ((3 * x1 * x1 + a) * inv_mod(2 * y1, p)) % p
    x3 = (lam*lam - x1 - x2) % p
    y3 = (lam*(x1 - x3) - y1) % p
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

def sm2_key_exchange_A(dA: int, PA: tuple, PB: tuple,
                       IDA: bytes, IDB: bytes, klen: int):
    def calc_Z(ID, P):
        ENTL = len(ID) * 8
        return sm3_hash(
            pack('>H', ENTL) + ID +
            a.to_bytes(32,'big') + b.to_bytes(32,'big') +
            Gx.to_bytes(32,'big') + Gy.to_bytes(32,'big') +
            P[0].to_bytes(32,'big') + P[1].to_bytes(32,'big')
        )
    ZA = calc_Z(IDA, PA)
    ZB = calc_Z(IDB, PB)
    rA = int.from_bytes(os.urandom(32), 'big') % (n - 1) + 1
    RA = scalar_mult(rA, G)
    x1 = RA[0]
    w = (n.bit_length() + 1) // 2 - 1
    x1_ = (x1 & ((1 << w) - 1)) | (1 << w)
    tA = (dA + x1_ * rA) % n
    return RA, ZA, ZB, tA

def sm2_key_exchange_B(dB: int, PB: tuple, PA: tuple,
                       IDB: bytes, IDA: bytes, RA: tuple, klen: int):
    def calc_Z(ID, P):
        ENTL = len(ID) * 8
        return sm3_hash(
            pack('>H', ENTL) + ID +
            a.to_bytes(32,'big') + b.to_bytes(32,'big') +
            Gx.to_bytes(32,'big') + Gy.to_bytes(32,'big') +
            P[0].to_bytes(32,'big') + P[1].to_bytes(32,'big')
        )
    ZB = calc_Z(IDB, PB)
    ZA = calc_Z(IDA, PA)
    rB = int.from_bytes(os.urandom(32), 'big') % (n - 1) + 1
    RB = scalar_mult(rB, G)
    x2 = RB[0]
    w = (n.bit_length() + 1) // 2 - 1
    x2_ = (x2 & ((1 << w) - 1)) | (1 << w)
    tB = (dB + x2_ * rB) % n
    x1 = RA[0]
    x1_ = (x1 & ((1 << w) - 1)) | (1 << w)
    S = point_add(PA, scalar_mult(x1_, RA))
    U = scalar_mult((h_cofactor * tB) % n, S)
    xU, yU = U
    KA = kdf(xU.to_bytes(32,'big') + yU.to_bytes(32,'big') + ZA + ZB, klen)
    return RB, KA, tB

def sm2_key_exchange_final_A(tA: int, RA: tuple, RB: tuple,
                             PA: tuple, PB: tuple, ZA: bytes, ZB: bytes, klen: int):
    w = (n.bit_length() + 1) // 2 - 1
    x2 = RB[0]
    x2_ = (x2 & ((1 << w) - 1)) | (1 << w)
    S = point_add(PB, scalar_mult(x2_, RB))
    U = scalar_mult((h_cofactor * tA) % n, S)
    xU, yU = U
    KA = kdf(xU.to_bytes(32,'big') + yU.to_bytes(32,'big') + ZA + ZB, klen)
    return KA

IDA = b'ALICE123'
IDB = b'BOB456'
dA = int.from_bytes(os.urandom(32), 'big') % (n-1) + 1
dB = int.from_bytes(os.urandom(32), 'big') % (n-1) + 1
PA = scalar_mult(dA, G)
PB = scalar_mult(dB, G)
RA, ZA, ZB, tA = sm2_key_exchange_A(dA, PA, PB, IDA, IDB, klen=32)
RB, KB_B, tB = sm2_key_exchange_B(dB, PB, PA, IDB, IDA, RA, klen=32)
KB_A = sm2_key_exchange_final_A(tA, RA, RB, PA, PB, ZA, ZB, klen=32)
print("A 端共享密钥:", KB_A.hex())
print("B 端共享密钥:", KB_B.hex())
print("一致:", KB_A == KB_B)