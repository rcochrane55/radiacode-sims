import uproot
import awkward as ak
import numpy as np
import matplotlib.pyplot as plt
import warnings
import lmfit
import becquerel as bq
from scipy.interpolate import RegularGridInterpolator
from scipy.special import ndtr
import pandas as pd

file = uproot.open("rootOutput.root")
tree = file["t"]

raw = tree["RawEdep"].array(library="np") * 1000
stepE = tree["fStepEnergy"].array()
stepX = tree["fStepX"].array()
stepY = tree["fStepY"].array()
stepZ = tree["fStepZ"].array()

""" print("X range:", ak.min(ak.flatten(stepX)), "to", ak.max(ak.flatten(stepX)))
print("Y range:", ak.min(ak.flatten(stepY)), "to", ak.max(ak.flatten(stepY)))
print("Z range:", ak.min(ak.flatten(stepZ)), "to", ak.max(ak.flatten(stepZ))) """

optical_map = pd.read_csv("optical_response_map.csv")

""" print("\nOptical response statistics:")
print("Minimum:", optical_map["efficiency"].min())
print("Maximum:", optical_map["efficiency"].max())
print("Mean:", optical_map["efficiency"].mean())
print("Standard deviation:", optical_map["efficiency"].std()) """

x_values = np.sort(optical_map["x_mm"].unique())
y_values = np.sort(optical_map["y_mm"].unique())
z_values = np.sort(optical_map["z_mm"].unique())

""" print("\nOptical map ranges:")
print("X:", x_values.min(), "to", x_values.max())
print("Y:", y_values.min(), "to", y_values.max())
print("Z:", z_values.min(), "to", z_values.max()) """

response_grid = (
    optical_map.sort_values(["x_mm", "y_mm", "z_mm"])["efficiency"]
    .to_numpy()
    .reshape(len(x_values), len(y_values), len(z_values))
)

interpolator = RegularGridInterpolator(
    (x_values, y_values, z_values),
    response_grid,
    bounds_error=False,
    fill_value=0
)

eta_center = interpolator([[0.0, 4.999, 0.5]])[0]
#print("Interpolated center efficiency:", eta_center)

flat_x = ak.to_numpy(ak.flatten(stepX))
flat_y = ak.to_numpy(ak.flatten(stepY))
flat_z = ak.to_numpy(ak.flatten(stepZ)) - 30.0

""" print("\nStep coordinate ranges:")
print("X:", flat_x.min(), "to", flat_x.max())
print("Y:", flat_y.min(), "to", flat_y.max())
print("Z:", flat_z.min(), "to", flat_z.max()) """

outside_x = (flat_x < x_values.min()) | (flat_x > x_values.max())
outside_y = (flat_y < y_values.min()) | (flat_y > y_values.max())
outside_z = (flat_z < z_values.min()) | (flat_z > z_values.max())

""" print("\nOutside by axis:")
print("X:", 100 * np.mean(outside_x), "%")
print("Y:", 100 * np.mean(outside_y), "%")
print("Z:", 100 * np.mean(outside_z), "%")
 """
positions = np.column_stack((flat_x, flat_y, flat_z))
flat_response = interpolator(positions)

outside = (
    (flat_x < x_values.min()) | (flat_x > x_values.max()) |
    (flat_y < y_values.min()) | (flat_y > y_values.max()) |
    (flat_z < z_values.min()) | (flat_z > z_values.max())
)

flat_stepE = ak.to_numpy(ak.flatten(stepE))

""" print("\nEnergy outside optical map:")
print("Outside energy:", np.sum(flat_stepE[outside]))
print("Total deposited energy:", np.sum(flat_stepE))
print("Percentage energy outside:", 100 * np.sum(flat_stepE[outside]) / np.sum(flat_stepE))
print("Steps outside optical map:", np.sum(outside))
print("Total steps:", len(outside))
print("Percentage outside:", 100 * np.mean(outside), "%") """

weighted_stepE = flat_stepE * flat_response
event_lengths = ak.num(stepE, axis=1)
weighted_stepE = ak.unflatten(weighted_stepE, event_lengths)
weighted_energy = ak.sum(weighted_stepE, axis=1)

raw_event_energy = ak.to_numpy(ak.sum(stepE, axis=1))
weighted_energy_np = ak.to_numpy(weighted_energy)

full_energy = (raw_event_energy > 1.40) & (raw_event_energy < 1.52)

print("\nFull-energy events:")
print("Number:", np.sum(full_energy))

if np.sum(full_energy) > 0:
    optical_factor = weighted_energy_np[full_energy] / raw_event_energy[full_energy]
    reconstructed_full = weighted_energy_np[full_energy] / eta_center

    print("Mean raw energy:", np.mean(raw_event_energy[full_energy]) * 1000, "keV")
    print("Mean weighted energy:", np.mean(weighted_energy_np[full_energy]) * 1000, "keV")
    print("Mean reconstructed energy:", np.mean(reconstructed_full) * 1000, "keV")
    print("Mean optical weighting factor:", np.mean(optical_factor))
    print("Minimum optical weighting factor:", np.min(optical_factor))
    print("Maximum optical weighting factor:", np.max(optical_factor))

reconstructed_energy = ak.to_numpy(weighted_energy / eta_center) * 1000
reconstructed_mean = np.mean(reconstructed_full) * 1000

hist, edges = np.histogram(reconstructed_energy[reconstructed_energy > 0], bins=1024, range=(0,3000))
centers = (edges[:-1] + edges[1:]) / 2
plt.step(centers, hist, where="mid")
plt.xlabel("Energy (keV)")
plt.ylabel("Counts")
plt.title("Uncalibrated Reconstructed Spectrum")
plt.xlim(0, 3000)
plt.show()

#reconstructed_energy *= 1.56146314

# Fit optical FWHM from uncalibrated, weighted, unsmeared spectrum
unsmeared_energy = reconstructed_energy[reconstructed_energy > 0]

unsmeared_hist, unsmeared_edges = np.histogram(
    unsmeared_energy, bins=1024, range=(0, 3000)
)

unsmeared_centers = (unsmeared_edges[:-1] + unsmeared_edges[1:]) / 2
bin_width = unsmeared_edges[1] - unsmeared_edges[0]

bq_spec = bq.Spectrum.from_listmode(
    listmode_data=unsmeared_energy,
    bins=unsmeared_edges,
    is_cal=True
)

y = bq_spec.counts_vals
y_unc = np.sqrt(np.maximum(y, 1))
dx = np.full_like(bq_spec.bin_centers_kev, bin_width, dtype=float)

model = (
    bq.fitting.GaussModel(prefix="gauss0_") +
    bq.fitting.LineModel(prefix="linear_")
)

fitter = bq.Fitter(
    model,
    x=bq_spec.bin_centers_kev,
    y=y,
    y_unc=y_unc,
    dx=dx,
    roi=(reconstructed_mean - 100, reconstructed_mean + 100)
)

fitter.fit(backend="lmfit")
centroid = fitter.param_val("gauss0_mu")
sigma = fitter.param_val("gauss0_sigma")
peak_area = fitter.param_val("gauss0_amp")
optical_fwhm = fitter.param_val("gauss0_fwhm")

print("\nWeighted Uncalibrated K-40 peak fit:")
print("Centroid:", centroid, "keV")
print("Sigma:", sigma, "keV")
print("Peak area:", peak_area, "counts")
print("Optical FWHM:", optical_fwhm, "keV")
print("Optical FWHM:", 100 * optical_fwhm / centroid, "%")

fitter.custom_plot()
plt.tight_layout()
plt.show()

cal_factor = 1460.8/centroid
reconstructed_energy *= cal_factor
optical_fwhm *= cal_factor

#deterministic detector resolution convolution
n_bins = 1024
energy_range = (0,3000)

#histogram unsmeared calibrated spectrum
unsmeared_hist, edges = np.histogram(reconstructed_energy[reconstructed_energy > 0], bins=n_bins,range=energy_range)
centers = (edges[:-1] + edges[1:]) / 2

bin_width = edges[1] - edges[0]

a = 1.7655580395167625e-16
b = 4.266077307991198
c = 0.00012501425424887478

fwhm_squared = a + b * centers + c * centers**2
predicted_fwhm = np.sqrt(np.maximum(0, fwhm_squared))

fwhm_to_sigma = 1 / 2.355

# calibrated optical FWHM at 1460.8 keV
optical_fraction = optical_fwhm / 1460.8

# optical width already present at each energy
optical_fwhm_bins = optical_fraction * centers

sigma_optical_bins = optical_fwhm_bins * fwhm_to_sigma
sigma_target_bins = predicted_fwhm * fwhm_to_sigma

# extra Gaussian broadening needed
sigma_add_bins = np.sqrt(
    np.maximum(
        0,
        sigma_target_bins**2 - sigma_optical_bins**2
    )
)

response_matrix = np.zeros((n_bins, n_bins))

for i, (energy, sigma) in enumerate(zip(centers, sigma_add_bins)):
    if sigma <= 0:
        response_matrix[i, i] = 1.0
        continue

    z_hi = (edges[1:] - energy) / sigma
    z_lo = (edges[:-1] - energy) / sigma
    response_matrix[:, i] = (ndtr(z_hi) - ndtr(z_lo))

hist = (response_matrix @ unsmeared_hist)

print("Unsmeared counts:", np.sum(unsmeared_hist))
print("Smeared counts:", np.sum(hist))


print("\nNumber of events:", len(reconstructed_energy))
print("Minimum:", np.min(reconstructed_energy))
print("Maximum:", np.max(reconstructed_energy))
print("Mean:", np.mean(reconstructed_energy))
print("Non-zero events:", np.sum(reconstructed_energy > 0))
print("Unique values:", len(np.unique(reconstructed_energy)))

centers = (edges[:-1] + edges[1:]) / 2

peak_index = np.argmax(hist)
peak_energy = centers[peak_index]

print("Highest bin:", peak_energy, "keV")
print("Peak counts:", hist[peak_index])

roi = (centers > 1300) & (centers < 1600)
roi_peak_index = np.argmax(hist[roi])
roi_centers = centers[roi]
roi_hist = hist[roi]
k40_peak_energy = roi_centers[roi_peak_index]

print("K-40 peak near 1460 keV:", k40_peak_energy, "keV")
print("K-40 peak counts:", roi_hist[roi_peak_index])

plt.step(centers, hist, where="mid")
plt.xlabel("Energy (keV)")
plt.ylabel("Counts")
plt.title("Weighted + Smeared Energy Spectrum")
plt.xlim(0, 3000)
plt.show()

rng = np.random.default_rng(12345)

N = int(np.sum(unsmeared_hist))

p = unsmeared_hist / np.sum(unsmeared_hist)

areas = []

for _ in range(1000):

    # New statistically equivalent Geant4 spectrum
    bootstrap_unsmeared = rng.multinomial(N, p)

    # Deterministic detector convolution
    bootstrap_smeared = (
        response_matrix @ bootstrap_unsmeared
    )

    y = bootstrap_smeared

    fitter = bq.Fitter(
        model,
        x=centers,
        y=y,
        y_unc=np.ones_like(y),  # only used to obtain fit;
                                # don't trust lmfit's covariance here
        dx=dx,
        roi=(1360.8, 1560.8)
    )

    fitter.fit(backend="lmfit")

    areas.append(
        fitter.param_val("gauss0_amp")
    )

areas = np.asarray(areas)

print("\nMonte Carlo statistical uncertainty:")
print(
    f"Area = {np.mean(areas):.1f}"
    f" +/- {np.std(areas, ddof=1):.1f}"
)

print(
    "68% CI:",
    np.percentile(areas, [16, 84])
)

y = hist
y_unc = np.sqrt(np.maximum(y, 1))
dx = np.full_like(centers, bin_width, dtype=float)

model = (
    bq.fitting.GaussModel(prefix="gauss0_") +
    bq.fitting.LineModel(prefix="linear_")
)

fitter = bq.Fitter(
    model,
    x=bq_spec.bin_centers_kev,
    y=y,
    y_unc=y_unc,
    dx=dx,
    roi=(1360.8, 1560.8)
)
fitter.fit(backend="lmfit", guess={
    "gauss0_mu": 1460.8
})
centroid = fitter.param_val("gauss0_mu")
sigma = fitter.param_val("gauss0_sigma")
peak_area = fitter.param_val("gauss0_amp")
optical_fwhm = fitter.param_val("gauss0_fwhm")
print("\nWeighted + Calibrated + Smeared K-40 Peak Fit:")
print("Centroid:", centroid, "keV")
print("Sigma:", sigma, "keV")
print("Peak area:", peak_area, "counts")
print("Optical FWHM:", optical_fwhm, "keV")
print("Optical FWHM:", 100 * optical_fwhm / centroid, "%")
eff_sim =peak_area/100000000
sigma_eff_sim = np.std(areas, ddof=1)/100000000
sigma_eff_sim_rel = np.sqrt((sigma_eff_sim/eff_sim)**2 + (5)**2)
sigma_sim_total = sigma_eff_sim_rel * eff_sim
print("Simulated K-40 peak efficiency:", eff_sim)
print("Standard deviation of simulated peak efficiency:", sigma_eff_sim)
print("Relative uncertainty of simulated peak efficiency:", sigma_eff_sim_rel)
print("Total uncertainty of simulated peak efficiency:", sigma_sim_total)
fitter.custom_plot()
plt.tight_layout()
plt.show()

#put measured spectrum through identical fitting routine

kcl_foreground = bq.Spectrum.from_file("kcl_foreground.spe")
shielded_background = bq.Spectrum.from_file("shielded_background.spe")

counts_bg = shielded_background.counts_vals
livetime_bg = shielded_background.livetime

counts_fg = kcl_foreground.counts_vals
livetime_fg = kcl_foreground.livetime

print(livetime_fg)
print(livetime_bg)

print(np.allclose(kcl_foreground.bin_edges_kev, shielded_background.bin_edges_kev))

alpha = livetime_fg / livetime_bg

net_counts = counts_fg - alpha * counts_bg

net_uncs = np.sqrt(counts_fg + alpha**2 * counts_bg)

net_spectrum = bq.Spectrum(counts=net_counts, uncs=net_uncs, bin_edges_kev=kcl_foreground.bin_edges_kev,livetime=kcl_foreground.livetime)

y = net_spectrum.counts_vals
y_unc = net_spectrum.counts_uncs
x = net_spectrum.bin_centers_kev
dx = net_spectrum.bin_widths_kev

model = (
    bq.fitting.GaussModel(prefix="gauss0_") +
    bq.fitting.LineModel(prefix="linear_")
)

fitter = bq.Fitter(
    model,
    x=x,
    y=y,
    y_unc=y_unc,
    dx=dx,
    roi=(1360.8, 1560.8)
)

fitter.fit(backend="lmfit", guess={
    "gauss0_mu": 1460.8,
    "gauss0_sigma": 35.0,
    "gauss0_amp": 21800.0
})

peak_counts = fitter.param_val("gauss0_amp")
centroid = fitter.param_val("gauss0_mu")
sigma = fitter.param_val("gauss0_sigma")
sigma_unc = fitter.param_unc("gauss0_sigma")
fwhm = 2.355 * sigma
fwhm_unc = 2.355 * sigma_unc

try:
    peak_counts_unc = fitter.param_unc("gauss0_amp")
except AttributeError:
    peak_counts_unc = fitter.result.params["gauss0_amp"].stderr

live_time_s = 531951.56
peak_cps = peak_counts/live_time_s
peak_cps_unc = peak_counts_unc/live_time_s
activity_ref = 3837.106899
activity_ref_unc = 8.1687222
k40_yield = 0.1034
k40_yield_unc = 0.0007
peak_eff = peak_cps/(activity_ref * k40_yield)
peak_eff_rel_unc = np.sqrt((peak_cps_unc/peak_cps)**2 + (activity_ref_unc/activity_ref)**2 + (k40_yield_unc/k40_yield)**2)
peak_eff_unc = peak_eff * peak_eff_rel_unc
activity_calculated = peak_cps/(eff_sim * k40_yield)
activity_calculated_unc = activity_calculated * np.sqrt((peak_cps_unc/peak_cps)**2 + (sigma_eff_sim/eff_sim)**2 + (k40_yield_unc/k40_yield)**2)



print("\nMeasured K-40 peak analysis:")
print("Centroid:", centroid)
print("Sigma:", sigma)
print("FWHM:", fwhm)
print("Peak area:", peak_counts, "+/-", peak_counts_unc)

print("\nLive time:", live_time_s, "s")
print("Peak CPS:", peak_cps, "+/-", peak_cps_unc)
print("Measured K-40 peak efficiency:", peak_eff, "+/-", peak_eff_unc)
print("Reference activity (Bq):", activity_ref, "+/-", activity_ref_unc, "Bq")
print("Calculated Activity (Bq):", activity_calculated, "+/-", activity_calculated_unc, "Bq")

print("\nComparison of simulated and measured K-40 peak efficiency:")
print("Simulated efficiency:", eff_sim, "+/-", sigma_sim_total)
print("Measured efficiency:", peak_eff, "+/-", peak_eff_unc)
delta_eff = peak_eff - eff_sim
delta_eff_unc = np.sqrt(sigma_sim_total**2 + peak_eff_unc**2)
print("Difference (measured - simulated):", delta_eff, "+/-", delta_eff_unc)
print("Agreement:", delta_eff/delta_eff_unc, "sigma")

fitter.custom_plot()
plt.tight_layout()
plt.show()