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


assert Rule.fromGMLString('rule [ ruleID "Q" ]').name == "Q"
assert Rule.fromGMLString('rule [ ruleID "Q" ]', name="R").name == "R"

with open("myRule.gml", "w") as f:
	f.write('rule [ ruleID "Q" ]\n')
assert Rule.fromGMLFile(CWDPath("myRule.gml")).name == "Q"
assert Rule.fromGMLFile(CWDPath("myRule.gml"), name="R").name == "R"

assert Rule.fromDFS(">>", name="R").name == "R"
