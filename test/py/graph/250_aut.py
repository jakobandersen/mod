if True:
	a = Graph.fromSMILES("C(=CC(=O)O)C(=O)O", name="Fumarate")
	p = GraphPrinter()
	p.withIndex = True
	a.print(p)
	g = a.aut()
	print("Aut:", g)
	print("Perm:", g.gens[0])
	print("Num gens:", len(g.gens))
	for p in g.gens:
		print(p)
	v = next(a.vertices.__iter__())
	print("Get input: ", v)
	print("Get output:", g.gens[1][v])

a = Graph.fromSMILES("CC1=C(CC2=C(C)N=S(=O)(O)N=C2C)C(=NS(=O)(=N1)O)C")
a = Graph.fromSMILES("CC(C)C[C@@H](C(=O)O)N", name="Leucine")
p = GraphPrinter()
p.setMolDefault()
p.collapseHydrogens = False
f = a.print(p)
f = f[0][:-3]
fInc = f + "tex"
fName = "out/aut.tex"
pairs = set()
with open(fName, "w") as f:
	f.write("\\input{%s}\n" % fInc)
	f.write("\\begin{tikzpicture}[overlay, remember picture]\n")
	for p in a.aut().gens:
		for v in a.vertices:
			if v.stringLabel == "H" and v.degree == 1:
				continue
			vImg = p[v]
			if v == vImg:
				continue
			s, t = v, vImg
			if s.id > t.id:
				s, t = t, s
			if (s.id, t.id) in pairs:
				continue
			pairs.add((s.id, t.id))
			f.write("\\path[modRCMatchEdge] (v-%d) to[bend left=15] (v-%d);\n" % (v.id, vImg.id))
	f.write("\\end{tikzpicture}\n")
post.summaryInput(fName)
