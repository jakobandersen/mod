include("xx0_helpers.py")

dg =  DG()
d1 = Derivations()
d1.left = [smiles("OCC=O")]
d1.right = [smiles("OC=CO")]
d2 = Derivations()
d2.left = d1.right
d2.right = d1.left
with dg.build() as b:
	e1 = b.addDerivation(d1)
	assert dg.numVertices == 2
	assert dg.numEdges == 1
	assert not e1.inverse

	e2 = b.addDerivation(d2)
	assert dg.numVertices == 2
	assert dg.numEdges == 2
	assert e1.inverse == e2
	assert e2.inverse == e1

fail(lambda: DG.Vertex().id, "Can not get ID on a null vertex.")
fail(lambda: DG.Vertex().dg, "Can not get DG on a null vertex.")
fail(lambda: DG.Vertex().inDegree, "Can not get in-degree on a null vertex.")
fail(lambda: DG.Vertex().inEdges, "Can not get in-edges on a null vertex.")
fail(lambda: DG.Vertex().outDegree, "Can not get out-degree on a null vertex.")
fail(lambda: DG.Vertex().outEdges, "Can not get out-edges on a null vertex.")
fail(lambda: DG.Vertex().graph, "Can not get graph on a null vertex.")

fail(lambda: DG.HyperEdge().id, "Can not get ID on a null hyperedge.")
fail(lambda: DG.HyperEdge().dg, "Can not get DG on a null hyperedge.")
fail(lambda: DG.HyperEdge().numSources, "Can not get number of sources on a null hyperedge.")
fail(lambda: DG.HyperEdge().sources, "Can not get sources on a null hyperedge.")
fail(lambda: DG.HyperEdge().numTargets, "Can not get number of targets on a null hyperedge.")
fail(lambda: DG.HyperEdge().targets, "Can not get targets on a null hyperedge.")
fail(lambda: DG.HyperEdge().rules, "Can not get rules on a null hyperedge.")
fail(lambda: DG.HyperEdge().inverse, "Can not get inverse on a null hyperedge.")
fail(lambda: DG.HyperEdge().print(), "Can not print a null hyperedge.")
