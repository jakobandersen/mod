include("xx0_helpers.py")

dgOther = DG()
dgOther.build().addAbstract("A -> B")
eOther = next(iter(dgOther.edges))
vAOther = next(iter(eOther.sources))
vBOther = next(iter(eOther.targets))
assert dgOther.findEdge([vAOther], [vBOther]) == eOther

g = smiles('O')
v = DG.Vertex()

dg =  DG()

msg = "The DG neither has an active builder nor is locked yet."
fail(lambda: dg.findVertex(g), msg)
fail(lambda: dg.findEdge([v], [v]), msg)
fail(lambda: dg.findEdge([g], [g]), msg)

def active():
	assert dg.findVertex(g) == DG.Vertex()
	assert dg.findEdge([vA], [vB]) == e
	fail(lambda: dg.findEdge([v], []), "Source vertex is null.")
	fail(lambda: dg.findEdge([], [v]), "Target vertex is null.")
	fail(lambda: dg.findEdge([g], []), "Source vertex is null.")
	fail(lambda: dg.findEdge([], [g]), "Target vertex is null.")
	fail(lambda: dg.findEdge([vAOther], []),
		f"Source vertex does not belong to this derivation graph: {vAOther}")
	fail(lambda: dg.findEdge([], [vBOther]),
		f"Target vertex does not belong to this derivation graph: {vBOther}")

	fail(lambda: dg.findVertex(None), "The graph is a null pointer.")
	fail(lambda: dg.findEdge([None], [g]), "Incompatible Data Type", err=TypeError)
	fail(lambda: dg.findEdge([g], [None]), "Incompatible Data Type", err=TypeError)
	fail(lambda: dg.findEdge([g, None], [g]), "Source graph is a null pointer.")
	fail(lambda: dg.findEdge([g], [g, None]), "Target graph is a null pointer.")

b = dg.build()
b.addAbstract("A -> B")
e = next(iter(dg.edges))
vA = next(iter(e.sources))
vB = next(iter(e.targets))
# Active builder, not locked
active()

del b
# Locked, no active builder
active()
