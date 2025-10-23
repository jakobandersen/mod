include("common.py")
a = Graph.fromSMILES("C1CCCP11NNNN1")
b = Graph.fromDFS("C1CCCP2(1)NNNN2")
assert a.isomorphism(b) == 1
