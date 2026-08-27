import uproot
import awkward as ak
import numpy as np
import matplotlib.pyplot as plt

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

a = 0.05

weight = np.exp(-a * distance)

weighted_energy = ak.sum(stepE * weight, axis=1)

weighted_energy = ak.to_numpy(weighted_energy) * 1000

def smear_energy(E):
    a = -1252.39
    b = 8.390725
    c = -0.00205

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

plt.figure(figsize=(10, 6))
#plt.hist(raw[raw > 0], bins=3000, range=(0, 3000), histtype='step', label='Raw Energy')

plt.hist(weighted_energy[weighted_energy > 1], bins=1024, range=(0, 3000), histtype='step', label='Weighted Energy')

plt.hist(smeared[smeared > 1], bins=1024, range=(0, 3000), histtype='step', label='Weighted + Smeared Energy')
plt.xlabel('Energy (keV)')
plt.ylabel('Counts')
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
# is this tuff?
