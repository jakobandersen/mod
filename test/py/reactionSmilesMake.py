
def getReactionSmiles(dg):
	import re

	origSmiles = {}
	for v in dg.vertices:
		s = v.graph.smilesWithIds
		s = re.sub(":([0-9]+)]", ":o\\1]", s)
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
				if not pv:
					continue
				s = productSmiles[pv.graphIndex]
				s = s.replace(f":o{pv.vertex.id}]", f":{ev.id}]")
				productSmiles[pv.graphIndex] = s
			count = vms.left.numVertices
			for pv in vms.right.vertices:
				ev = vm.map.inverse(pv)
				if ev:
					continue
				s = productSmiles[pv.graphIndex]
				s = s.replace(f":o{pv.vertex.id}]", f":{count}]")
				count += 1
				productSmiles[pv.graphIndex] = s
			left = ".".join(eductSmiles)
			right = ".".join(productSmiles)
			s = f"{left}>>{right}"
			assert ":o" not in s
			strs.add(s)
		res[e] = list(sorted(strs))
	return res


def test(header, f):
	print("=" * 80)
	print(header)
	print("=" * 80)
	dg = DG(graphDatabase=inputGraphs)
	f(dg.build())
	dg.print()

	res = getReactionSmiles(dg)
	for e in dg.edges:
		print(e)
		for s in res[e]:
			print("\t", s)


include("formoseCommon/grammar.py")

test("Formose", lambda b: b.execute(addSubset(inputGraphs) >> inputRules >> inputRules))

# check aromatic bonds
a = Graph.fromSMILES("[C](:[H])(:[H])(:[H])")
r = Rule.fromDFS("[C]1>>[N]1")
test("Aromatic", lambda b: b.execute(addSubset(a) >> r))

# check created and deleted vertices
a = Graph.fromSMILES("[C][H]")
r = Rule.fromDFS("[C]1[H]>>[N]1[He]")
test("Created/Deleted", lambda b: b.execute(addSubset(a) >> r))
