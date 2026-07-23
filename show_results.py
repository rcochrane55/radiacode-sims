import uproot
import numpy as np
import matplotlib.pyplot as plt

fileName = "rootOutput.root"

edep = []
f = uproot.open(fileName)
tree = f['t']
print(tree.keys())
e  = f['t']['SmearedEdep'].array()
e = f["t"]["SmearedEdep"].array(library="np") * 1000.0  # convert to numpy array
x = f["t"]["FirstX"].array(library="np")
y = f["t"]["FirstY"].array(library="np")
z = f["t"]["FirstZ"].array(library="np")
# escludi quando gli ene dep == 0 (gamma uscito da NaI)
edep.append(e[e>0])

counts, edges = np.histogram(e, bins=3000, range=(0, 3000))

centers = (edges[:-1] + edges[1:]) / 2

# Save to CSV
np.savetxt(
    "energy_spectrum.csv",
    np.column_stack((centers, counts)),
    delimiter=",",
    header="Energy_keV,Counts",
    comments="")

"""
energy = e[e>0]

peak_energy = 1460.822 #keV
fwhm_percent = 5.0 #resolution %

fwhm = peak_energy * fwhm_percent / 100.0
sigma = fwhm / (2.0 * np.sqrt(2.0 * np.log(2.0)))

print(f"FWHM = {fwhm:.2f} keV, Sigma = {sigma:.2f} keV")

n_sigma = 2.5

lower_bound = 1410.0
upper_bound = peak_energy + n_sigma * sigma

print(f"Energy window: {lower_bound:.2f} keV - {upper_bound:.2f} keV")

peak_events = np.sum((energy >= lower_bound) & (energy <= upper_bound))

print(f"Peak events: {peak_events}")

n_emitted = 10000000
efficiency = peak_events / n_emitted

print(f"Detection efficiency: {efficiency:.6f}")
"""

# plot
plt.hist(edep,1024, histtype='step', label=fileName)
plt.yscale('log')
plt.xlabel('Energy [keV]')
plt.legend(loc='upper left')
plt.show()

plt.hist(y, bins=50)
plt.xlabel("First Interaction Y Position [mm]")
plt.show()
# plot peak fit
#plt.figure(figsize=(8,5))

#plt.hist(energy,bins=500,range=(1200,1700))

"""plt.axvline(x=lower_bound, color='r', linestyle='--', label=f'Lower Bound ({lower_bound:.2f} keV)')
plt.axvline(x=upper_bound, color='g', linestyle='--', label=f'Upper Bound ({upper_bound:.2f} keV)')
plt.axvline(peak_energy, color='b', linestyle='--', label=f'Peak Energy ({peak_energy:.2f} keV)')
plt.xlabel('Energy [keV]')
plt.ylabel('Counts')
plt.legend()
plt.show()"""
