include("../xxx_helpers.py")
post.enableInvokeMake()

dg = DG()
fail(lambda: dg.print(), "Can not create print data. The DG is not locked yet.")

with dg.build() as b:
	d = Derivation()
	d.left = [smiles("CO")]
	d.right = [smiles("CS")]
	d.rule = ruleGMLString('''rule [
		left [ node [ id 0 label "O" ] ]
		right [ node [ id 0 label "S" ] ]
	]''')
	b.addDerivation(d)
	d = Derivation()
	d.left = [graphDFS("[image1]", name="image1")]
	d.right = [graphDFS("[image2]", name="image2")]
	b.addDerivation(d)
	s = "A + hide -> B\n"
	for i in range(1, 4):
		s += "{i} A{i} -> {i} B\n".format(i=i)
	for i in range(1, 4):
		for j in range(1, 4):
			s += "{i} C{i}_{j} + D{i}_{j} -> {j} C{i}_{j} + E\n".format(i=i, j=j)
	b.addAbstract(s)
dg.print()

post.summarySection("withIndex, withShortcutEdges, labelsAsLatexMath")
p = DGPrinter()
p.graphPrinter.withIndex = True
p.withShortcutEdges = False
p.labelsAsLatexMath = False
dg.print(p)

post.summarySection("withGraphImages")
p = DGPrinter()
p.withGraphImages = False
dg.print(p)

post.summarySection("vertexVisible")
p = DGPrinter()
p.pushVertexVisible(False)
dg.print(p)
p.popVertexVisible()
p.pushVertexVisible(lambda v: v.graph.name != "hide")
dg.print(p)
p.popVertexVisible()
checkDeprecated(lambda:
	p.pushVertexVisible(lambda g, dg: g.name != "hide"))
dg.print(p)
p.popVertexVisible()

post.summarySection("edgeVisible")
p = DGPrinter()
p.pushEdgeVisible(False)
dg.print(p)
p.popEdgeVisible()
p.pushEdgeVisible(lambda e: len(e.rules) != 0)
dg.print(p)
p.popEdgeVisible()

post.summarySection("withShortcutEdgesAfterVisibility")
p = DGPrinter()
p.pushVertexVisible(lambda v: v.graph.name != "hide")
p.withShortcutEdgesAfterVisibility = True
dg.print(p)

post.summarySection("vertexLabelSep")
p = DGPrinter()
p.pushVertexLabel("vLabelConstant")
p.vertexLabelSep = " sep "
dg.print(p)

post.summarySection("vertexLabel")
p = DGPrinter()
p.pushVertexLabel("vLabelConstant")
dg.print(p)
p.popVertexLabel()
p.pushVertexLabel(lambda v: "vLabelCallback")
dg.print(p)
p.popVertexLabel()
checkDeprecated(lambda:
	p.pushVertexLabel(lambda g, dg: "vLabelCallbackDep"))
dg.print(p)
p.popVertexLabel()

post.summarySection("vertexLabelEmpty")
p = DGPrinter()
p.withGraphName = False
dg.print(p)
p.pushVertexLabel("")
p.pushVertexLabel("")
p.pushVertexLabel("1")
p.pushVertexLabel("")
p.pushVertexLabel("")
p.pushVertexLabel("2")
p.pushVertexLabel("")
p.pushVertexLabel("")
dg.print(p)

post.summarySection("edgeLabelSep")
p = DGPrinter()
p.pushEdgeLabel("eLabelConstant")
p.edgeLabelSep = " sep "
dg.print(p)

post.summarySection("edgeLabel")
p = DGPrinter()
p.pushEdgeLabel("eLabelConstant")
dg.print(p)
p.popEdgeLabel()
p.pushEdgeLabel(lambda e: "eLabelCallback")
dg.print(p)
p.popEdgeLabel()

post.summarySection("edgeLabelEmpty")
p = DGPrinter()
p.withRuleName = False
dg.print(p)
p.pushEdgeLabel("")
p.pushEdgeLabel("")
p.pushEdgeLabel("1")
p.pushEdgeLabel("")
p.pushEdgeLabel("")
p.pushEdgeLabel("2")
p.pushEdgeLabel("")
p.pushEdgeLabel("")
dg.print(p)

post.summarySection("withGraphName, withRuleName, withRuleId")
p = DGPrinter()
p.withGraphName = False
p.withRuleName = True
p.withRuleId = False
dg.print(p)

post.summarySection("withInlineGraphs")
p = DGPrinter()
p.withInlineGraphs = True
f = dg.print(p)
post.summaryInput("{}.tex".format(f[0][:-4]))
e = next(e for e in dg.edges if len(e.rules) > 0)
src = next(iter(e.sources))
tar = next(iter(e.targets))
vSrc = next(v for v in src.graph.vertices if v.stringLabel == "C")
vTar = next(v for v in tar.graph.vertices if v.stringLabel == "C")
with open("out/extra.tex", "w") as f:
	f.write(r"""
\begin{{tikzpicture}}[overlay, remember picture]
\draw[blue] (v-{dgSrc}-0-v-{gSrc}) to[bend left=45] (v-{dgTar}-0-v-{gTar});
\end{{tikzpicture}}
""".format(dgSrc=src.id, dgTar=tar.id, gSrc=vSrc.id, gTar=vTar.id))
post.summaryInput("out/extra.tex")

post.summarySection("vertexColour")
p = DGPrinter()
p.pushVertexColour("blue")
dg.print(p)
p.popVertexColour()
p.pushVertexColour(lambda v: "green")
dg.print(p)
p.popVertexColour()
checkDeprecated(lambda:
	p.pushVertexColour(lambda g, dg: "red"))
dg.print(p)
p.popVertexColour()

p = DGPrinter()
p.pushVertexColour("blue", extendToEdges=False)
dg.print(p)

post.summarySection("edgeColour")
p = DGPrinter()
p.pushEdgeColour("blue")
dg.print(p)
p.popEdgeColour()
p.pushEdgeColour(lambda v: "green")
dg.print(p)
p.popEdgeColour()

post.summarySection("rotationOverwrite")
p = DGPrinter()
p.setRotationOverwrite(45)
dg.print(p)
p.setRotationOverwrite(lambda g: -45)
dg.print(p)

post.summarySection("mirrorOverwrite")
p = DGPrinter()
p.setMirrorOverwrite(True)
dg.print(p)
p = DGPrinter()
p.setMirrorOverwrite(lambda g: True)
dg.print(p)

post.summarySection("imageOverride")
p = DGPrinter()
def customImage(v, dupNum):
	if v.graph.name != "image1":
		return ("", "")
	with open("out/custom.tex", "w") as f:
		f.write("""\\begin{tikzpicture}
\\node {custom};
\\end{tikzpicture}""")
	return ("out/custom", "compileTikz \"out/custom\" \"out/custom\"")
p.setImageOverwrite(customImage)
dg.print(p)
p.setImageOverwrite(None)
dg.print(p)

post.summarySection("graphvizPrefix")
p = DGPrinter()
p.graphvizPrefix = 'layout = "dot";'
dg.print(p)

post.summarySection("tikzpictureOption")
p = DGPrinter()
p.tikzpictureOption += ', draw=blue'
dg.print(p)
