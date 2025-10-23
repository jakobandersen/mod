O = Graph.fromSMILES("O")
g = UnionGraph([O, O])
for v in g.vertices:
	if v.atomId != AtomIds.O:
		continue
	v.printStereo()
