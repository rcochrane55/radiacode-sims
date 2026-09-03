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
#smeared = np.zeros_like(weighted_energy)

#mask = weighted_energy > 0

#smeared[mask] = smear_energy(weighted_energy[mask])

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

"""
print("\nSMEARED")
print("min:", np.min(smeared))
print("max:", np.max(smeared))
print("mean:", np.mean(smeared))
print("nonzero:", np.count_nonzero(smeared))
print("finite:", np.count_nonzero(np.isfinite(smeared)))
"""

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
#histogram of weighted energies, not yet calibrated or smeared
hist, edges = np.histogram(
    weighted_energy[weighted_energy > 0],
    bins=1024,
    range=(0, 3000)
)
centers = (edges[:-1] + edges[1:]) / 2

#identified peaks before recalibration
peaks, properties = find_peaks(hist, prominence=np.max(hist) * 0.1)
print("Indices:", peaks)
print("Energies:", centers[peaks])

peak_mask = ((centers[peaks] > 1261) & (centers[peaks] < 1661))

#identify peaks in spectrum
k40_candidates = peaks[peak_mask]

#shifted K-40 mode
k40_peak = k40_candidates[np.argmax(hist[k40_candidates])]

peak_energy = centers[k40_peak]

print("K-40 mode, uncorrected:", peak_energy)

widths, width_heights, lower_bound, upper_bound = peak_widths(hist, peaks, rel_height=0.5)

weighted_k40_width = widths[peak_mask]
weighted_lower_bound = lower_bound[peak_mask]
weighted_upper_bound = upper_bound[peak_mask]

bin_width = edges[1] - edges[0]

weighted_k40_fwhm = weighted_k40_width * bin_width
k40_lower_energy = edges[0] + weighted_lower_bound * bin_width
k40_upper_energy = edges[0] + weighted_upper_bound * bin_width

print("Unsmeared K-40 FWHM:", weighted_k40_fwhm[0], "keV")
print("Unsmeared FWHM lower bound:", k40_lower_energy[0], "keV")
print("Unsmeared FWHM upper bound:", k40_upper_energy[0], "keV")

#calculate centroid
centroid_window = ((centers > peak_energy - (0.5*weighted_k40_fwhm[0])) & (centers < peak_energy + (0.5*weighted_k40_fwhm[0])))

centroid = np.average(centers[centroid_window], weights=hist[centroid_window])

print("K-40 centroid, uncalibrated/unsmeared:", centroid, "keV")

cal_factor = 1460.8/centroid

print("recalibration factor:", cal_factor)

#new calibrated energies and histogram, not yet smeared
calibrated_energies = weighted_energy[weighted_energy > 0] * cal_factor

#apply gaussian smearing
k40_smeared = np.zeros_like(calibrated_energies)
mask = calibrated_energies > 0
k40_smeared[mask] = smear_energy(calibrated_energies[mask])

#calibrated and smeared histogram
calibrated_hist, edges = np.histogram(k40_smeared, bins=1024, range=(0,3000))
centers = (edges[:-1] + edges[1:]) / 2

#search again for recalibrated peaks
calibrated_peaks, properties = find_peaks(calibrated_hist, prominence=np.max(calibrated_hist) * 0.1)

print("calibrated indices:", calibrated_peaks)
print("calibrated energies:", centers[calibrated_peaks])

cal_peak_mask = ((centers[calibrated_peaks] > 1400) & (centers[calibrated_peaks] < 1500))

#print("Calibrated modes:", centers[calibrated_peaks])
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
#print("FWHM %:", (k40_fwhm[0] / cal_k40_peak) * 100, "%")

#calibrated smeared centroid
cal_centroid_window = ((centers > 1460.8 - (0.5*k40_fwhm[0])) & (centers < 1460.8 + (0.5*k40_fwhm[0])))
cal_k40_centroid = np.average(centers[cal_centroid_window], weights=calibrated_hist[cal_centroid_window])

print("Calibrated K-40 centroid:", cal_k40_centroid, "keV")

#calculate peak area by ROI integration
roi = ((centers > cal_k40_centroid - k40_fwhm[0]) & (centers < cal_k40_centroid + k40_fwhm[0]))

peak_area = np.sum(calibrated_hist[roi])

print("Peak area:", peak_area)

efficiency = peak_area/100000000

print("Efficiency:", efficiency)

plt.hist(
    k40_smeared[k40_smeared > 1],
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
