a = Graph.fromSMILES("C")
a.print()
def customImage():
	with open("out/custom.tex", "w") as f:
		f.write("""\\begin{tikzpicture}
\\node {custom};
\\end{tikzpicture}""")
	return "out/custom"
a.image = customImage
a.imageCommand = "compileTikz \"out/custom\" \"out/custom\""
a.print()
