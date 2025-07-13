import random
from ecdsa import SECP256k1
from phe import paillier
import hashlib
# use SECP256k1 
curve = SECP256k1
G = curve.generator
order = G.order()

# Define the hash function using SHA-256
def H(u):
    h = hashlib.sha256(u.encode()).hexdigest()
    val = int(h, 16) % order
    return val * G

# Setup:
V = ['alice', 'bob', 'carol']#set V
W = [('alice', 30), ('dave', 20), ('carol', 50)]#set W
k1 = random.randint(1, order - 1)#p1 key
k2 = random.randint(1, order - 1)#p2 key
pk, sk = paillier.generate_paillier_keypair()#generate p2  keypair
# Round 1 
H_vk1_list = [k1 * H(v) for v in V]
random.shuffle(H_vk1_list)#shuffle
# send it to p2
#Round 2
Z = [k2 * pt for pt in H_vk1_list]#compute Z
random.shuffle(Z)#shuffle
Hwk2_AEnc_t = []
for w, t in W:
    hw = H(w)
    hwk2 = k2 * hw
    enc_t = pk.encrypt(t)
    Hwk2_AEnc_t.append((hwk2, enc_t))
random.shuffle(Hwk2_AEnc_t)#shuffle

#Round 3
intersection_ciphertexts = []
for hwk2, enc_t in Hwk2_AEnc_t:
    hwk1k2 = k1 * hwk2
    if hwk1k2 in Z:
        intersection_ciphertexts.append(enc_t)
if intersection_ciphertexts:
    final_cipher = intersection_ciphertexts[0]
    for ct in intersection_ciphertexts[1:]:
        final_cipher += ct
else:
    final_cipher = pk.encrypt(0)
#output
intersection_sum = sk.decrypt(final_cipher)
print(f"交集大小: {len(intersection_ciphertexts)}")
print(f"交集元素 ti 之和: {intersection_sum}")