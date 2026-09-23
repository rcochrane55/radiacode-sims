import numpy as np
import csv

x_coords = np.linspace(-4.5, 4.5, 7)
y_coords = np.linspace(-4.5, 4.5, 7)
z_coords = np.linspace(-4.5, 4.5, 7)

n_photons = 50000

csv_headers = ["run_id", "x_mm", "y_mm", "z_mm"]

with open("map_coords.csv", "w", newline="") as coords:
    writer = csv.writer(coords)
    writer.writerow(csv_headers)

with open("generate_map.mac", "w") as f:
    f.write("/run/numberOfThreads 1\n")
    f.write("/run/verbose 2\n")
    f.write("/run/initialize\n\n")
    f.write("/gps/particle opticalphoton\n\n")

    f.write("/gps/pos/type Point\n")
    f.write("/gps/ang/type iso\n")
    f.write("/gps/polarization 0 0 0\n\n")

    f.write("/gps/ene/type Arb\n")
    f.write("/gps/ene/emspec true\n")
    f.write("/gps/ene/diffspec true\n")
    f.write("/gps/hist/type arb\n")
    f.write("/gps/hist/file emission_gps.dat\n")
    f.write("/gps/hist/inter Lin\n\n")

    run_id = 0
    for x in x_coords:
        for y in y_coords:
            for z in z_coords:
                f.write(f"/gps/pos/centre "
                f"{x} {y} {z} mm\n")
                f.write(f"/run/beamOn {n_photons}\n")
                with open("map_coords.csv", "w", newline="") as coords:
                    writer = csv.writer(coords)
                    data = [run_id, x, y, z]
                    writer.writerow(data)
                run_id += 1