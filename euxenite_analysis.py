import becquerel as bq
import numpy as np
import pandas as pd
from scipy.interpolate import interp1d
import matplotlib.pyplot as plt
from scipy.stats import chi2

eux_fg = bq.Spectrum.from_file("euxenite_foreground.spe")
eux_bg = bq.Spectrum.from_file("euxenite_background.spe")

counts_fg = eux_fg.counts_vals
counts_bg = eux_bg.counts_vals

livetime_fg = eux_fg.livetime
livetime_bg = eux_bg.livetime

alpha = livetime_fg/livetime_bg

net_counts = counts_fg - alpha * counts_bg

net_uncs = np.sqrt(counts_fg + alpha**2 * counts_bg)

net_spectrum = bq.Spectrum(counts=net_counts, uncs=net_uncs, bin_edges_kev=eux_fg.bin_edges_kev, livetime=livetime_fg)

y = net_spectrum.counts_vals
y_unc = net_spectrum.counts_uncs
x = net_spectrum.bin_centers_kev
dx = net_spectrum.bin_widths_kev

centers = net_spectrum.bin_centers_kev
edges = net_spectrum.bin_edges_kev

eff_data = pd.read_csv("efficiency_curve.csv")

energy_grid = eff_data["energy_keV"].to_numpy()
eff_grid = eff_data["efficiency"].to_numpy()
rel_unc_grid = eff_data["efficiency_rel_unc_total"].to_numpy()

eff_interp = interp1d(energy_grid, eff_grid, kind="linear", bounds_error=True)

rel_unc_interp = interp1d(energy_grid, rel_unc_grid, kind="linear", bounds_error=True)

#fwhm coefficients
a = 1.7655580395167625e-16
b = 4.266077307991198
c = 0.00012501425424887478

fwhm_662kev = np.sqrt(a + b * 662 + c * 662**2)
ref_energy = 662

kernel = bq.GaussianPeakFilter(ref_x=ref_energy, ref_fwhm=fwhm_662kev, fwhm_at_0=0.5)
peakfinder = bq.PeakFinder(net_spectrum, kernel)

peakfinder.find_peaks(min_snr=3, max_num=50)

peak_channels = np.array(peakfinder.centroids)
peak_fwhm_channels = np.array(peakfinder.fwhms)
peak_areas = np.array(peakfinder.integrals)

peak_energies = np.interp(peak_channels, net_spectrum.bin_centers_raw, net_spectrum.bin_centers_kev)

left_channels = peak_channels - peak_fwhm_channels/2
right_channels = peak_channels + peak_fwhm_channels/2

left_keV = np.interp(left_channels, net_spectrum.bin_centers_raw, net_spectrum.bin_centers_kev)

right_keV = np.interp(right_channels, net_spectrum.bin_centers_raw, net_spectrum.bin_centers_kev)

peak_fwhm_keV = right_keV - left_keV

peak_df = pd.DataFrame({
    "centroid_channel": peak_channels,
    "centroid_keV": peak_energies,
    "FWHM_channel": peak_fwhm_channels,
    "FWHM_keV": peak_fwhm_keV,
    "area": peak_areas,
    "snr": peakfinder.snrs
})

for E, FWHM, area in zip(peak_energies, peak_fwhm_keV, peak_areas):
    print(f"E = {E:7.1f} keV, "
    f"FWHM = {FWHM:5.1f} keV, "
    f"Area = {area:8.0f}"
    )

singlet_model = bq.fitting.GaussModel(prefix="gauss0_") + bq.fitting.LineModel(prefix="linear_")

fitter_238 = bq.Fitter(singlet_model, x=centers, y=y, y_unc=y_unc, dx=dx, roi=(238 - 35, 238 + 35))
fitter_238.fit(backend="lmfit")

centroid_238 = fitter_238.param_val("gauss0_mu")
counts_238 = fitter_238.param_val("gauss0_amp")
counts_238_unc = fitter_238.param_unc("gauss0_amp")
cps_238 = counts_238/livetime_fg
cps_238_unc = counts_238_unc/livetime_fg
yield_238 = 0.436
print("\nCentroid, 238 keV:", centroid_238)
print("238 keV peak CPS:", cps_238, "+/-", cps_238_unc)

eff_238 = float(eff_interp(centroid_238))
eff_rel_unc_238 = float(rel_unc_interp(centroid_238))

eff_unc_238 = eff_238 * eff_rel_unc_238

activity_238 = cps_238/(eff_238 * yield_238)
act_238_rel_unc = np.sqrt((cps_238_unc/cps_238)**2 + eff_rel_unc_238**2)
act_238_unc = activity_238 * act_238_rel_unc

print("Pb-212 238 keV activity:", activity_238, "+/-", act_238_unc, "Bq")

fitter_583 = bq.Fitter(singlet_model, x=centers, y=y, y_unc=y_unc, dx=dx, roi=(583 - 54.4, 583 + 54.4))
fitter_583.fit(backend="lmfit")

centroid_583 = fitter_583.param_val("gauss0_mu")
counts_583 = fitter_583.param_val("gauss0_amp")
counts_583_unc = fitter_583.param_unc("gauss0_amp")
cps_583 = counts_583/livetime_fg
cps_583_unc = counts_583_unc/livetime_fg
yield_583 = 0.85 * 0.3585
print("\nCentroid, 583 keV:", centroid_583)
print("583 keV peak CPS:", cps_583, "+/-", cps_583_unc)

eff_583 = float(eff_interp(centroid_583))
eff_rel_unc_583 = float(rel_unc_interp(centroid_583))

eff_unc_583 = eff_583 * eff_rel_unc_583

activity_583 = cps_583/(eff_583 * yield_583)
act_583_rel_unc = np.sqrt((cps_583_unc/cps_583)**2 + eff_rel_unc_583**2)
act_583_unc = activity_583 * act_583_rel_unc

print("Tl-208 583 keV activity:", activity_583, "+/-", act_583_unc, "Bq")

#fit 911/969 keV doublet as one feature
feature_model = bq.fitting.GaussModel(prefix="feature_") + bq.fitting.LineModel(prefix="linear_")

feature_fitter = bq.Fitter(feature_model, x=x, y=y, y_unc=y_unc, dx=dx, roi=(850, 1050))

feature_fitter.fit(backend="lmfit")

feature_mu = feature_fitter.param_val("feature_mu")
feature_sigma = feature_fitter.param_val("feature_sigma")
feature_area = feature_fitter.param_val("feature_amp")

def predict_fwhm(energy):
    predicted_fwhm = np.sqrt(a + b * energy + c * energy**2)
    return predicted_fwhm

#fit 911/969 separately within it
doublet_model = bq.fitting.GaussModel(prefix="g911_") + bq.fitting.GaussModel(prefix="g969_") + bq.fitting.LineModel(prefix="linear_")

doublet_fitter = bq.Fitter(doublet_model, x=x, y=y, y_unc=y_unc, dx=dx, roi=(850, 1050))

#guess values
doublet_fitter.params["g911_mu"].set(value=911.2)
doublet_fitter.params["g969_mu"].set(value=968.97)

expected_sigma_911 = predict_fwhm(911.2)/2.355

doublet_fitter.params["g911_sigma"].set(value=expected_sigma_911, vary=False)

doublet_fitter.params["g911_amp"].set(value=0.65 * feature_area, min=0)
doublet_fitter.params["g969_amp"].set(value=0.35 * feature_area, min=0)

#constrain 969 centroid relative to 911
doublet_fitter.params["g969_mu"].expr = "g911_mu + 57.77"

#constrain peaks to share width
doublet_fitter.params["g969_sigma"].expr = "g911_sigma"
doublet_fitter.params["g911_sigma"].set(min=5, max=60)

doublet_fitter.fit(backend="lmfit")

centroid_911 = doublet_fitter.param_val("g911_mu")
centroid_969 = doublet_fitter.param_val("g969_mu")

area_911 = doublet_fitter.param_val("g911_amp")
area_969 = doublet_fitter.param_val("g969_amp")

area_911_unc = doublet_fitter.param_unc("g911_amp")
area_969_unc = doublet_fitter.param_unc("g969_amp")

yield_911 = 0.258
yield_969 = 0.158

eff_911 = float(eff_interp(centroid_911))
eff_969 = float(eff_interp(centroid_969))

eff_rel_unc_911 = float(rel_unc_interp(centroid_911))
eff_rel_unc_969 = float(rel_unc_interp(centroid_969))

cps_911 = area_911/livetime_fg
cps_969 = area_969/livetime_fg

cps_911_unc = area_911_unc/livetime_fg
cps_969_unc = area_969_unc/livetime_fg

activity_911 = cps_911/(eff_911 * yield_911)
act_911_rel_unc = np.sqrt((cps_911_unc/cps_911)**2 + eff_rel_unc_911**2)
act_911_unc = activity_911 * act_911_rel_unc

activity_969 = cps_969/(eff_969 * yield_969)
act_969_rel_unc = np.sqrt((cps_969_unc/cps_969)**2 + eff_rel_unc_969**2)
act_969_unc = activity_969 * act_969_rel_unc

print("\n911 keV peak CPS:", cps_911, "+/-", cps_911_unc)
print("\n969 keV peak CPS:", cps_969, "+/-", cps_969_unc)
print("Ac-228 911 keV activity:", activity_911, "+/-", act_911_unc, "Bq")
print("Ac-228 969 keV activity:", activity_969, "+/-", act_969_unc, "Bq")

#calculate weighted average of Ac-228 peaks
activities = np.array([activity_911, activity_969])
uncs = np.array([act_911_unc, act_969_unc])

weights = 1/uncs**2

weighted_activity = np.sum(weights * activities) / np.sum(weights)
weighted_unc = np.sqrt(1/np.sum(weights))

print(f"Ac-228 weighted activity: {weighted_activity} +/- {weighted_unc} Bq")

#calculate weighted avg th232 activity estimate
nuclide_activities = np.array([activity_238, activity_583, weighted_activity])
nuclide_uncs = np.array([act_238_unc, act_583_unc, weighted_unc])

weights = 1 / nuclide_uncs**2

activity_th232 = np.sum(weights * nuclide_activities)/np.sum(weights)
act_th232_unc = np.sqrt(1/np.sum(weights))
print(f"\nWeighted mean Th-232 activity estimate: {activity_th232} +/- {act_th232_unc} Bq")
print(f"Total sample activity, full decay chain: {activity_th232 * 11} +/- {act_th232_unc * 11} Bq")

#chi squared
chi2_val = np.sum((nuclide_activities - activity_th232)**2 / nuclide_uncs**2)

dof = len(nuclide_activities) - 1
reduced_chi2 = chi2_val/dof

p_val = chi2.sf(chi2_val, dof)

print("Chi squared:", chi2_val)
print("Reduced chi squared:", reduced_chi2)
print("Degrees of freedom:", dof)
print("p-value:", p_val)

fig1, ax = plt.subplots(figsize=(12,6))

ax.step(x, y, where="mid", label="Spectrum", linewidth=1)

for fitter, label in [(fitter_238, "Pb-212 238 keV"), (fitter_583, "Tl-208 583 keV"), (doublet_fitter, "Ac-228 911/969 keV")]:
    x_fit = fitter.x_roi
    y_fit = fitter.eval(x_fit, params=fitter.result.params)
    dx_fit = fitter.dx_roi
    y_fit *= dx_fit

    ax.plot(x_fit, y_fit, linewidth=2, label=label)

x_fit = doublet_fitter.x_roi
dx_fit = doublet_fitter.dx_roi

components = doublet_fitter.result.eval_components(x=x_fit)

background = components["linear_"] * dx_fit

ax.plot(x_fit, (components["g911_"] * dx_fit) + background, linestyle="--", label="911 keV")
ax.plot(x_fit, (components["g969_"] * dx_fit) + background, linestyle="--", label="969 keV")

ax.set_xlabel("Energy (keV)")
ax.set_ylabel("Counts")
ax.set_yscale("log")
ax.legend()
ax.set_xlim(0, max(x))
plt.tight_layout()

plt.show()
