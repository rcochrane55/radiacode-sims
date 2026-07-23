import uproot
import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
import os

fileName = "rootOutput.root"

def gaussian_linear_bkg(x, A, mu, sigma, m, b):
    return(A * np.exp(-(x - mu)**2 / (2 * sigma**2)) + m * x + b)

edep = []
f = uproot.open(fileName)
tree = f['t']
print(tree.keys())
e  = f['t']['SmearedEdep'].array()
e = f["t"]["SmearedEdep"].array(library="np") * 1000.0  # convert to numpy array
x = f["t"]["FirstX"].array(library="np")
y = f["t"]["FirstY"].array(library="np")
z = f["t"]["FirstZ"].array(library="np")
edep.append(e[e>0])
z_local = z + 1.2
depth = np.minimum.reduce([
    5.0 - np.abs(x),
    5.0 - np.abs(y),
    5.0 - np.abs(z_local)
])

counts, edges = np.histogram(e, bins=1024, range=(0, 3000))

centers = (edges[:-1] + edges[1:]) / 2

# Save to CSV
os.makedirs("GeneratedSpectrums", exist_ok=True)

i = 1
while os.path.exists(f"GeneratedSpectrums/spectrum{i}.csv"):
    i += 1

np.savetxt(
    f"GeneratedSpectrums/spectrum{i}.csv",
    np.column_stack((centers, counts)),
    delimiter=",",
    header="Energy_keV,Counts",
    comments=""
)

photopeak = (e >= 1410) & (e <= 1520)

bins = np.linspace(0, 5, 11)

depth_centers = []
probability = []

hist, edges = np.histogram(e, bins=120, range=(1350,1575))

allevents_centers = (edges[:-1] + edges[1:]) / 2

a_0 = hist.max()
mu_0 = 1461
sigma_0 = 30
m_0 = 0
b_0 = hist.min()

p_0 = [a_0, mu_0, sigma_0, m_0, b_0]

params, covariance = curve_fit(gaussian_linear_bkg,allevents_centers,hist, p0=p_0)

A, mu, sigma, m, b = params
bin_width = edges[1] - edges[0]
peak_area = A * sigma * np.sqrt(2*np.pi) / bin_width
print(peak_area)

def fit_peak(e):
    hist, edges = np.histogram(
        e,
        bins=225,
        range=(1350,1575)
    )

    centers = (edges[:-1] + edges[1:]) / 2

    bin_width = edges[1] - edges[0]

    p0 = [
        hist.max(),
        1460.8,
        30,
        0,
        hist.min()
    ]

    params, covariance = curve_fit(
        gaussian_linear_bkg,
        centers,
        hist,
        p0=p0
    )

    A, mu, sigma, m, b = params

    peak_area = A * sigma * np.sqrt(2*np.pi) / bin_width

    return peak_area

probability_error = []

for i in range(len(bins)-1):

    mask = (depth >= bins[i]) & (depth < bins[i+1])

    n_total = np.sum(mask)

    if n_total == 0:
        continue

    peak_area = fit_peak(e[mask])

    A_err = np.sqrt(covariance[0,0])
    sigma_err = np.sqrt(covariance[2,2])

    peak_error = peak_area * np.sqrt((A_err/A)**2 + (sigma_err/sigma)**2)

    probability_err = peak_error / n_total

    print(
        f"{bins[i]:.1f}-{bins[i+1]:.1f} mm : "
        f"{n_total:7d} interactions   "
        f"{peak_area:8.1f} peak"
    )

    depth_centers.append((bins[i] + bins[i+1]) / 2)

    probability.append(peak_area / n_total)

    probability_error.append(probability_err)

# Plot full energy spectrum (0–3000 keV, 1024 bins)
plt.figure()

plt.hist(
    e,
    bins=1024,
    range=(0, 3000),
    histtype='step',
    label=fileName
)

plt.yscale('log')
plt.xlim(0, 3000)
plt.xlabel('Energy [keV]')
plt.ylabel('Counts')
plt.legend(loc='upper left')
plt.grid(True, alpha=0.3)

plt.show()  

plt.figure()
plt.errorbar(depth_centers, probability, yerr=probability_error, fmt='o-',capsize=3)
plt.xlabel("Distance to crystal surface (mm)")
plt.ylabel("Photopeak Probability")
plt.grid(True)
plt.show()

plt.figure()

plt.hist(
    e,
    bins=120,
    range=(1350,1575),
    histtype='step',
    label='Simulation'
)

xfit = np.linspace(1350,1575,500)

plt.plot(
    xfit,
    gaussian_linear_bkg(xfit,*params),
    label='Fit'
)

plt.legend()
plt.show()
