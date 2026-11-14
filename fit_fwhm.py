import numpy as np
import matplotlib.pyplot as plt
import becquerel as bq

from scipy.optimize import curve_fit


# ============================================================
# USER SETTINGS
# ============================================================

FOREGROUND_FILE = "th232_fwhm_cal.spe"
BACKGROUND_FILE = "fwhm_cal_bg.spe"

# Known useful peaks in a natural Th-232 decay-chain spectrum.
#
# Only enable peaks that are clearly resolved in YOUR spectrum.
#
# energy_keV : (fit_low, fit_high)
PEAKS = {
    238.63:  (200, 280),     # Pb-212
    338.32:  (300, 380),     # Ac-228
    583.19:  (530, 640),     # Tl-208
    727.33:  (680, 775),     # Bi-212
    911.20:  (860, 945),     # Ac-228
    968.97:  (945, 1010),    # Ac-228
    1588.19: (1510, 1660),   # Ac-228
    2614.51: (2500, 2725),   # Tl-208
}


# ============================================================
# LOAD FOREGROUND + BACKGROUND
# ============================================================

fg = bq.Spectrum.from_file(FOREGROUND_FILE)
bg = bq.Spectrum.from_file(BACKGROUND_FILE)

print("\nForeground:")
print(fg)

print("\nBackground:")
print(bg)

if fg.livetime is None or bg.livetime is None:
    raise RuntimeError(
        "Both SPE files must contain valid live times."
    )

if len(fg) != len(bg):
    raise RuntimeError(
        "Foreground and background have different numbers of channels."
    )

if not fg.is_calibrated or not bg.is_calibrated:
    raise RuntimeError(
        "Both spectra must have energy calibration."
    )

if not np.allclose(
    fg.bin_edges_kev,
    bg.bin_edges_kev,
    rtol=1e-7,
    atol=1e-6
):
    raise RuntimeError(
        "Foreground and background energy calibrations/binning do not match."
    )


# ============================================================
# BACKGROUND SUBTRACTION
# ============================================================

foreground_counts = fg.counts_vals
background_counts = bg.counts_vals

alpha = fg.livetime / bg.livetime

print("\nBackground subtraction")
print("----------------------")
print("Foreground live time:", fg.livetime)
print("Background live time:", bg.livetime)
print("Background scale factor alpha:", alpha)

net_counts = (
    foreground_counts
    - alpha * background_counts
)

# Independent Poisson foreground/background statistics:
#
# Var(F - alpha B)
#     = Var(F) + alpha^2 Var(B)
#     = F + alpha^2 B
#
net_uncs = np.sqrt(
    foreground_counts
    + alpha**2 * background_counts
)

net = bq.Spectrum(
    counts=net_counts,
    uncs=net_uncs,
    bin_edges_kev=fg.bin_edges_kev,
    livetime=fg.livetime
)

x = net.bin_centers_kev
y = net.counts_vals
y_unc = net.counts_uncs
dx = net.bin_widths_kev


# ============================================================
# FIT INDIVIDUAL PEAKS
# ============================================================

fit_energies = []
fit_centroids = []
fit_fwhm = []
fit_sigma = []


for nominal_energy, roi in PEAKS.items():

    print()
    print("=" * 60)
    print(f"Fitting nominal peak: {nominal_energy:.2f} keV")
    print(f"ROI: {roi[0]:.1f} - {roi[1]:.1f} keV")

    model = (
        bq.fitting.GaussModel(prefix="gauss0_")
        + bq.fitting.LineModel(prefix="linear_")
    )

    fitter = bq.Fitter(
        model,
        x=x,
        y=y,
        y_unc=y_unc,
        dx=dx,
        roi=roi
    )

    try:
        fitter.fit(
            backend="lmfit"
        )

        centroid = fitter.param_val(
            "gauss0_mu"
        )

        sigma = fitter.param_val(
            "gauss0_sigma"
        )

        area = fitter.param_val(
            "gauss0_amp"
        )

        fwhm = 2.35482 * sigma

        print("Centroid:", centroid)
        print("Sigma:", sigma)
        print("FWHM:", fwhm)
        print(
            "FWHM (%):",
            100 * fwhm / centroid
        )
        print("Peak area:", area)

        # Basic sanity checks
        if not np.isfinite(centroid):
            print("Skipping: invalid centroid")
            continue

        if not np.isfinite(sigma):
            print("Skipping: invalid sigma")
            continue

        if sigma <= 0:
            print("Skipping: non-positive sigma")
            continue

        if not (
            roi[0]
            <= centroid
            <= roi[1]
        ):
            print(
                "Skipping: centroid outside ROI"
            )
            continue

        centroid_tolerance = 0.015  # 1.5%
        min_area = 300
        min_resolution = 0.02      # 2%
        max_resolution = 0.20      # 20%

        relative_centroid_error = abs(centroid - nominal_energy) / nominal_energy
        relative_fwhm = fwhm / centroid

        if relative_centroid_error > centroid_tolerance:
            print("Skipping: centroid too far from expected energy")
            continue

        if area < min_area:
            print("Skipping: peak area too small")
            continue

        if relative_fwhm < min_resolution:
            print("Skipping: implausibly narrow peak")
            continue

        if relative_fwhm > max_resolution:
            print("Skipping: implausibly broad peak")
            continue

        fit_energies.append(
            nominal_energy
        )

        fit_centroids.append(
            centroid
        )

        fit_sigma.append(
            sigma
        )

        fit_fwhm.append(
            fwhm
        )

        fit_centroids = np.append(
        fit_centroids,
        1460.8
        )

        fit_fwhm = np.append(
            fit_fwhm,
            82.39
        )

        fit_centroids = np.append(fit_centroids, 894.51)
        fit_fwhm = np.append(fit_fwhm, 62.88)
        fit_centroids = np.append(fit_centroids, 951.36)
        fit_fwhm = np.append(fit_fwhm, 62.82)
        fit_centroids = np.append(fit_centroids, 2584.32)
        fit_fwhm = np.append(fit_fwhm, 108.61)

    except Exception as exc:

        print(
            f"Fit failed: {exc}"
        )


fit_energies = np.asarray(
    fit_energies
)

fit_centroids = np.asarray(
    fit_centroids
)

fit_sigma = np.asarray(
    fit_sigma
)

fit_fwhm = np.asarray(
    fit_fwhm
)


# ============================================================
# PRINT PEAK TABLE
# ============================================================

print()
print("=" * 70)
print("RESOLUTION DATA")
print("=" * 70)

print(
    f"{'Nominal':>12}"
    f"{'Centroid':>12}"
    f"{'Sigma':>12}"
    f"{'FWHM':>12}"
    f"{'FWHM %':>12}"
)

for E0, mu, sigma, fwhm in zip(
    fit_energies,
    fit_centroids,
    fit_sigma,
    fit_fwhm
):

    print(
        f"{E0:12.3f}"
        f"{mu:12.3f}"
        f"{sigma:12.3f}"
        f"{fwhm:12.3f}"
        f"{100*fwhm/mu:12.3f}"
    )


# ============================================================
# FIT DETECTOR RESOLUTION MODEL
# ============================================================

def fwhm_squared_model(
    E,
    A,
    B,
    C
):
    return (
        A
        + B * E
        + C * E**2
    )


if len(fit_energies) < 3:
    raise RuntimeError(
        "Need at least three usable peaks "
        "to fit A + B E + C E^2."
    )


# Fit FWHM^2, not FWHM
y_resolution = fit_fwhm**2


# Physical constraint:
#
# A >= 0
# B >= 0
# C >= 0
#
# This guarantees the model remains positive
# throughout the positive-energy range.
#
popt, pcov = curve_fit(
    fwhm_squared_model,
    fit_centroids,
    y_resolution,
    bounds=(
        [0.0, 0.0, 0.0],
        [np.inf, np.inf, np.inf]
    )
)

A, B, C = popt

print()
print("=" * 70)
print("RESOLUTION MODEL")
print("=" * 70)

print("A =", A)
print("B =", B)
print("C =", C)

print()
print(
    "FWHM(E) = sqrt("
    f"{A:.8g} "
    f"+ {B:.8g}*E "
    f"+ {C:.8g}*E^2)"
)


# Parameter uncertainties, if covariance is valid
if (
    pcov is not None
    and np.all(np.isfinite(pcov))
):

    perr = np.sqrt(
        np.diag(pcov)
    )

    print()
    print(
        "Coefficient uncertainties:"
    )

    print(
        "A =",
        A,
        "+/-",
        perr[0]
    )

    print(
        "B =",
        B,
        "+/-",
        perr[1]
    )

    print(
        "C =",
        C,
        "+/-",
        perr[2]
    )


# ============================================================
# PLOT RESOLUTION CURVE
# ============================================================

energy_grid = np.linspace(
    1,
    3000,
    2000
)

fwhm_fit = np.sqrt(
    fwhm_squared_model(
        energy_grid,
        A,
        B,
        C
    )
)

plt.figure(
    figsize=(9, 6)
)

plt.scatter(
    fit_centroids,
    fit_fwhm,
    label="Measured peak FWHM"
)

plt.plot(
    energy_grid,
    fwhm_fit,
    label="Resolution fit"
)

plt.xlabel(
    "Energy (keV)"
)

plt.ylabel(
    "FWHM (keV)"
)

plt.title(
    "Detector Energy Resolution"
)

plt.grid(
    alpha=0.3
)

plt.legend()

plt.tight_layout()
plt.show()


# ============================================================
# PLOT RELATIVE RESOLUTION
# ============================================================

relative_resolution = (
    100
    * fwhm_fit
    / energy_grid
)

plt.figure(
    figsize=(9, 6)
)

plt.scatter(
    fit_centroids,
    100 * fit_fwhm / fit_centroids,
    label="Measured peaks"
)

plt.plot(
    energy_grid,
    relative_resolution,
    label="Resolution fit"
)

plt.xlabel(
    "Energy (keV)"
)

plt.ylabel(
    "FWHM / Energy (%)"
)

plt.title(
    "Relative Detector Resolution"
)

plt.grid(
    alpha=0.3
)

plt.legend()

plt.tight_layout()
plt.show()


# ============================================================
# PRINT VALUES USEFUL FOR YOUR GEANT4 ANALYSIS
# ============================================================

print()
print("=" * 70)
print("MODEL CHECK")
print("=" * 70)

for E in [
    50,
    100,
    150,
    200,
    250,
    500,
    1000,
    1460.8,
    2000,
    2614.5,
    2800
]:

    fwhm_E = np.sqrt(
        fwhm_squared_model(
            E,
            A,
            B,
            C
        )
    )

    print(
        f"{E:8.1f} keV : "
        f"FWHM = {fwhm_E:8.3f} keV "
        f"({100*fwhm_E/E:6.3f} %)"
    )