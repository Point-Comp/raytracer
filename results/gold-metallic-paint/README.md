# gold-metallic-paint: GGX fit study

One MERL material, two GGX fits that differ only in the loss, and the question:
does the fit that is better in BRDF space also produce the better image?

## Fits

Both fits use the same data: 200,000 MERL table bins sampled uniformly by index
(`th`, `td`, `pd`), with directions beyond 80° from the normal excluded, leaving
124,839 pairs. Each is optimised with L-BFGS-B from three starting roughnesses
(0.1, 0.3, 0.6); all three starts converge to the same answer in both cases.

| Key in `ggx_params.json`    | Loss                          | Roughness | F0 (R, G, B)          |
|-----------------------------|-------------------------------|-----------|-----------------------|
| `gold-metallic-paint-log1p` | MSE of `log(1 + x)`           | 0.662     | 0.582, 0.391, 0.145   |
| `gold-metallic-paint`       | MSE of `log(x + 1e-4)`        | 0.487     | 0.224, 0.143, 0.045   |

**Why the log1p fit is too rough.** `log(1 + x) ≈ x` for `x < 1`, and the
table's 99th percentile (red channel) is 1.77, so log1p compresses almost
nothing: the fit is effectively a linear MSE. That barely penalises the tail,
where values are small in absolute terms, so the fit trades a highlight that is
half as bright as measured for a tail up to 8× too bright (`theta_half_sweep.png`).

**Independent check on the roughness.** Fitting only the highlight's shape,
with GGX given a free brightness per 10° band of theta_d, gives roughness 0.482.
The log-loss fit (0.487) agrees, so its roughness is set by the highlight's
shape and not by brightness trade-offs across theta_d.

**Diffuse term.** Adding a Lambertian term (7 parameters) changes nothing: it
fits to exactly zero under either loss. This material does not need one.

### eps sensitivity

The log loss is `log(x + eps)`. eps sets the value below which errors stop
mattering, so it must sit below the data. The smallest MERL values for this
material are around 5e-3.

| eps   | Roughness | F0 (R, G, B)        |
|-------|-----------|---------------------|
| 1e-6  | 0.486     | 0.223, 0.142, 0.045 |
| 1e-5  | 0.486     | 0.223, 0.142, 0.045 |
| 1e-4  | 0.487     | 0.224, 0.143, 0.045 |
| 1e-3  | 0.494     | 0.228, 0.148, 0.049 |
| 1e-2  | 0.518     | 0.253, 0.172, 0.058 |

Stable from 1e-6 to 1e-3. It only moves at 1e-2, where eps crosses into the
data's range and the loss starts treating the dim tail as linear. 1e-4 is used.

## Image comparison

Scene: the Utah teapot on a Lambertian ground with a mirror sphere, lit by
`kloppenheim_06_2k.hdr`. 400×225 pixels.

- Reference: MERL material, 4096 samples per pixel, mixture sampling.
- Tests: each GGX fit at 1024 samples per pixel.
- Noise floor: MERL itself at 1024 samples per pixel. This is the error a
  perfect BRDF match would score at the test sample count, so differences
  below it are noise.

"Crop" is the teapot body (rows 30–80%, columns 28–70% of the image). The full
image includes ground and sky that are near-identical across renders, which
dilutes the averages.

| Test                        | LDR-FLIP | crop  | HDR-FLIP | crop  | PSNR (dB) | crop  |
|-----------------------------|----------|-------|----------|-------|-----------|-------|
| Noise floor (MERL, 1024)    | 0.018    | 0.018 | 0.020    | 0.028 | 42.7      | 43.8  |
| GGX r=0.49 (log loss)       | 0.051    | 0.112 | 0.084    | 0.218 | 34.9      | 33.6  |
| GGX r=0.66 (log1p loss)     | 0.137    | 0.369 | 0.186    | 0.528 | 24.4      | 19.6  |

**Result.** The fit that is better in BRDF space also produces the better
image, on every metric and by a wide margin: teapot-crop LDR-FLIP 0.112 vs
0.369, PSNR 33.6 vs 19.6 dB. Both are well above the noise floor, so both
differences are real.

**Where the better fit still fails.** In `flip_maps.png` the r=0.49 error is
concentrated on the silhouette and rim, at grazing angles. This matches the
BRDF-space residual: GGX's highlight brightens too fast as theta_d grows (scaled
to match MERL at theta_d = 5°, it is 2.9× too bright at 70°), and its tail turns
upward beyond theta_half ≈ 60° where MERL stays flat. That residual is a
limitation of single-lobe GGX on this material, not of the fitting.

## Fit quality and sampling efficiency

The MERL material samples from a mixture of 80% GGX (using the fitted
roughness) and 20% cosine. Better-fitted GGX parameters are also a better
proposal distribution: the fraction of samples that land above the surface at
normal incidence rose from 0.87 (r=0.66) to 0.96 (r=0.49), and from 0.66 to
0.73 at 80°. Fitting quality and sampling efficiency are coupled.

## Caveats

- One material, one scene, one camera. The ordering of the two fits is clear
  here; whether perceptual and BRDF-space rankings agree across materials is
  the open question for the full study.
- The renderer seeds its random generator identically every run, so the noise
  floor is MERL at 1024 vs MERL at 4096 samples per pixel, not two independent
  runs at the same count.
- eps was only checked on this material. A dark material (values near 1e-3)
  stresses it most, since that is where `log(x + eps)` turns linear; a sharp
  metal stresses the bin sampling instead.

## Reproducing

From `C++/`:

```
python3 ../Python/fit_ggx.py gold-metallic-paint
python3 ../Python/fit_ggx.py gold-metallic-paint --loss log1p

./main merl 4096 x 400                          > ref/img.ppm    # out.pfm is written alongside
./main merl 1024 x 400                          > floor/img.ppm
./main ggx  1024 gold-metallic-paint 400        > log/img.ppm
./main ggx  1024 gold-metallic-paint-log1p 400  > log1p/img.ppm

python3 ../Python/compare_renders.py ref "noise floor=floor" "GGX log=log" "GGX log1p=log1p" --maps .
```

Each render writes `out.pfm` into the folder it runs from, so run each from its
own folder (with `utah_teapot.obj` and `../Data` reachable) and keep `img.ppm`
beside its `out.pfm`. Render times on this machine: about 14 min for the
reference and 3–4 min for each 1024-sample render.
