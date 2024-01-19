include("../xxx_helpers.py")

W = Graph.fromDFS("[*]", name="W")
r = Rule.fromDFS("[*]1>>[S]1")

W.print()
W.printTermState()
r.print()
r.printTermState()

ls = LabelSettings(LabelType.Term, LabelRelation.Specialisation)

dg = DG(graphDatabase=inputGraphs, labelSettings=ls)
dg.build().apply([W], r)
assert dg.numEdges == 1
e = next(iter(dg.edges))
maps = DGVertexMapper(e, verbosity=10)
assert maps.size == 1
