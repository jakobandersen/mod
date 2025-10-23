include("../xxx_helpers.py")

g = Graph.fromSMILES("O")
assert g.id == 0
assert str(g) == "'{}'".format(g.name)
assert repr(g) == "{}({})".format(str(g), g.id)
assert not (g < g)
assert g == g


p = GraphPrinter()
g.print()
g.print(p)
g.print(p, p)
