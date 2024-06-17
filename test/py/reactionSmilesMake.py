def getReactionSmiles(dg):
	origSmiles = {}
	for v in dg.vertices:
		s = v.graph.smilesWithIds
		s = s.replace(":", ":o")
		origSmiles[v.graph] = s

	res = {}
	for e in dg.edges:
		vms = DGVertexMapper(e)
		eductSmiles = [origSmiles[g] for g in vms.left]

		for ev in vms.left.vertices:
			s = eductSmiles[ev.graphIndex]
			s = s.replace(f":o{ev.vertex.id}]", f":{ev.id}]")
			eductSmiles[ev.graphIndex] = s

		strs = set()
		for vm in DGVertexMapper(e):
			productSmiles = [origSmiles[g] for g in vms.right]
			for ev in vms.left.vertices:
				pv = vm.map[ev]
				s = productSmiles[pv.graphIndex]
				s = s.replace(f":o{pv.vertex.id}]", f":{ev.id}]")
				productSmiles[pv.graphIndex] = s
			left = ".".join(eductSmiles)
			right = ".".join(productSmiles)
			s = f"{left}>>{right}"
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
