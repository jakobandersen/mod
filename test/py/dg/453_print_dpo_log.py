s = "[A]"
for i in range(3):
	s += "([H])"
A = Graph.fromDFS(s)

dg = DG()
dg.build().apply([A], Rule.fromDFS("[A]1>>[B]1"))
assert dg.numEdges == 1
e = next(iter(dg.edges))

for v in (0, 1, 9, 10, 19, 20):
	print("*" * 80)
	print("Verbosity", v)
	print("*" * 80)
	e.print(verbosity=v)
