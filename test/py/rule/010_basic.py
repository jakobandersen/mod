include("../xxx_helpers.py")

r = Rule.fromGMLString("rule []")
assert r.id == 0
assert str(r) == "'{}'".format(r.name)
assert repr(r) == "{}({})".format(str(r), r.id)
assert not (r < r)
assert r == r


p = GraphPrinter()
r.print()
r.print(printCombined=True)
r.print(p)
r.print(p, printCombined=True)
r.print(p, p)
r.print(p, p, printCombined=True)


data = """rule [
left  [ node [ id 0 label "L" ] ]
right [ node [ id 0 label "R" ] ]
]"""
r = Rule.fromGMLString(data)
ri = Rule.fromGMLString(data, invert=True)
assert r.makeInverse().isomorphism(ri) != 0
