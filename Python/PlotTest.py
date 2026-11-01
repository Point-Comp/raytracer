import numpy as np

path = "data/merl/gold-metallic-paint.binary"

#its kinda weird each binary file has 4.3mil numbers which represent stuff which can be used to be angles and later be useful in the c++ renderer
with open(path, "rb") as f:
    dims = np.fromfile(f, dtype=np.int32, count=3) #number of steps along the angles, The angles are what those indices represent.
    data = np.fromfile(f, dtype=np.float64)

data = data.reshape(3, 90, 90, 180)
print(data.shape)
print(data.min(), data.max())

print((data < 0).mean())        # fraction of invalid entries
print(data[data >= 0].mean())   # average of valid ones


import matplotlib.pyplot as plt

SCALE = [1.0/1500, 1.15/1500, 1.66/1500]   # red, green, blue

# sweep theta_half, with theta_diff and phi_diff fixed at 0
red = data[0, :, 0, 0] * SCALE[0]

plt.plot(range(90), red)
plt.yscale("log")
plt.xlabel("theta_half index (0 = mirror direction)")
plt.ylabel("reflectance, red channel")
plt.title("gold-metallic-paint")
plt.show()

plt.savefig("plot.png", dpi=150)
print("saved")