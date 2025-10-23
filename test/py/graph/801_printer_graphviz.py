include("../xxx_helpers.py")
post.enableInvokeMake()

a = Graph.fromSMILES("C1CCCCC1CCC")

post.summaryChapter("First")
p = GraphPrinter()
f11 = a.print(p)
p.withGraphvizCoords = True
f12 = a.print(p)
p.graphvizPrefix = 'layout = "dot"';
f13 = a.print(p)

post.summaryChapter("Second")
p = GraphPrinter()
f21 = a.print(p)
p.withGraphvizCoords = True
f22 = a.print(p)
p.graphvizPrefix = 'layout = "dot"';
f23 = a.print(p)

assert f11 == f21
assert f12 == f22
assert f13 == f23
