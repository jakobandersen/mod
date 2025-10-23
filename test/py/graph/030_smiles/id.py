include("common.py")

Graph.fromSMILES("[H][H]")
Graph.fromSMILES("CNOPS")
for a in inputGraphs:
	print("-" * 60)
	print("SMILES:", a.smiles)
	print("WithId:", a.smilesWithIds)
