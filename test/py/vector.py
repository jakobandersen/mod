a = Graph.fromSMILES("O")

assert libpymod._VecGraph() == libpymod._VecGraph()
assert libpymod._VecGraph() == []
assert [] == libpymod._VecGraph()
l = libpymod._VecGraph()
l.append(a)
assert l == [a]
assert [a] == l
