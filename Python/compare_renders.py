import argparse
import os

import numpy as np
import flip_evaluator as flip
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt


def load_ppm(path):
    """The renderer's ASCII P3 output, as floats in [0, 1] (display-referred, gamma applied)."""
    with open(path) as f:
        tokens = f.read().split()
    assert tokens[0] == "P3", f"{path} is not a P3 PPM"
    w, h, maxval = int(tokens[1]), int(tokens[2]), int(tokens[3])
    return np.array(tokens[4:4 + w * h * 3], dtype=np.float64).reshape(h, w, 3) / maxval


def load_pfm(path):
    """The renderer's linear output, flipped so row 0 is the top like the PPM."""
    with open(path, "rb") as f:
        f.readline()
        w, h = map(int, f.readline().split())
        f.readline()
        return np.fromfile(f, dtype=np.float32).reshape(h, w, 3)[::-1].copy()


def psnr(ref, test):
    mse = np.mean((ref - test) ** 2)
    return 10 * np.log10(1.0 / mse)


parser = argparse.ArgumentParser(description="FLIP and PSNR of test renders against a reference render")
parser.add_argument("reference", help="run folder containing img.ppm and out.pfm")
parser.add_argument("tests", nargs="+", help="label=run_folder")
parser.add_argument("--crop", default="0.30,0.80,0.28,0.70",
                    help="region of interest as fractions top,bottom,left,right (default: the teapot)")
parser.add_argument("--maps", help="folder to write FLIP error maps to")
args = parser.parse_args()

top, bottom, left, right = map(float, args.crop.split(","))

ref_ldr = load_ppm(os.path.join(args.reference, "img.ppm"))
ref_hdr = load_pfm(os.path.join(args.reference, "out.pfm"))
h, w = ref_ldr.shape[:2]
roi = (slice(int(top * h), int(bottom * h)), slice(int(left * w), int(right * w)))

print(f"reference: {args.reference} ({w}x{h}), crop rows {roi[0].start}-{roi[0].stop}, cols {roi[1].start}-{roi[1].stop}")
print(f"{'test':<28} {'LDR-FLIP':>9} {'crop':>7} {'HDR-FLIP':>9} {'crop':>7} {'PSNR dB':>8} {'crop':>7}")

maps = []
for item in args.tests:
    label, folder = item.rsplit("=", 1)
    test_ldr = load_ppm(os.path.join(folder, "img.ppm"))
    test_hdr = load_pfm(os.path.join(folder, "out.pfm"))

    ldr_map, ldr_mean, _ = flip.evaluate(ref_ldr.astype(np.float32), test_ldr.astype(np.float32), "LDR", applyMagma=False)
    hdr_map, hdr_mean, _ = flip.evaluate(ref_hdr, test_hdr, "HDR", applyMagma=False)
    ldr_map, hdr_map = np.squeeze(ldr_map), np.squeeze(hdr_map)

    print(f"{label:<28} {ldr_mean:9.4f} {ldr_map[roi].mean():7.4f} {hdr_mean:9.4f} {hdr_map[roi].mean():7.4f} "
          f"{psnr(ref_ldr, test_ldr):8.2f} {psnr(ref_ldr[roi], test_ldr[roi]):7.2f}")
    maps.append((label, test_ldr, ldr_map))

if args.maps:
    os.makedirs(args.maps, exist_ok=True)
    fig, axes = plt.subplots(2, len(maps) + 1, figsize=(4.2 * (len(maps) + 1), 5.2))
    axes[0, 0].imshow(ref_ldr); axes[0, 0].set_title("reference")
    axes[1, 0].axis("off")
    for k, (label, img, emap) in enumerate(maps, start=1):
        axes[0, k].imshow(img); axes[0, k].set_title(label)
        im = axes[1, k].imshow(emap, cmap="magma", vmin=0, vmax=1); axes[1, k].set_title(f"LDR-FLIP {label}")
    for ax in axes.flat:
        ax.set_xticks([]); ax.set_yticks([])
    fig.colorbar(im, ax=axes[1, 1:].tolist(), shrink=0.8)
    out = os.path.join(args.maps, "flip_maps.png")
    fig.savefig(out, dpi=100, bbox_inches="tight")
    print(f"wrote {out}")
