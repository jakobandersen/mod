include("common.py")
a = Graph.fromSMILES("C1C2CCC12(N)(P)")
config.graph.smilesCheckAST = False
b = Graph.fromSMILES("C1C2CCC(N)1(P)2")
config.graph.smilesCheckAST = True
assert a.isomorphism(b) > 0
