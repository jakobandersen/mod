include("../xxx_helpers.py")

dgOther = DG()
dgOther.build().addAbstract("A -> B")
eOther = next(iter(dgOther.edges))
vOther = next(iter(dgOther.vertices))

dg = DG()
with dg.build() as b:
	b.addAbstract("""
		A -> B
	""")

	fail(lambda: DGPrintData(dg), "Can not create print data. The DG is not locked yet.")
e = next(iter(dg.edges))
vTail = next(iter(e.sources))
vHead = next(iter(e.targets))

d = DGPrintData(dg)
assert d.dg == dg

fail(lambda: d.makeDuplicate(DG.HyperEdge(), 1), "The hyperedge is null.")
fail(lambda: d.makeDuplicate(eOther, 1),
	"The hyperedge does not belong to the derivation graph this data is for.")
fail(lambda: d.makeDuplicate(e, 0), "Duplicate already exists.")

fail(lambda: d.removeDuplicate(DG.HyperEdge(), 1), "The hyperedge is null.")
fail(lambda: d.removeDuplicate(eOther, 1),
	"The hyperedge does not belong to the derivation graph this data is for.")
fail(lambda: d.removeDuplicate(e, 42), "Duplicate does not exist.")

fail(lambda: d.reconnectSource(DG.HyperEdge(), 0, vTail, 0),
	"The hyperedge is null.")
fail(lambda: d.reconnectSource(eOther, 0, vTail, 0),
	"The hyperedge does not belong to the derivation graph this data is for.")
fail(lambda: d.reconnectSource(e, 0, DG.Vertex(), 0),
	"The vertex is null.")
fail(lambda: d.reconnectSource(e, 0, vOther, 0),
	"The vertex does not belong to the derivation graph this data is for.")
fail(lambda: d.reconnectSource(e, 0, vHead, 0),
	"The vertex is not a source of the given hyperedge.")
fail(lambda: d.reconnectSource(e, 42, vTail, 42),
	"Hyperedge duplicate does not exist.")

fail(lambda: d.reconnectTarget(DG.HyperEdge(), 0, vHead, 0),
	"The hyperedge is null.")
fail(lambda: d.reconnectTarget(eOther, 0, vHead, 0),
	"The hyperedge does not belong to the derivation graph this data is for.")
fail(lambda: d.reconnectTarget(e, 0, DG.Vertex(), 0),
	"The vertex is null.")
fail(lambda: d.reconnectTarget(e, 0, vOther, 0),
	"The vertex does not belong to the derivation graph this data is for.")
fail(lambda: d.reconnectTarget(e, 0, vTail, 0),
	"The vertex is not a target of the given hyperedge.")
fail(lambda: d.reconnectTarget(e, 42, vHead, 42),
	"Hyperedge duplicate does not exist.")



