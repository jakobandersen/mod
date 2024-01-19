include("../xxx_helpers.py")

import math


def check(numH, expCount, isoG=True, leftLimit=None, rightLimit=None, silent=False, verbosity=0):
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
		Rule.fromDFS("[L1].[A]1[H]2.[L2]>>[R1].[B]1[H]2.[R2]"))
	assert dg.numEdges == 1
	e = next(iter(dg.edges))

	header = f"#H={numH}, isoG={isoG}, leftLimit={leftLimit}, rightLimit={rightLimit}"
	print(header)
	if leftLimit is None:
		leftLimit = 2**30
	if rightLimit is None:
		rightLimit = 2**30
	maps = DGVertexMapper(e, upToIsomorphismG=isoG,
		leftLimit=leftLimit, rightLimit=rightLimit,
		verbosity=verbosity)
	if not silent:
		for res in maps:
			r = res.rule
			m = res.map
			print("  ", [
				"{}:{:2s}".format(v.id, v.stringLabel)
				for v in m.domain.vertices
			])
			print("->", [
				"{}:{:2s}".format(
					m[v].id if m[v] else ".",
					m[v].stringLabel if m[v] else ".")
				for v in m.domain.vertices
			])

			print("  ", [
				"{}:{:2s}".format(v.id, v.stringLabel)
				for v in m.codomain.vertices
			])
			print("->", [
				"{}:{:2s}".format(
					m.inverse(v).id if m.inverse(v) else ".",
					m.inverse(v).stringLabel if m.inverse(v) else ".")
				for v in m.codomain.vertices
			])
			for v in m.domain.vertices:
				if m[v]:
					assert m.inverse(m[v]) == v, (m, m[v], m.inverse(m[v]))
	print(f"count={len(maps)}")
	assert len(maps) == expCount, (len(maps), expCount)


print("="*80)
check(3, 1,
	isoG=True,  leftLimit=1,    rightLimit=1,    silent=True)

print("="*80)
check(3, math.factorial(3),
	isoG=True,  leftLimit=1,    rightLimit=None, silent=True)
print("="*80)
check(3, math.factorial(3),
	isoG=True,  leftLimit=None, rightLimit=1,    silent=True)
print("="*80)
check(3, 3,
	isoG=False, leftLimit=1,    rightLimit=1,    silent=True)

print("="*80)
check(3, math.factorial(3)*3,
	isoG=False, leftLimit=None, rightLimit=1,    silent=True)
print("="*80)
check(3, math.factorial(3)*3,
	isoG=False, leftLimit=1,    rightLimit=None, silent=True)
print("="*80)
check(3, math.factorial(3)**2 * 3,
	isoG=False, leftLimit=None, rightLimit=None, silent=True)


print("#"*80)

check(7, math.factorial(7), rightLimit=1, silent=True)

print("=" * 80)
check(9, 1, leftLimit=1, rightLimit=1)
check(9, math.factorial(9), rightLimit=1, silent=True)
