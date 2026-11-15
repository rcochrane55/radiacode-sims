import numpy as np

energies = np.arange(50, 2800 + 50, 50)

n_events = 10000000

with open("energyscanner.mac", "w") as f:
    f.write("/run/initialize\n")
    f.write("/run/numberOfThreads 16\n\n")

    f.write("/gps/particle gamma\n")
    f.write("/gps/ang/type iso\n\n")

    for energy in energies:
        f.write(
            f"/analysis/setFileName "
            f"energy_scan/euxenite_{energy:04d}keV.root\n"
        )

        f.write(f"/gps/energy {energy} keV\n")

        f.write(f"/gps/energy {energy} keV\n")

        f.write(f"/run beamOn {n_events}\n\n")