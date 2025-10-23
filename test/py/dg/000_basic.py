include("xx0_helpers.py")

g = Graph.fromSMILES('O')
v = DG.Vertex()

dg =  DG()
assert dg.id == 0
assert str(dg) == "DG(%d)" % dg.id
assert str(dg) == repr(dg)

assert not dg.hasActiveBuilder
assert not dg.locked

msg = "The DG neither has an active builder nor is locked yet."
fail(lambda: dg.numVertices, msg)
fail(lambda: dg.vertices, msg)
fail(lambda: dg.numEdges, msg)
fail(lambda: dg.edges, msg)

b1 = dg.build()
# Active builder, not locked
assert not dg.locked
assert dg.hasActiveBuilder

assert dg.numVertices == 0
assert list(dg.vertices) == []
assert dg.numEdges == 0
assert list(dg.edges) == []

fail(lambda: dg.build(), "Another build is already in progress.")

del b1
# Locked, no active builder
fail(lambda: dg.build(), "The DG is locked.")

assert dg.numVertices == 0
assert list(dg.vertices) == []
assert dg.numEdges == 0
assert list(dg.edges) == []


dg = DG()
assert dg.labelSettings.type == LabelType.String
assert dg.labelSettings.relation == LabelRelation.Isomorphism
assert not dg.labelSettings.withStereo
assert dg.labelSettings.stereoRelation == LabelRelation.Isomorphism
ls = LabelSettings(LabelType.Term, LabelRelation.Isomorphism, LabelRelation.Isomorphism)
dg = DG(labelSettings=ls)
assert dg.labelSettings.type == LabelType.Term
assert dg.labelSettings.relation == LabelRelation.Isomorphism
assert dg.labelSettings.withStereo
assert dg.labelSettings.stereoRelation == LabelRelation.Isomorphism

fail(lambda: DG(labelSettings=LabelSettings(LabelType.String, LabelRelation.Unification)),
	"The label settings does not form a category. The relation is Unification, but must be Isomorphism or Specialisation.")
fail(lambda: DG(labelSettings=LabelSettings(LabelType.String, LabelRelation.Isomorphism, LabelRelation.Unification)),
	"The label settings does not form a category. The stereoRelation is Unification, but must be Isomorphism or Specialisation.")

fail(lambda: DG(graphDatabase=[None]), "Null pointer in graph database.")
g1 = Graph.fromSMILES('O')
g2 = Graph.fromSMILES('O')
fail(lambda: DG(graphDatabase=[g1, g2]), "Isomorphic graphs '{}' and '{}' in initial graph database.".format(g1.name, g2.name))
dg = DG(graphDatabase=[g1, g2], graphPolicy=IsomorphismPolicy.TrustMe)

dg = DG(graphDatabase=[g])
assert dg.createdGraphs == []
dg.build().execute(addSubset(g) >> Rule.fromGMLString("""rule [
	left [ node [ id 0 label "O" ] ]
	right [ node [ id 0 label "S" ] ]
]"""))
assert len(dg.createdGraphs) == 1
assert dg.createdGraphs[0].isomorphism(Graph.fromSMILES("S")) == 1

dg = DG(graphDatabase=[g])
assert dg.createdGraphs == []
dg.build().apply([g], Rule.fromGMLString("""rule [
	left [ node [ id 0 label "O" ] ]
	right [ node [ id 0 label "S" ] ]
]"""))
assert len(dg.createdGraphs) == 1
assert dg.createdGraphs[0].isomorphism(Graph.fromSMILES("S")) == 1
