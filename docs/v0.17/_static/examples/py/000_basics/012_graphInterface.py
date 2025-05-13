g = graphDFS("[R]{x}C([O-])CC=O")
print("|V| =", g.numVertices)
print("|E| =", g.numEdges)
for v in g.vertices:
	print("v%d: label='%s'" % (v.id, v.stringLabel), end="")
	print("\tas molecule: atomId=%d, charge=%d" % (v.atomId, v.charge), end="")
	print("\tis oxygen?", v.atomId == AtomIds.Oxygen)
	print("\td(v) =", v.degree)
	for e in v.incidentEdges:
		print("\tneighbour:", e.target.id)
for e in g.edges:
	print("(v%d, v%d): label='%s'" % (e.source.id, e.target.id, e.stringLabel), end="")
	try:
		bt = str(e.bondType)
	except LogicError:
		bt = "Invalid"
	print("\tas molecule: bondType=%s" % bt, end="")
	print("\tis double bond?", e.bondType == BondType.Double)
# rst-name: Graph Interface
# rst: Graph objects have a full interface to access individual vertices and
# rst: edges.
# rst: The attributes of vertices and edges can be accessed both in their raw
# rst: string form, and as their chemical counterpart (if they have one).
