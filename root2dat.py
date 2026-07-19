#!/usr/bin/env python3

# Valerio, 6 ottobre 2020
# Script per trasformare i dati nel TTree di ROOT
# in un file di testo (a colonne)
#
# Uso:
#
# ./root2dat [ROOTFILE]
#
#


import uproot
import numpy as np
import sys
import os
# check argument, and file existence
if len(sys.argv)<2:
	print('Error, no argument.\nUsage:')
	print('./root2dat.py ROOTFILE ')
	sys.exit(1)

if not os.path.exists(sys.argv[1]):
	print("Error, input file",sys.argv[1],"not found.")
	sys.exit(1)

# set the output filename and check if already exists
outfile = sys.argv[1][:-5] + '.dat' 
if os.path.exists(outfile):
	answer = input("Output file %s already exists: overwrite? [N|y] " % outfile)
	if (answer == 'y') | (answer == 'Y') :
		pass
	else:
		print('script stopped.')
		sys.exit(1)



# open the root file with uproot
f = uproot.open(sys.argv[1]) 
edep = f['t']['Edep'].array()
# edep2 = edep[np.isfinite(edep)]
np.savetxt(outfile, edep*1000, fmt='%.1f', )
# print('Before filtering size =',edep.size,', after ',(edep2[edep2>0]).size,'.')
print("File",outfile,"created.")	
