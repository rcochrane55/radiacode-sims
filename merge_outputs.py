import pandas as pd

map_coords = pd.read_csv("map_coords.csv")
pdes = pd.read_csv("optical_pdes.csv")

merged_csv = pd.merge(map_coords, pdes, on="run_id", how="inner")

merged_csv.to_csv("optical_response_map.csv", index=False)