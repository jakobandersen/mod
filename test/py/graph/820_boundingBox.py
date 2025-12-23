strs = [
	"[H][H]",
	"O",
	"[CH4]",
	"[CH4.]",
	"[CH4+]",
	"[13CH4]",
	"CCC",
]

for s in strs:
	Graph.fromSMILES(s)

for gr in True, False:
	post.summaryChapter(f"Graphviz = {gr}")
	for g in inputGraphs:
		post.summarySection(g.smiles)
		for h in True, False:
			for c in True, False:
				for i in True, False:
					for chg in True, False:
						for iso in True, False:
							g.name = f"{g.smiles}, h={h}, c={c}, i={i}, chg={chg}, iso={iso}"
							p = GraphPrinter()
							p.withGraphvizCoords = gr
							p.collapseHydrogens = h
							p.simpleCarbons = c
							p.withIndex = i
							p.raiseCharges = chg
							p.raiseIsotopes = iso
							g.print(p)
