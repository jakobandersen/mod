include("../xxx_helpers.py")

fail(lambda: causality.Net(None), "The derivation graph is a null pointer.")
fail(lambda: causality.Net(DG()), "The DG neither has an active builder nor is locked yet.")

dg = DG()
with dg.build():
	net = causality.Net(dg)
	assert net.dg == dg

assert str(net) == "Net{{{}}}".format(net.dg)
	
net = causality.Net(dg)
assert net.dg == dg

net.syncSize()

dg = DG()
dg.build().addAbstract("A -> B   C -> 2 D")
e1 = next(e for e in dg.edges if e.numTargets == 1)
B = next(iter(e1.targets))
assert B.graph.name == "B"
e2 = next(e for e in dg.edges if e.numTargets == 2)
D = next(iter(e2.targets))
assert D.graph.name == "D"

net = causality.Net(dg)
ps1 = net.getPostPlaces(e1)
ps2 = net.getPostPlaces(e2)
assert ps1 == [B]
assert ps2 == [D]

