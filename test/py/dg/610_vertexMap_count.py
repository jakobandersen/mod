include("6xx_vertexMap_helpers.py")

import math


def check(numH, expCount, iso=True, rightLimit=2**30, silent=False, verbosity=0):
	inputGraphs[:] = []
	extra = [
		Graph.fromDFS("[L1]"),
		Graph.fromDFS("[L2]"),
	]
	s = "[A]"
	for i in range(numH):
		s += "([H])"
	A = Graph.fromDFS(s)

	dg = DG(graphDatabase=inputGraphs)
	dg.build().apply(extra + [A],
		Rule.fromDFS("[L1].[A]1([H]2)([H]3).[L2]>>[R1].[B]1([H]2)([H]3)([H]4).[R2]"))
	assert dg.numEdges == 1
	e = next(iter(dg.edges))

	header = f"#H={numH}, iso={iso}, rightLimit={rightLimit}"
	print(header)
	args = {"verbosity": verbosity}
	if iso is not None:
		args["upToIsomorphismGDH"] = iso
	if rightLimit is not None:
		args["rightLimit"] = rightLimit
	maps = DGVertexMapper(e, **args)
	checkMaps(maps, silent)
	print(f"count={len(maps)}")
	assert len(maps) == expCount, (len(maps), expCount)

# The graph G has numH amount of 'H' vertices.
# The graph L has 2 'H' vertices, and there is thus numH * (numH - 1) amount of matches.
# The graph R has 3 'H' vertices.
# The graph H thus have numH + 1 H vertices, and the amount of right-morphisms
# is thus (numH + 1)!.
# Up to isomorphism there is only a single bottom span G <- D -> H.

print("="*80)
check(4, 1,
	iso=True,  rightLimit=1,      silent=True)

print("="*80)
check(4, math.factorial(5),
	iso=True,  rightLimit=2**30,  silent=True)

print("="*80)
print("Default args")
check(4, math.factorial(5),
	iso=None,  rightLimit=None,   silent=True)
print("="*80)
check(4, math.factorial(5) * 4*3,
	iso=False, rightLimit=2**30, silent=False)

print("="*80)
check(4, 4*3,
	iso=False, rightLimit=1,      silent=True)

print("="*80)
check(4, math.factorial(5) * 4*3,
	iso=False, rightLimit=2**30,  silent=True)


print("=" * 80)
check(8, 1, rightLimit=1)
