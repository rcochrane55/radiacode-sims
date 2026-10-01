import uproot
import awkward as ak
import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit

file = uproot.open("rootOutput.root")
tree = file["t"]
#print(tree.keys())
#print(tree.num_entries)
raw = tree["RawEdep"].array(library="np") * 1000

stepE = tree["fStepEnergy"].array()
stepX = tree["fStepX"].array()
stepY = tree["fStepY"].array()
stepZ = tree["fStepZ"].array()

print(stepE[0])
print(stepX[0])
print(stepY[0])
print(stepZ[0])

step_sum = ak.sum(stepE, axis=1) * 1000

has_deposit = step_sum > 0

difference = raw - ak.to_numpy(step_sum)

sipm_centerx = 0
sipm_centery = 5
sipm_centerz = -1.2

distance = np.sqrt((sipm_centerx - stepX) ** 2 + (sipm_centery - stepY) ** 2 + (sipm_centerz - stepZ) ** 2)

#print("Events with deposits:", np.sum(has_deposit))
#print("Events with no deposits:", np.sum(~has_deposit))
#print("Maximum difference:", np.max(np.abs(difference)))
#print("Mean difference:", np.mean(difference))

#print("Maximum distance:", ak.max(distance))
#print("Minimum distance:", ak.min(distance))

weight = np.exp(-distance/54.8)

weighted_energy = ak.sum(1.126308404 * stepE * weight, axis=1)

weighted_energy = ak.to_numpy(weighted_energy) * 1000

def smear_energy(E):
    a = -391.425599
    b = 6.723136106
    c = -0.000543789

    fwhm2 = a + (b*E) + (c*E)**2
    fwhm2 = np.maximum(fwhm2, 0.1)  # Ensure non-negative values
    sigma = np.sqrt(fwhm2) / 2.355

    return np.random.normal(E, sigma)

smeared = np.zeros_like(weighted_energy)

mask = weighted_energy > 0

smeared[mask] = smear_energy(weighted_energy[mask])

print("\nRAW")
print("min:", np.min(raw))
print("max:", np.max(raw))
print("mean:", np.mean(raw))
print("nonzero:", np.count_nonzero(raw))

print("\nWEIGHTED")
print("min:", np.min(weighted_energy))
print("max:", np.max(weighted_energy))
print("mean:", np.mean(weighted_energy))
print("nonzero:", np.count_nonzero(weighted_energy))

print("\nSMEARED")
print("min:", np.min(smeared))
print("max:", np.max(smeared))
print("mean:", np.mean(smeared))
print("nonzero:", np.count_nonzero(smeared))
print("finite:", np.count_nonzero(np.isfinite(smeared)))

def gaussian_linear_bkg(x, A, mu, sigma, m, b):
    return A * np.exp(-(x - mu) ** 2 / (2 * sigma ** 2)) + m * x + b


def fit_peak(energies):
    hist, edges = np.histogram(energies, bins=120, range=(1350, 1575))
    centers = (edges[:-1] + edges[1:]) / 2
    bin_width = edges[1] - edges[0]

    a_0 = hist.max()
    mu_0 = 1460.8
    sigma_0 = 30
    m_0 = 0
    b_0 = hist.min()
    p_0 = [a_0, mu_0, sigma_0, m_0, b_0]

    params, covariance = curve_fit(
        gaussian_linear_bkg,
        centers,
        hist,
        p0=p_0,
        bounds=(
            [0, 1400, 0.1, -np.inf, -np.inf],
            [np.inf, 1525, 100, np.inf, np.inf],
        ),
    )

    A, mu, sigma, m, b = params
    peak_area = A * sigma * np.sqrt(2 * np.pi) / bin_width

    return peak_area, params, hist, edges, centers

peak_area, params, peak_hist, peak_edges, peak_centers = fit_peak(
    smeared[smeared > 0]
)
A, mu, sigma, m, b = params

print("\n1460.8 keV PEAK FIT")
print(f"Fitted centroid: {mu:.2f} keV")
print(f"Fitted sigma:    {sigma:.2f} keV")
print(f"Peak area:       {peak_area:.1f} counts")

plt.figure(figsize=(10, 6))

plt.hist(
    smeared[smeared > 1],
    bins=1024,
    range=(0, 3000),
    histtype="step",
    label="Weighted + Smeared Energy",
)

fit_x = np.linspace(1350, 1575, 1000)
fit_y = gaussian_linear_bkg(fit_x, *params)

# Scale fitted counts/bin to match the wide-spectrum histogram bin width.
wide_bin_width = 3000 / 1024
fit_y_scaled = fit_y * wide_bin_width / (peak_edges[1] - peak_edges[0])

plt.plot(
    fit_x,
    fit_y_scaled,
    color="red",
    label=f"1460.8 keV fit (area = {peak_area:.0f})",
)

plt.xlabel("Energy (keV)")
plt.ylabel("Counts")
plt.legend()
plt.show()
"""
stepE = tree["fStepEnergy"].array()
stepX = tree["fStepX"].array()
stepY = tree["fStepY"].array()
stepZ = tree["fStepZ"].array()
rawE = tree["RawEdep"].array()
print("Total events:", len(rawE))
nonzero = np.nonzero(np.asarray(rawE) > 0)[0]
print("Events with non-zero RawEdep:", len(nonzero))
print("\nFirst non-zero events:")
 for i in nonzero[:20]:
    print("=" * 50)
    print("Event:", i)
    print("RawEdep:", rawE[i])
    print("fStepEnergy:", stepE[i])
    print("fStepX:", stepX[i])
    print("fStepY:", stepY[i])
    print("fStepZ:", stepZ[i]) 
"""

# Works 5 hours after - dzybba
