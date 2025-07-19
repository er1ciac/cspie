import hashlib
import secrets
from typing import Tuple

P = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
N = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
GX = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
GY = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
G = (GX, GY)
INF = (0, 0)

def point_add(p1: Tuple[int, int], p2: Tuple[int, int]) -> Tuple[int, int]:
    if p1 == INF:
        return p2
    if p2 == INF:
        return p1
    x1, y1 = p1
    x2, y2 = p2
    if x1 == x2:
        if y1 == y2:
            s = (3 * x1 * x1) * pow(2 * y1, -1, P) % P
        else:
            return INF
    else:
        s = (y2 - y1) * pow(x2 - x1, -1, P) % P
    x3 = (s * s - x1 - x2) % P
    y3 = (s * (x1 - x3) - y1) % P
    return (x3, y3)

def scalar_mult(k: int, point: Tuple[int, int]) -> Tuple[int, int]:
    if k == 0:
        return INF
    if k == 1:
        return point
    result = INF
    addend = point
    while k:
        if k & 1:
            result = point_add(result, addend)
        addend = point_add(addend, addend)
        k >>= 1
    return result

def generate_keypair() -> Tuple[int, Tuple[int, int]]:
    private_key = secrets.randbelow(N - 1) + 1
    public_key = scalar_mult(private_key, G)
    return private_key, public_key

def sign(message: bytes, private_key: int) -> Tuple[int, int]:
    z = int.from_bytes(hashlib.sha256(message).digest(), 'big') % N
    while True:
        k = secrets.randbelow(N - 1) + 1
        point = scalar_mult(k, G)
        r = point[0] % N
        if r == 0:
            continue
        k_inv = pow(k, -1, N)
        s = (k_inv * (z + r * private_key)) % N
        if s == 0:
            continue
        
        return r, s

def verify(message: bytes, signature: Tuple[int, int], public_key: Tuple[int, int]) -> bool:
    r, s = signature
    if not (1 <= r < N and 1 <= s < N):
        return False
    z = int.from_bytes(hashlib.sha256(message).digest(), 'big') % N
    s_inv = pow(s, -1, N)
    u1 = (z * s_inv) % N
    u2 = (r * s_inv) % N
    point = point_add(scalar_mult(u1, G), scalar_mult(u2, public_key))
    return (point[0] % N) == r

def verify_signature_only(signature: Tuple[int, int], public_key: Tuple[int, int], z: int = 0) -> bool:
    r, s = signature
    if not (1 <= r < N and 1 <= s < N):
        return False
    s_inv = pow(s, -1, N)
    u1 = (z * s_inv) % N
    u2 = (r * s_inv) % N
    point = point_add(scalar_mult(u1, G), scalar_mult(u2, public_key))
    return (point[0] % N) == r

def direct_empty_digest_attack(public_key: Tuple[int, int]) -> Tuple[int, int]:
    while True:
        k = secrets.randbelow(N - 1) + 1
        point_p = scalar_mult(k, public_key)
        r = point_p[0] % N
        if r == 0:
            continue
        k_inv = pow(k, -1, N)
        s = (r * k_inv) % N
        if s == 0:
            continue
        return r, s

private_key, public_key = generate_keypair()
original_message = b"This is a legitimate message"
legitimate_signature = sign(original_message, private_key)
is_valid = verify(original_message, legitimate_signature, public_key)
forged_r, forged_s = direct_empty_digest_attack(public_key)
attack_valid = verify_signature_only((forged_r, forged_s), public_key, z=0)
print(f"Original signature valid: {is_valid}")
print(f"Forged signature valid : {attack_valid}")