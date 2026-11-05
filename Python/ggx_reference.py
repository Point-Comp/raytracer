import json

import numpy as np

from brdf import ggx_eval, normalize, random_upper_hemisphere

params_path = "../Data/ggx_params.json"
out_path = "Tests/ggx_reference.txt"

with open(params_path) as f:
    fitted = json.load(f)

# "fixed" lines are built in C++ from the numbers on the line; named lines through the JSON constructor
param_sets = [("fixed", 0.05, [1.0, 0.71, 0.29]),
              ("fixed", 0.3, [0.9, 0.6, 0.3]),
              ("fixed", 1.0, [0.04, 0.04, 0.04])]
for name, p in fitted.items():
    param_sets.append((name, p["roughness"], p["f0"]))

rng = np.random.default_rng(4321)
wi = np.concatenate([
    normalize(np.array([[0.0, 1.0, 0.0], [0.5, 1.0, 0.0], [0.3, 0.6, 0.7], [0.8, 0.3, 0.1]])),
    random_upper_hemisphere(rng, 8)])
wo = np.concatenate([
    normalize(np.array([[0.0, 1.0, 0.0], [-0.5, 1.0, 0.0], [-0.3, 0.6, -0.7], [0.2, 0.9, -0.4]])),
    random_upper_hemisphere(rng, 8)])

with open(out_path, "w") as f:
    for name, roughness, f0 in param_sets:
        values = ggx_eval(wi, wo, np.array(f0), roughness)
        for i in range(len(wi)):
            numbers = [roughness, *f0, *wi[i], *wo[i], *values[i]]
            f.write(name + " " + " ".join(f"{x:.17g}" for x in numbers) + "\n")

print(f"wrote {len(param_sets) * len(wi)} lines to {out_path}")
