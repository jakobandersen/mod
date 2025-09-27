include("../xxx_helpers.py")

fail(lambda: causality.Marking(None), "The Petri net is a null pointer.")

dgOther = DG()
dgOther.build().addAbstract("A -> B")
vOther = next(iter(dgOther.vertices))
eOther = next(iter(dgOther.edges))
gOther = vOther.graph

dg = DG()
dg.build().addAbstract("A -> B")
A = next(v for v in dg.vertices if v.graph.name == "A")
B = next(v for v in dg.vertices if v.graph.name == "B")
e = next(iter(dg.edges))
gA = A.graph

net = causality.Net(dg)
m = causality.Marking(net)

assert(m.net == net)

m.syncSize()

assert str(m) == "Marking{}"
assert m[A] == 0
assert m[B] == 0
assert m.numTokens == 0

fail(lambda: m.add(DG.Vertex(), 0), "Can not add tokens to a null vertex.")
fail(lambda: m.add(vOther, 0), "The vertex does not belong to the underlying derivation graph.")
fail(lambda: m.add(None, 0), "The graph is a null pointer.")
fail(lambda: m.add(gOther, 0), "Can not find vertex with the graph in the underlying derivation graph.")
fail(lambda: m.add( A, -1), "Can not add a negative token amount, use remove() instead.")
fail(lambda: m.add(gA, -1), "Can not add a negative token amount, use remove() instead.")

m.add( A, 0)
m.add(gA, 0)
assert str(m) == "Marking{}"
m.add(A, 1)
assert str(m) == "Marking{A: 1}"
assert m[ A] == 1
assert m[gA] == 1
m.add(gA, 1)
assert m[ A] == 2
assert m[gA] == 2
assert m[ B] == 0
assert m.numTokens == 2

fail(lambda: m.remove(DG.Vertex(), 0), "Can not remove tokens from a null vertex.")
fail(lambda: m.remove(vOther, 0), "The vertex does not belong to the underlying derivation graph.")
fail(lambda: m.remove(None, 0), "The graph is a null pointer.")
fail(lambda: m.remove(gOther, 0), "Can not find vertex with the graph in the underlying derivation graph.")
fail(lambda: m.remove( A, -1), "Can not remove a negative token amount, use add() instead.")
fail(lambda: m.remove(gA, -1), "Can not remove a negative token amount, use add() instead.")
fail(lambda: m.remove(A, 100), "Too few tokens left on the place.")

m.remove(A, 1)
assert str(m) == "Marking{A: 1}"
m.remove(gA, 1)
assert str(m) == "Marking{}"

fail(lambda: m[DG.Vertex()], "Can not get token count for a null vertex.")
fail(lambda: m[vOther], "The vertex does not belong to the underlying derivation graph.")
fail(lambda: m[None], "The graph is a null pointer.")
fail(lambda: m[gOther], "Can not find vertex with the graph in the underlying derivation graph.")

fail(lambda: m.getEmptyPostPlaces(DG.HyperEdge()), "Can not get empty post places for null edge.")
fail(lambda: m.getEmptyPostPlaces(eOther), "The edge does not belong to the underlying derivation graph.")

fail(lambda: m.isEnabled(DG.HyperEdge()), "Can not check if a null edge is enabled.")
fail(lambda: m.isEnabled(eOther), "The edge does not belong to the underlying derivation graph.")

fail(lambda: m.fire(DG.HyperEdge()), "Can not fire a null edge.")
fail(lambda: m.fire(eOther), "The edge does not belong to the underlying derivation graph.")
fail(lambda: m.fire(e), "The edge is not enabled.")


m = causality.Marking(net)
assert m.getAllEnabled() == []
assert m.getNonZeroPlaces() == []
assert m.getEmptyPostPlaces(e) == [B]
assert not m.isEnabled(e)
m.add(A, 1)
assert m.getAllEnabled() == [e]
assert m.getNonZeroPlaces() == [A]
assert m.getEmptyPostPlaces(e) == [B]
assert m.isEnabled(e)
m.fire(e)
assert m.getAllEnabled() == []
assert m.getNonZeroPlaces() == [B]
assert m.getEmptyPostPlaces(e) == []
assert not m.isEnabled(e)


# str() changed size
####################
dg = DG()
with dg.build() as b:
	b.addAbstract("A -> B")
	net = causality.Net(dg)
	m = causality.Marking(net)
	m.add(next(v for v in dg.vertices if v.graph.name == "B"), 1)
	assert str(m) == "Marking{B: 1}"
	b.addAbstract("B -> C")
	assert str(m) == "Marking{B: 1}"


# ==
###########
dg = DG()
dgOther = DG()
dgOther.build()
with dg.build() as b:
	net = causality.Net(dg)
	net2 = causality.Net(dg)
	netOther = causality.Net(dgOther)

	m1 = causality.Marking(net)
	m2 = causality.Marking(net)
	mOther = causality.Marking(netOther)

	assert m1 == m2
	assert m1 != mOther
	assert m2 != mOther
	assert not (m1 == mOther)
	assert not (m2 == mOther)
