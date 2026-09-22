import csv

input = "emission_spectrum_data.csv"
output = "emission_gps.dat"

with open(input, "r", newline="") as fin, open(output, "w") as fout:

    reader = csv.DictReader(fin)

    for row in reader:
        energy_eV = float(row["photon_energy_eV"])
        intensity = float(row["relative_intensity"])

        energy_MeV = energy_eV * 1e-6

        fout.write(f"{energy_MeV:.8e} {intensity:.8e}\n")
    