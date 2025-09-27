include("../xxx_helpers.py")
dg = DG()
dg.build().addAbstract("A -> B")
A = next(v for v in dg.vertices if v.graph.name == "A")
B = next(v for v in dg.vertices if v.graph.name == "B")
e = next(iter(dg.edges))

a = causality.EdgeAction()
assert str(a) == "EdgeAction{%s}" % DG.HyperEdge()
a = causality.EdgeAction(e)
assert str(a) == "EdgeAction{%s}" % e

a = causality.InputAction()
assert str(a) == "InputAction{%s}" % DG.Vertex()
a = causality.InputAction(A)
assert str(a) == "InputAction{%s}" % A

a = causality.OutputAction()
assert str(a) == "OutputAction{%s}" % DG.Vertex()
a = causality.OutputAction(A)
assert str(a) == "OutputAction{%s}" % A
