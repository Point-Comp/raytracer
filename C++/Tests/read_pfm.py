import numpy as np

with open("out.pfm", "rb") as f:
    magic, dimensions, scale = f.readline(), f.readline(), f.readline()
    w, h = map(int, dimensions.split())

    img = np.fromfile(f, dtype=np.float32).reshape((h, w, 3))

    print(img.shape)
    print(img.max())
    print(img[0].mean(), img[-1].mean())