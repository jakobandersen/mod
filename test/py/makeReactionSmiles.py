def getReactionSmiles(dg):
	origSmiles = {}
	# change the class labels for every atom
	for v in dg.vertices:
		s = v.graph.smilesWithIds
		s = s.replace(":", ":o")
		origSmiles[v.graph] = s


	res = {}
	for e in dg.edges:
		maps = DGVertexMapper(e)

		# replace the original local IDs with global IDs
		eductSmiles = [origSmiles[g] for g in maps.left]
		for v in maps.left.vertices:
			s = eductSmiles[v.graphIndex]
			s = s.replace(":o%d]" % v.vertex.id, ":%d]" % v.id)
			eductSmiles[v.graphIndex] = s

		strs = set()
		for r in maps:
			m = r.map
			productSmiles = [origSmiles[g] for g in maps.right]
			for vLeft in maps.left.vertices:
				vRight = m[vLeft]
				s = productSmiles[vRight.graphIndex]
				s = s.replace(":o%d]" % vRight.vertex.id, ":%d]" % vRight.id)
				productSmiles[vRight.graphIndex] = s
			left = ".".join(eductSmiles)
			right = ".".join(productSmiles)
			s = "%s>>%s" % (left, right)
			assert ":o" not in s
			strs.add(s)
		res[e] = list(sorted(strs))
	return res

include("formoseCommon/grammar.py")

dg = DG(graphDatabase=inputGraphs)
dg.build().execute(addSubset(inputGraphs) >> inputRules >> inputRules)
dg.print()

res = getReactionSmiles(dg)
for e in dg.edges:
	print(e)
	for s in res[e]:
		print("\t", s)
