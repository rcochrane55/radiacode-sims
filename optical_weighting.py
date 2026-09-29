import uproot
import awkward as ak
import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
from scipy.signal import find_peaks, peak_widths
import warnings
import lmfit
import becquerel as bq
from scipy.interpolate import RegularGridInterpolator
import pandas as pd

file = uproot.open("rootOutput.root")
tree = file["t"]

raw = tree["RawEdep"].array(library="np") * 1000
stepE = tree["fStepEnergy"].array()
stepX = tree["fStepX"].array()
stepY = tree["fStepY"].array()
stepZ = tree["fStepZ"].array()

print("X range:", ak.min(ak.flatten(stepX)), "to", ak.max(ak.flatten(stepX)))
print("Y range:", ak.min(ak.flatten(stepY)), "to", ak.max(ak.flatten(stepY)))
print("Z range:", ak.min(ak.flatten(stepZ)), "to", ak.max(ak.flatten(stepZ)))

optical_map = pd.read_csv("optical_response_map.csv")

print("\nOptical response statistics:")
print("Minimum:", optical_map["efficiency"].min())
print("Maximum:", optical_map["efficiency"].max())
print("Mean:", optical_map["efficiency"].mean())
print("Standard deviation:", optical_map["efficiency"].std())

x_values = np.sort(optical_map["x_mm"].unique())
y_values = np.sort(optical_map["y_mm"].unique())
z_values = np.sort(optical_map["z_mm"].unique())

print("\nOptical map ranges:")
print("X:", x_values.min(), "to", x_values.max())
print("Y:", y_values.min(), "to", y_values.max())
print("Z:", z_values.min(), "to", z_values.max())

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

eta_center = interpolator([[0.0, 4.95, 0.0]])[0]
print("Interpolated center efficiency:", eta_center)

flat_x = ak.to_numpy(ak.flatten(stepX))
flat_y = ak.to_numpy(ak.flatten(stepY))
flat_z = ak.to_numpy(ak.flatten(stepZ)) - 30.0

print("\nStep coordinate ranges:")
print("X:", flat_x.min(), "to", flat_x.max())
print("Y:", flat_y.min(), "to", flat_y.max())
print("Z:", flat_z.min(), "to", flat_z.max())

outside_x = (flat_x < x_values.min()) | (flat_x > x_values.max())
outside_y = (flat_y < y_values.min()) | (flat_y > y_values.max())
outside_z = (flat_z < z_values.min()) | (flat_z > z_values.max())

print("\nOutside by axis:")
print("X:", 100 * np.mean(outside_x), "%")
print("Y:", 100 * np.mean(outside_y), "%")
print("Z:", 100 * np.mean(outside_z), "%")

positions = np.column_stack((flat_x, flat_y, flat_z))
flat_response = interpolator(positions)

outside = (
    (flat_x < x_values.min()) | (flat_x > x_values.max()) |
    (flat_y < y_values.min()) | (flat_y > y_values.max()) |
    (flat_z < z_values.min()) | (flat_z > z_values.max())
)

flat_stepE = ak.to_numpy(ak.flatten(stepE))

print("\nEnergy outside optical map:")
print("Outside energy:", np.sum(flat_stepE[outside]))
print("Total deposited energy:", np.sum(flat_stepE))
print("Percentage energy outside:", 100 * np.sum(flat_stepE[outside]) / np.sum(flat_stepE))
print("Steps outside optical map:", np.sum(outside))
print("Total steps:", len(outside))
print("Percentage outside:", 100 * np.mean(outside), "%")

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
y_unc = np.sqrt(y)
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
    roi=(1000, 1200)
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

# Detector energy-resolution broadening
a = -1252.39
b = 8.390725
c = -0.00205

fwhm_squared = (a + b * reconstructed_energy + c * reconstructed_energy**2)
predicted_fwhm = np.sqrt(np.maximum(0, fwhm_squared))

sigma_predicted = predicted_fwhm / 2.355
sigma_optical = optical_fwhm / 2.355

sigma_additional = np.sqrt(
    np.maximum(0, sigma_predicted**2 - sigma_optical**2)
)

smeared_energy = np.random.normal(
    reconstructed_energy,
    sigma_additional)

print("\nNumber of events:", len(reconstructed_energy))
print("Minimum:", np.min(reconstructed_energy))
print("Maximum:", np.max(reconstructed_energy))
print("Mean:", np.mean(reconstructed_energy))
print("Non-zero events:", np.sum(reconstructed_energy > 0))
print("Unique values:", len(np.unique(reconstructed_energy)))

hist, edges = np.histogram(
    smeared_energy[smeared_energy > 0],
    bins=1024,
    range=(0, 3000)
)

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

bq_spec = bq.Spectrum.from_listmode(
    listmode_data=smeared_energy[smeared_energy > 0],
    bins=edges,
    is_cal=True
)

y = bq_spec.counts_vals
y_unc = np.sqrt(y)
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
    roi=(1340, 1600)
)
fitter.fit(backend="lmfit")
centroid = fitter.param_val("gauss0_mu")
sigma = fitter.param_val("gauss0_sigma")
peak_area = fitter.param_val("gauss0_amp")
optical_fwhm = fitter.param_val("gauss0_fwhm")
print("Weighted + Calibrated + Smeared K-40 Peak Fit:")
print("Centroid:", centroid, "keV")
print("Sigma:", sigma, "keV")
print("Peak area:", peak_area, "counts")
print("Optical FWHM:", optical_fwhm, "keV")
print("Optical FWHM:", 100 * optical_fwhm / centroid, "%")

fitter.custom_plot()
plt.tight_layout()
plt.show()

#full_energy = (raw > 1450) & (raw < 1470)
#full_reco = reconstructed_energy[full_energy]

""" plt.hist(full_reco, bins=200, histtype="step")
plt.xlabel("Reconstructed Energy (keV)")
plt.ylabel("Counts")
plt.title("Optical Response of Full-Energy Events")
plt.show()

step_total = ak.to_numpy(ak.sum(stepE, axis=1))
weighted_total = ak.to_numpy(weighted_energy)

response_ratio = np.divide(
    weighted_total,
    step_total,
    out=np.zeros_like(weighted_total),
    where=step_total > 0
)

full_ratio = response_ratio[full_energy]

print("Full-energy event optical response:")
print("Mean:", np.mean(full_ratio))
print("Median:", np.median(full_ratio))
print("Std:", np.std(full_ratio))
print("Min:", np.min(full_ratio))
print("Max:", np.max(full_ratio)) """

""" plt.hist(full_ratio, bins=200, histtype="step")
plt.xlabel("Energy-weighted optical efficiency")
plt.ylabel("Events")
plt.show() """