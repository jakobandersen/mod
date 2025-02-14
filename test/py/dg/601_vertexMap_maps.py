include("6xx_vertexMap_helpers.py")

def check(title: str, gsStr: str, rStr: str, silent: bool = True):
	print("="*80)
	print(title)
	print(f"Graphs: {gsStr}, Rule: {rStr}")
	gs = Graph.fromDFSMulti(gsStr, add=False)
	r = Rule.fromDFS(rStr, add=False)
	if not silent:
		print(f"Rule vertices:")
		for v in r.vertices:
			print(f"  id={v.id}: {v.left.stringLabel if v.left else '.'} <-> {v.right.stringLabel if v.right else '.'}")
		for g in gs:
			print(f"G({g}) vertices:")
			for v in g.vertices:
				print(f"  id={v.id}: {v.stringLabel}")
	dg = DG()
	dg.build().apply(gs, r)
	assert dg.numEdges == 1
	maps = DGVertexMapper(next(iter(dg.edges)))
	checkMaps(maps, silent)


check("All same",  "[A]", "[A]1>>[B]1")
check("L larger than K", "[A][L]", "[A]1[L]>>[B]1")
check("R larger than K", "[A]", "[A]1>>[B]1[R]")
check("L and R larger than K", "[A][L]", "[A]1[L]>>[B]1[R]")

check("G larger than L larger than K",
	"[L][A][G]", "[A]1[L]>>[B]1")
check("G larger than L larger than K, and R larger than K",
	"[L][A][G]", "[A]1[L]>>[B]1[R]")
