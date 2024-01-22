g0 = Graph.fromDFS("[_X]{f(_X, a)}[g(h(_X), i)]")
g = UnionGraph([g0, g0])
g0.printTermState()
g.printTermState()
