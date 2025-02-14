include("xx0_helpers.py")

def check(dg, res, exp):
	for name in exp:
		g = res.getGraph(name)
		assert g, name
		assert g.numVertices == 1
		vName = next(iter(g.vertices)).stringLabel
		assert vName == name, (vName, name)
	for v in dg.vertices:
		g = v.graph
		if g.numVertices != 1:
			continue
		name = next(iter(g.vertices)).stringLabel
		gr = res.getGraph(name)
		if not gr:
			assert name in exp, (name, g.name)
			continue
		assert g == gr, (g.name, gr.name)

dg = DG()
r = dg.build().addAbstract("a -> b")
check(dg, r, "ab")

dg = DG()
with dg.build() as b:
	r = b.addAbstract("a -> b")
	check(dg, r, "ab")

dg = DG()
with dg.build() as b:
	r1 = b.addAbstract("a -> b")
	check(dg, r1, "ab")
	r2 = b.addAbstract("a -> b")
	check(dg, r2, "ab")
	assert r1.getGraph("a") == r2.getGraph("a")
	assert r1.getGraph("b") == r2.getGraph("b")

gaPre = Graph.fromDFS("[a]", name="a pre")
dg = DG(graphDatabase=[gaPre])
r = dg.build().addAbstract("a -> b")
check(dg, r, "ab")
assert r.getGraph("a") == gaPre
assert gaPre.name == "a pre"
