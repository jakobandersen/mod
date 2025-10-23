include("xxx_helpers.py")

# importantly, all the objects must have the same .id to really test it
g = Graph.fromSMILES("O")
r = Rule.fromDFS("[A]>>[B]")
dg = DG()
d = Derivation()
d.left = d.right = [g]
dg.build().addDerivation(d)
flow = hyperflow.Model(dg)

l = [g, r, dg, flow]
for a in l:
	for b in l:
		print(f"{a}     {b}     {id(a) == id(b)}")
		assert a.id == b.id
		if id(a) == id(b):
			assert a == b
			assert b == a
			assert not (a < b)
			assert not (a > b)
		else:
			assert a != b
			assert b != a
			fail(lambda: a < b, "'<' not supported", err=TypeError, isSubstring=True)
			fail(lambda: b < a, "'<' not supported", err=TypeError, isSubstring=True)
