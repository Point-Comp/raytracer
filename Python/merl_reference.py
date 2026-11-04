import numpy as np

path = "../Data/merl/gold-metallic-paint.binary"
out_path = "Tests/merl_reference.txt"

SCALE = [1.0/1500, 1.15/1500, 1.66/1500]

with open(path, "rb") as f:
    dims = np.fromfile(f, dtype=np.int32, count=3)
    data = np.fromfile(f, dtype=np.float64)

data = data.reshape(3, 90, 90, 180)


def normalize(v):
    return v / np.linalg.norm(v)


def rotate(v, axis, angle):
    c, s = np.cos(angle), np.sin(angle)
    return v * c + axis * np.dot(axis, v) * (1 - c) + np.cross(axis, v) * s


def lookup(wi, wo):
    if wi[1] <= 0 or wo[1] <= 0:
        return np.zeros(3)

    w_in = normalize(np.array([wi[2], wi[0], wi[1]]))
    w_out = normalize(np.array([wo[2], wo[0], wo[1]]))

    half = normalize(w_in + w_out)
    theta_half = np.arccos(np.clip(half[2], -1, 1))
    phi_half = np.arctan2(half[1], half[0])

    temp = rotate(w_in, np.array([0.0, 0.0, 1.0]), -phi_half)
    diff = rotate(temp, np.array([0.0, 1.0, 0.0]), -theta_half)
    theta_diff = np.arccos(np.clip(diff[2], -1, 1))
    phi_diff = np.arctan2(diff[1], diff[0])

    th = 0 if theta_half <= 0 else int(np.sqrt(theta_half / (np.pi / 2) * 90 * 90))
    th = min(max(th, 0), 89)
    td = min(max(int(theta_diff / (np.pi / 2) * 90), 0), 89)
    if phi_diff < 0:
        phi_diff += np.pi
    pd = min(max(int(phi_diff / np.pi * 180), 0), 179)

    return np.array([max(0.0, data[c, th, td, pd] * SCALE[c]) for c in range(3)])


rng = np.random.default_rng(1234)


def random_upper():
    while True:
        v = rng.normal(size=3)
        if v[1] > 0.05:
            return normalize(v)


pairs = [
    (np.array([0.0, 1.0, 0.0]), np.array([0.0, 1.0, 0.0])),
    (normalize(np.array([0.5, 1.0, 0.0])), normalize(np.array([-0.5, 1.0, 0.0]))),
    (normalize(np.array([0.3, 0.6, 0.7])), normalize(np.array([-0.3, 0.6, -0.7]))),
    (normalize(np.array([0.8, 0.3, 0.1])), normalize(np.array([0.2, 0.9, -0.4]))),
]
while len(pairs) < 12:
    pairs.append((random_upper(), random_upper()))

with open(out_path, "w") as f:
    for wi, wo in pairs:
        value = lookup(wi, wo)
        f.write(" ".join(f"{x:.17g}" for x in [*wi, *wo, *value]) + "\n")

print(f"wrote {len(pairs)} pairs to {out_path}")
