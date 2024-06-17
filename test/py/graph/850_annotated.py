g = Graph.fromSMILES("OCC(=O)C(N)C(=N)C(S)C(=S)")

p = GraphPrinter()
p.setMolDefault()
fs = g.print(p)
fTex = fs[0][:-3] + "tex"

post.summarySection("Annotated")
post.summaryInput(fTex)
s = "\\begin{tikzpicture}[remember picture, overlay]\n"
def makeTikzID(v):
	return f"\\modIdPrefix v-{v.id}"
for v in g.vertices:
	# we are hiding the hydrogens, so don't annotate those
	if v.atomId == AtomIds.H and v.charge == 0:
		continue
	s += f"\\draw[color=red] ({makeTikzID(v)}) circle (1em);\n"
	s += f"\\node[at=({makeTikzID(v)}.south east), anchor=north west, inner sep=0] {{\\tiny $\\delta 2.1$}};\n"
s += "\\end{tikzpicture}\n"
post.summaryRaw(s)
