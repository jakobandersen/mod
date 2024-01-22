def printReactionWithAAM(e, m, *, withH=False, withDuplicates=False):
	# interface constraints
	assert e, "The hyperedge must be non-null."
	assert not withDuplicates, "TODO: doesn't work yet, nag Jakob for a fix."

	dg = e.dg
	# check that the library is doing what we want
	assert [v.graph for v in e.sources] == list(m.domain)
	assert [v.graph for v in e.targets] == list(m.codomain)
	srcs = list(e.sources)
	tars = list(e.targets)
	d = DGPrintData(dg)
	if withDuplicates:
		# Make each source and target have a unique duplicate ID int he print data.
		# Note: we need the duplicate numbers later for printing the extra Latex.
		i = 1
		for v in e.sources:
			d.reconnectSource(e, 0, v, i)
			i += 1
		for v in e.targets:
			d.reconnectTarget(e, 0, v, i)
			i += 1
	p = DGPrinter()
	p.withInlineGraphs = True  # print the DG Latex where molecules are \input'ed
	if withH:
		p.graphPrinter.collapseHydrogens = False
	p.pushEdgeVisible(lambda eCand: e == eCand)  # only print our candidate edge
	# Print it to get the Latex file, the actual depiction is unused.
	# TODO: this is sort of a hax, MØD will be changed to directly print a DG with
	#       extra Latex.
	fDG = dg.print(p, d)  

	# write an extra Latex file with the AAM as an overlay
	fName = makeUniqueFilePrefix() + "vm.tex"
	with open(fName, "w") as f:
		f.write("\\begin{tikzpicture}[overlay, remember picture]\n")
		for vSrc in m.domain.vertices:
			if not withH and vSrc.atomId == AtomIds.H:
				continue
			vTar = m[vSrc]
			assert vTar, "Atom is being destroyed"
			srcDup = 0
			tarDup = 0
			sSrc = "v-{}-{}-v-{}".format(srcs[vSrc.graphIndex].id, srcDup, vSrc.vertex.id)
			sTar = "v-{}-{}-v-{}".format(tars[vTar.graphIndex].id, tarDup, vTar.vertex.id)
			f.write(f"\\draw[red, dashed, ->, >=stealth'] ({sSrc}) to[bend left=10] ({sTar});\n")
		f.write("\\end{tikzpicture}")
	# TODO: the following is how we get the depicition, but in the future it will be
	#       simplified to allow direct printing of a DG with extra Latex.
	# input the DG Latex in the summary inline
	post.summaryInput(fDG[0][:-3] + "tex")
	# and then input the AAM Latex
	post.summaryInput(fName)


def test():
	# some semi-arbitrary example to ensure test of some specific cases
	hcn = Graph.fromSMILES("C#N")
	water = Graph.fromSMILES("O")
	ammonia = Graph.fromSMILES("N")
	res = Graph.fromSMILES("N#CC=O")

	r = Rule.fromDFS("[H]2[C]1.[C]20#[N]21.[H]31[O]30[H]32>>[C]1[C]20=[O]30.[H]2[N]21([H]31)([H]32)")
	dg = DG(graphDatabase=inputGraphs)
	dg.build().execute(addSubset(hcn, water) >> r)

	e = next(iter(dg.edges))
	maps = DGVertexMapper(e)
	m = maps[0].map
	printReactionWithAAM(e, m)
	printReactionWithAAM(e, m, withH=True)

test()
