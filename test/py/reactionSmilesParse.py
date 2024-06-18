def parseReactionSmiles(line: str) -> Rule:
	sLeft, sRight = line.split(">>")
	ssLeft = sLeft.split(".")
	ssRight = sRight.split(".")
	mLeft = [smiles(s, add=False) for s in ssLeft]
	mRight = [smiles(s, add=False) for s in ssRight]
	def printGraph(g):
		extFromInt = {}
		for iExt in range(g.minExternalId, g.maxExternalId + 1):
			v = g.getVertexFromExternalId(iExt)
			if not v.isNull():
				extFromInt[v] = iExt
		s = ""
		for v in g.vertices:
			assert v in extFromInt
			s += '\t\tnode [ id %d label "%s" ]\n' % (extFromInt[v], v.stringLabel)
		for e in g.edges:
			s += '\t\tedge [ source %d target %d label "%s" ]\n' % (extFromInt[e.source], extFromInt[e.target], e.stringLabel)
		return s
	s = "rule [\n\tleft [\n"
	for m in mLeft:
		s += printGraph(m)
	s += "\t]\n\tright [\n"
	for m in mRight:
		s += printGraph(m)
	s += "\t]\n]\n"
	return Rule.fromGMLString(s, add=False)


strs = [
	"[C:8]([C:7]1[C:0]([H:11])=[C:1]([C:2]([H:13])=[C:3]([C:4]=1[O:5][C:6]([H:17])([H:16])[H:15])[H:14])[H:12])([O-:10])=[O:9]>>[C:0]1([C-:7]=[C:4]([C:3]([H:14])=[C:2]([C:1]=1[H:12])[H:13])[O:5][C:6]([H:15])([H:16])[H:17])[H:11].[C:8](=[O:10])=[O:9]",
	"[C:0]1([C-:7]=[C:4]([C:3]([H:11])=[C:2]([C:1]=1[H:9])[H:10])[O:5][C:6]([H:14])([H:13])[H:12])[H:8]>>[C:1]1([C:2]([H:10])=[C:3]([C:4]([O:5][C-:6]([H:12])[H:13])=[C:7]([C:0]=1[H:8])[H:14])[H:11])[H:9]",
	"[C:1]1([C:2]([H:10])=[C:3]([C:4]([O:5][C-:6]([H:14])[H:13])=[C:7]([C:0]=1[H:8])[H:12])[H:11])[H:9]>>[C:3]1([C-:4]=[C:7]([C:0]([H:8])=[C:1]([C:2]=1[H:10])[H:9])[H:12])[H:11].[C:6]([H:13])([H:14])=[O:5]",
]
for s in strs:
	r = parseReactionSmiles(s)
	p = GraphPrinter()
	p.setReactionDefault()
	p.withIndex = True
	r.print(p)
