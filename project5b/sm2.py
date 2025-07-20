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
    return pow(x, m - 2, m)

def point_add(P: tuple, Q: tuple) :
    if P is None: return Q
    if Q is None: return P
    x1, y1 = P; x2, y2 = Q
    if x1 == x2 and (y1 + y2) % p == 0:
        return None
    if P != Q:
        l = ((y2 - y1) * inv_mod(x2 - x1, p)) % p
    else:
        l = ((3 * x1 * x1 + a) * inv_mod(2 * y1, p)) % p
    x3 = (l * l - x1 - x2) % p
    y3 = (l * (x1 - x3) - y1) % p
    return (x3, y3)

def scalar_mult(k: int, P: tuple) :
    Q = None
    N = P
    while k:
        if k & 1:
            Q = point_add(Q, N)
        N = point_add(N, N)
        k >>= 1
    return Q

def sm2_sign(message: bytes, dA: int, IDA: bytes = b'ALICE123@YH'):
    ENTL = len(IDA) * 8
    PA = scalar_mult(dA, G)
    Za_data = (
        pack('>H', ENTL) + IDA +
        a.to_bytes(32, 'big') +
        b.to_bytes(32, 'big') +
        Gx.to_bytes(32, 'big') +
        Gy.to_bytes(32, 'big') +
        PA[0].to_bytes(32, 'big') +
        PA[1].to_bytes(32, 'big')
    )
    ZA = sm3_hash(Za_data)
    e = int.from_bytes(sm3_hash(ZA + message), 'big')
    while True:
        k = int.from_bytes(os.urandom(32), 'big') % (n - 1) + 1
        x1, y1 = scalar_mult(k, G)
        r = (e + x1) % n
        if r == 0 or r + k == n:
            continue
        s = (inv_mod(1 + dA, n) * (k - r * dA)) % n
        if s == 0:
            continue
        return r, s

def sm2_verify(message: bytes, r: int, s: int, PA: tuple, IDA: bytes = b'ALICE123@YH'):
    if not (1 <= r <= n - 1 and 1 <= s <= n - 1):
        return False
    ENTL = len(IDA) * 8
    Za_data = (
        pack('>H', ENTL) + IDA +
        a.to_bytes(32, 'big') +
        b.to_bytes(32, 'big') +
        Gx.to_bytes(32, 'big') +
        Gy.to_bytes(32, 'big') +
        PA[0].to_bytes(32, 'big') +
        PA[1].to_bytes(32, 'big')
    )
    ZA = sm3_hash(Za_data)
    e = int.from_bytes(sm3_hash(ZA + message), 'big')
    t = (r + s) % n
    if t == 0:
        return False
    x1, _ = point_add(scalar_mult(s, G), scalar_mult(t, PA))
    return (e + x1) % n == r
