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
from pathlib import Path
import re

optical_map = pd.read_csv("optical_response_map.csv")

x_values = np.sort(optical_map["x_mm"].unique())
y_values = np.sort(optical_map["y_mm"].unique())
z_values = np.sort(optical_map["z_mm"].unique())

optical_response_grid = (
optical_map.sort_values(["x_mm", "y_mm", "z_mm"])["efficiency"]
.to_numpy()
.reshape(len(x_values), len(y_values), len(z_values))
)

interpolator = RegularGridInterpolator(
(x_values, y_values, z_values),
optical_response_grid,
bounds_error=False,
fill_value=0
)

eta_ref = interpolator([[0.0, 4.999, 0.5]])[0]

scan_dir = Path("./Release/energy_scan")

root_files = sorted(scan_dir.glob("*.root"))

results = []

for root_file in root_files:
    match = re.search(r"(\d+)keV", root_file.stem)

    if match is None:
        continue
    source_energy = float(match.group(1))
    print("Processing", source_energy, "keV...")

    file = uproot.open(root_file)

    tree = file["t"]

    raw = tree["RawEdep"].array(library="np") * 1000
    stepE = tree["fStepEnergy"].array()
    stepX = tree["fStepX"].array()
    stepY = tree["fStepY"].array()
    stepZ = tree["fStepZ"].array()

    flat_x = ak.to_numpy(ak.flatten(stepX))
    flat_y = ak.to_numpy(ak.flatten(stepY))
    flat_z = ak.to_numpy(ak.flatten(stepZ))

    positions = np.column_stack((flat_x, flat_y, flat_z))
    flat_response = interpolator(positions)

    outside = (
    (flat_x < x_values.min()) | (flat_x > x_values.max()) |
    (flat_y < y_values.min()) | (flat_y > y_values.max()) |
    (flat_z < z_values.min()) | (flat_z > z_values.max())
    )

    flat_stepE = ak.to_numpy(ak.flatten(stepE))

    weighted_stepE = flat_stepE * flat_response
    event_lengths = ak.num(stepE, axis=1)
    weighted_stepE = ak.unflatten(weighted_stepE, event_lengths)
    weighted_energy = ak.sum(weighted_stepE, axis=1)

    raw_event_energy = ak.to_numpy(ak.sum(stepE, axis=1))
    weighted_energy_np = ak.to_numpy(weighted_energy)

    source_energy_kev = float(match.group(1))
    source_energy_mev = source_energy_kev / 1000.0

    full_energy = (
    (raw_event_energy > 0.98 * source_energy_mev) &
    (raw_event_energy < 1.02 * source_energy_mev)
    )

    if np.sum(full_energy) > 0:
        optical_factor = weighted_energy_np[full_energy] / raw_event_energy[full_energy]
        reconstructed_full = weighted_energy_np[full_energy] / eta_ref

    reconstructed_energy = ak.to_numpy(weighted_energy / eta_ref) * 1000
    reconstructed_mean = np.mean(reconstructed_full) * 1000

    hist, edges = np.histogram(reconstructed_energy[reconstructed_energy > 0], bins=1024, range=(0,3000))
    centers = (edges[:-1] + edges[1:]) / 2

    unsmeared_energy = reconstructed_energy[reconstructed_energy > 0]
    unsmeared_hist, unsmeared_edges = np.histogram(unsmeared_energy, bins=1024, range=(0, 3000))
    unsmeared_centers = (unsmeared_edges[:-1] + unsmeared_edges[1:]) / 2
    bin_width = unsmeared_edges[1] - unsmeared_edges[0]

    bq_spec = bq.Spectrum.from_listmode(listmode_data=unsmeared_energy, bins=unsmeared_edges, is_cal=True)

    y = bq_spec.counts_vals
    y_unc = np.sqrt(np.maximum(y, 1))
    dx = np.full_like(bq_spec.bin_centers_kev, bin_width, dtype=float)

    model = (bq.fitting.GaussModel(prefix="gauss0_") + bq.fitting.LineModel(prefix="linear_"))
    fitter = bq.Fitter(model, x=bq_spec.bin_centers_kev, y=y, y_unc=y_unc, dx=dx, roi=(reconstructed_mean - 100, reconstructed_mean + 100))
    fitter.fit(backend="lmfit")

    x = bq_spec.bin_centers_kev
    x_fit = np.linspace(fitter.x_roi[0], fitter.x_roi[-1], 1000)
    y_fit = fitter.eval(x_fit, **fitter.best_values) * bin_width

    centroid = fitter.param_val("gauss0_mu")
    optical_fwhm = fitter.param_val("gauss0_fwhm")

    cal_factor = source_energy / centroid

    reconstructed_energy *= cal_factor
    optical_fwhm *= cal_factor

    # deterministic convolution
    n_bins = 1024
    energy_range = (0, 3000)

    unsmeared_hist, edges = np.histogram(
        reconstructed_energy[reconstructed_energy > 0],
        bins=n_bins,
        range=energy_range
    )

    centers = (edges[:-1] + edges[1:]) / 2
    bin_width = edges[1] - edges[0]


    # resolution model
    a = 1.7655580395167625e-16
    b = 4.266077307991198
    c = 0.00012501425424887478

    fwhm_squared = a + b * centers + c * centers**2
    predicted_fwhm = np.sqrt(np.maximum(0, fwhm_squared))

    fwhm_to_sigma = 1 / 2.355

    optical_fraction = optical_fwhm / source_energy
    optical_fwhm_bins = optical_fraction * centers

    sigma_optical_bins = optical_fwhm_bins * fwhm_to_sigma
    sigma_target_bins = predicted_fwhm * fwhm_to_sigma

    sigma_add_bins = np.sqrt(
        np.maximum(
            0,
            sigma_target_bins**2 - sigma_optical_bins**2
        )
    )


    # build detector response matrix
    detector_response_matrix = np.zeros((n_bins, n_bins))

    for i, (bin_energy, sigma) in enumerate(
        zip(centers, sigma_add_bins)
    ):
        if sigma <= 0:
            detector_response_matrix[i, i] = 1.0
            continue

        z_hi = (edges[1:] - bin_energy) / sigma
        z_lo = (edges[:-1] - bin_energy) / sigma

        detector_response_matrix[:, i] = (
            ndtr(z_hi) - ndtr(z_lo)
        )


    # deterministic smeared spectrum
    hist = detector_response_matrix @ unsmeared_hist


    # final peak ROI
    target_fwhm = np.sqrt(
        a + b * source_energy + c * source_energy**2
    )

    roi_half_width = max(
        3.0 * target_fwhm,
        30.0
    )

    roi = (
        source_energy - roi_half_width,
        source_energy + roi_half_width
    )


    # final peak fit
    y_final = hist
    y_unc_final = np.sqrt(np.maximum(y_final, 1.0))
    dx_final = np.full_like(centers, bin_width)

    final_model = (
        bq.fitting.GaussModel(prefix="gauss0_")
        + bq.fitting.LineModel(prefix="linear_")
    )

    final_fitter = bq.Fitter(
        final_model,
        x=centers,
        y=y_final,
        y_unc=y_unc_final,
        dx=dx_final,
        roi=roi
    )

    final_fitter.fit(
        backend="lmfit",
        guess={
            "gauss0_mu": source_energy
        }
    )

    final_centroid = final_fitter.param_val("gauss0_mu")
    final_sigma = final_fitter.param_val("gauss0_sigma")
    final_fwhm = 2.355 * final_sigma
    peak_area = final_fitter.param_val("gauss0_amp")


    # efficiency
    if source_energy <= 1000:
        n_primaries = 10000000
    else:
        n_primaries = 100000000
    efficiency = peak_area / n_primaries


    # bootstrap statistical uncertainty
    N = int(np.sum(unsmeared_hist))
    p = unsmeared_hist / np.sum(unsmeared_hist)
    n_bootstrap = 50
    rng = np.random.default_rng(int(source_energy))

    bootstrap_unsmeared = rng.multinomial(N, p, size=n_bootstrap)

    bootstrap_smeared_all = (
    bootstrap_unsmeared
    @ detector_response_matrix.T
    )

    bootstrap_areas = []

    for bootstrap_smeared in bootstrap_smeared_all:

        bootstrap_unc = np.sqrt(
            np.maximum(bootstrap_smeared, 1.0)
        )

        bootstrap_fitter = bq.Fitter(
            final_model,
            x=centers,
            y=bootstrap_smeared,
            y_unc=bootstrap_unc,
            dx=dx_final,
            roi=roi
        )

        try:
            bootstrap_fitter.fit(
                backend="lmfit",
                guess={
                    "gauss0_mu": source_energy
                }
            )

            bootstrap_areas.append(
                bootstrap_fitter.param_val("gauss0_amp")
            )

        except Exception:
            pass

    # bootstrap results
    bootstrap_areas = np.asarray(bootstrap_areas)

    peak_area_unc = np.std(
        bootstrap_areas,
        ddof=1
    )

    efficiency_unc_stat = (
        peak_area_unc / n_primaries
    )

    efficiency_rel_unc_stat = (
        efficiency_unc_stat / efficiency
    )


    # total uncertainty with 5% systematic
    systematic_rel = 0.05

    efficiency_rel_unc_total = np.sqrt(
        efficiency_rel_unc_stat**2
        + systematic_rel**2
    )

    efficiency_unc_total = (
        efficiency
        * efficiency_rel_unc_total
    )

    centroid_error = abs(final_centroid - source_energy) / source_energy
    relative_stat_unc = efficiency_unc_stat / efficiency

    valid_fit = (
        centroid_error < 0.02
        and final_fwhm > 0
        and final_fwhm < 3 * target_fwhm
        and peak_area > 0
        and relative_stat_unc < 0.5
    )

    if not valid_fit:
        print(
            f"WARNING: rejecting {source_energy:.0f} keV "
            f"(centroid={final_centroid:.1f}, "
            f"FWHM={final_fwhm:.1f}, "
            f"rel unc={relative_stat_unc:.1%})"
        )

    # store one result for this energy
    results.append({
        "energy_keV": source_energy,
        "centroid_keV": final_centroid,
        "fwhm_keV": final_fwhm,
        "peak_area": peak_area,
        "peak_area_unc": peak_area_unc,
        "efficiency": efficiency,
        "efficiency_unc_stat": efficiency_unc_stat,
        "efficiency_unc_total": efficiency_unc_total,
        "efficiency_rel_unc_stat": efficiency_rel_unc_stat,
        "efficiency_rel_unc_total": efficiency_rel_unc_total
    })

for i in results:
    print(i["energy_keV"])
    print(i["peak_area"])
    print(i["efficiency"])
    print(i["efficiency_rel_unc_total"])