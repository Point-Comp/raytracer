import numpy as np

MERL_SCALE = np.array([1.0/1500, 1.15/1500, 1.66/1500])


def load_merl(path):
    with open(path, "rb") as f:
        dims = np.fromfile(f, dtype=np.int32, count=3)
        data = np.fromfile(f, dtype=np.float64)
    return data.reshape(3, 90, 90, 180)


def normalize(v):
    return v / np.linalg.norm(v, axis=-1, keepdims=True)


def dot(a, b):
    return np.sum(a * b, axis=-1)


# Directions are (N, 3) arrays in the renderer's local frame, normal = +y

def rotate(v, axis, angle):
    c = np.cos(angle)[:, None]
    s = np.sin(angle)[:, None]
    axis = np.broadcast_to(axis, v.shape)
    return v * c + axis * dot(axis, v)[:, None] * (1 - c) + np.cross(axis, v) * s


def merl_lookup(data, wi, wo):
    """Returns (values, valid): scaled RGB per pair, and False where the table has no measurement."""
    w_in = normalize(wi[:, [2, 0, 1]])
    w_out = normalize(wo[:, [2, 0, 1]])

    half = normalize(w_in + w_out)
    theta_half = np.arccos(np.clip(half[:, 2], -1, 1))
    phi_half = np.arctan2(half[:, 1], half[:, 0])

    temp = rotate(w_in, np.array([0.0, 0.0, 1.0]), -phi_half)
    diff = rotate(temp, np.array([0.0, 1.0, 0.0]), -theta_half)
    theta_diff = np.arccos(np.clip(diff[:, 2], -1, 1))
    phi_diff = np.arctan2(diff[:, 1], diff[:, 0])

    th = np.sqrt(np.maximum(theta_half, 0) / (np.pi / 2) * 90 * 90).astype(int)
    th = np.where(theta_half <= 0, 0, np.clip(th, 0, 89))
    td = np.clip((theta_diff / (np.pi / 2) * 90).astype(int), 0, 89)
    phi_diff = np.where(phi_diff < 0, phi_diff + np.pi, phi_diff)
    pd = np.clip((phi_diff / np.pi * 180).astype(int), 0, 179)

    raw = np.stack([data[c, th, td, pd] for c in range(3)], axis=-1)
    above = (wi[:, 1] > 0) & (wo[:, 1] > 0)
    valid = np.all(raw >= 0, axis=-1) & above
    values = np.where(above[:, None], np.maximum(0.0, raw * MERL_SCALE), 0.0)
    return values, valid


def ggx_eval(wi, wo, f0, roughness):
    """Mirrors ggx::eval in C++/ggx.h."""
    alpha = max(1e-3, roughness * roughness)
    a2 = alpha * alpha

    ci = wi[:, 1]
    co = wo[:, 1]
    above = (ci > 0) & (co > 0)
    ci_s = np.where(above, ci, 1.0)
    co_s = np.where(above, co, 1.0)

    h = normalize(wi + wo)
    c2 = h[:, 1] ** 2
    D = a2 / (np.pi * (c2 * (a2 - 1) + 1) ** 2)

    F = f0 + (1 - f0) * (1 - np.maximum(0.0, dot(wi, h)))[:, None] ** 5

    def G1(c):
        return 2 * c / (c + np.sqrt(a2 + (1 - a2) * c * c))

    f = (D * G1(ci_s) * G1(co_s) / (4 * ci_s * co_s))[:, None] * F
    return np.where(above[:, None], f, 0.0)


def random_upper_hemisphere(rng, n):
    y = rng.uniform(0, 1, n)
    phi = rng.uniform(0, 2 * np.pi, n)
    r = np.sqrt(1 - y * y)
    return np.stack([r * np.cos(phi), y, r * np.sin(phi)], axis=-1)


def merl_bin_directions(th, td, pd):
    """Inverse of merl_lookup's index mapping: (wi, wo) at the centre of each table bin, phi_half = 0."""
    theta_half = ((th + 0.5) / 90) ** 2 * (np.pi / 2)
    theta_diff = (td + 0.5) / 90 * (np.pi / 2)
    phi_diff = (pd + 0.5) / 180 * np.pi

    d = np.stack([np.sin(theta_diff) * np.cos(phi_diff),
                  np.sin(theta_diff) * np.sin(phi_diff),
                  np.cos(theta_diff)], axis=-1)
    w_in = rotate(d, np.array([0.0, 1.0, 0.0]), theta_half)
    half = np.stack([np.sin(theta_half), np.zeros_like(theta_half), np.cos(theta_half)], axis=-1)
    w_out = 2 * dot(w_in, half)[:, None] * half - w_in

    return w_in[:, [1, 2, 0]], w_out[:, [1, 2, 0]]
