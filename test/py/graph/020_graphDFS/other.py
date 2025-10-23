include("common.py")

a = Graph.fromSMILES("[CH3][CH2][CH2][CH2][CH2][C@H]([OH])/[CH]=[CH]/[C@H]1[C@H]2[CH2][C@H]([O][O]2)[C@@H]1[CH2]/[CH]=[CH]\\[CH2][CH2][CH2][C](=[O])[O-]")
b = Graph.fromDFS(a.graphDFS)
assert a.isomorphism(b) != 0
c = Graph.fromDFS(a.graphDFSWithIds)
assert a.isomorphism(c) != 0
