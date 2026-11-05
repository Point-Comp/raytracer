import argparse
import json
import os

import numpy as np
from scipy.optimize import minimize

from brdf import load_merl, merl_lookup, ggx_eval, merl_bin_directions

parser = argparse.ArgumentParser()
parser.add_argument("name", nargs="?", default="gold-metallic-paint")
parser.add_argument("--loss", choices=["log", "log1p"], default="log",
                    help="log1p is the original loss, kept to reproduce the old fit; saved as <name>-log1p")
args = parser.parse_args()
name = args.name
key = name if args.loss == "log" else f"{name}-log1p"
merl_path = f"../Data/merl/{name}.binary"
params_path = "../Data/ggx_params.json"

data = load_merl(merl_path)

rng = np.random.default_rng(0)
n_pairs = 200000

# Sample table bins rather than uniform directions: the table packs its theta_half
# bins densely near the highlight, so uniform directions would almost never hit it
th = rng.integers(0, 90, n_pairs)
td = rng.integers(0, 90, n_pairs)
pd = rng.integers(0, 180, n_pairs)
wi, wo = merl_bin_directions(th, td, pd)

target, valid = merl_lookup(data, wi, wo)

# MERL's measurements are unreliable near grazing, so leave out directions beyond 80 degrees
max_angle = np.cos(np.radians(80))
valid &= (wi[:, 1] > max_angle) & (wo[:, 1] > max_angle)
wi, wo, target = wi[valid], wo[valid], target[valid]
# True log space: log1p(x) is nearly linear for x < 1, which is most of the table,
# so it barely penalised a tail many times too high. eps sits below the table's lowest values.
eps = 1e-4
if args.loss == "log":
    transform = lambda x: np.log(x + eps)
else:
    transform = np.log1p
log_target = transform(target)
print(f"{name}: {valid.sum()} valid pairs of {n_pairs}")


def loss(x):
    roughness, f0 = x[0], x[1:]
    return np.mean((transform(ggx_eval(wi, wo, f0, roughness)) - log_target) ** 2)


# Several starting roughnesses, since the loss can have local minima
best = None
for r0 in [0.1, 0.3, 0.6]:
    result = minimize(loss, x0=[r0, 0.5, 0.5, 0.5], method="L-BFGS-B",
                      bounds=[(0.01, 1.0), (0.0, 1.0), (0.0, 1.0), (0.0, 1.0)])
    print(f"  start roughness {r0}: loss {result.fun:.6g}, roughness {result.x[0]:.4f}, f0 {np.round(result.x[1:], 4)}")
    if best is None or result.fun < best.fun:
        best = result

roughness, f0 = float(best.x[0]), [float(c) for c in best.x[1:]]
print(f"best: roughness {roughness:.4f}, f0 {np.round(f0, 4)}, loss {best.fun:.6g}")

params = {}
if os.path.exists(params_path):
    with open(params_path) as f:
        params = json.load(f)
params[key] = {"roughness": roughness, "f0": f0}
with open(params_path, "w") as f:
    json.dump(params, f, indent=2)
print(f"wrote {key} to {params_path}")
