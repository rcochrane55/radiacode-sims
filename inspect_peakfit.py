import uproot
import awkward as ak
import numpy as np
import matplotlib.pyplot as plt
#from scipy.optimize import curve_fit
from scipy.signal import find_peaks, peak_widths


file = uproot.open("rootOutput.root")
tree = file["t"]
#print(tree.keys())
#print(tree.num_entries)
raw = tree["RawEdep"].array(library="np") * 1000

stepE = tree["fStepEnergy"].array()
stepX = tree["fStepX"].array()
stepY = tree["fStepY"].array()
stepZ = tree["fStepZ"].array()

#print(stepE[0])
#print(stepX[0])
#print(stepY[0])
#print(stepZ[0])

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

weighted_energy = ak.sum(stepE * weight, axis=1)

weighted_energy = ak.to_numpy(weighted_energy) * 1000

def smear_energy(E):
    a = -1252.39
    b = 8.390725
    c = -0.00205

    fwhm2 = a + (b*E) + (c*(E**2))
    fwhm2 = np.maximum(fwhm2, 0.1)  # Ensure non-negative values
    sigma = np.sqrt(fwhm2) / 2.355

    return np.random.normal(E, sigma)

#np.random.seed(12345)
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
#histogram of weighted smeared energies, not yet calibrated
hist, edges = np.histogram(
    smeared[smeared > 0],
    bins=1024,
    range=(0, 3000)
)

centers = (edges[:-1] + edges[1:]) / 2
#identified peaks before recalibration
peaks, properties = find_peaks(hist, prominence=np.max(hist) * 0.1)
print("Indices:", peaks)
print("Energies:", centers[peaks])

peak_mask = ((centers[peaks] > 1261) & (centers[peaks] < 1661))

k40_candidates = peaks[peak_mask]

#shifted K-40 centroid
k40_peak = k40_candidates[np.argmax(hist[k40_candidates])]

peak_energy = centers[k40_peak]

print("K-40 mode, uncorrected:", peak_energy)

cal_factor = 1460.8/peak_energy

print("recalibration factor:", cal_factor)

#new calibrated energies and histogram
calibrated_energies = smeared[smeared > 0] * cal_factor
calibrated_hist, edges = np.histogram(calibrated_energies, bins=1024, range=(0,3000))
centers = (edges[:-1] + edges[1:]) / 2

#search again for recalibrated peaks
calibrated_peaks, properties = find_peaks(calibrated_hist, prominence=np.max(calibrated_hist) * 0.1)

print("calibrated indices:", calibrated_peaks)
print("calibrated energies:", centers[calibrated_peaks])

cal_peak_mask = ((centers[calibrated_peaks] > 1400) & (centers[calibrated_peaks] < 1500))

cal_k40_peak = centers[calibrated_peaks[cal_peak_mask]][0]

print("K-40 mode, calibrated:", cal_k40_peak)

widths, width_heights, left_ips, right_ips = peak_widths(calibrated_hist, calibrated_peaks, rel_height=0.5)

k40_widths = widths[cal_peak_mask]
k40_left_ips = left_ips[cal_peak_mask]
k40_right_ips = right_ips[cal_peak_mask]

bin_width = edges[1] - edges[0]

k40_fwhm = k40_widths * bin_width

k40_left_energy = edges[0] + k40_left_ips * bin_width
k40_right_energy = edges[0] + k40_right_ips * bin_width

print("K-40 FWHM:", k40_fwhm[0], "keV")
print("FWHM lower bound:", k40_left_energy[0], "keV")
print("FWHM upper bound:", k40_right_energy[0], "keV")
print("FWHM %:", (k40_fwhm[0] / cal_k40_peak) * 100, "%")

plt.hist(
    calibrated_energies[calibrated_energies > 1],
    bins=1024,
    range=(0, 3000),
    histtype="step",
    label="Weighted + Smeared + Calibrated Energy",
)
#plt.yscale("log")
plt.legend()
plt.xlabel("Energy (keV)")
plt.ylabel("Counts")
plt.show()