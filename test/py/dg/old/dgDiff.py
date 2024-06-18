def makeGraph(name):
	return graphDFS("[%s]" % name)
def makeRule(name):
	return ruleGMLString('rule [ ruleID "%s" ]' % name)
def makeDer(left, rule, right):
	d = Derivation()
	d.left = left
	d.rule = rule
	d.right = right
	return d

a = makeGraph("a")
b = makeGraph("b")
c = makeGraph("c")
d = makeGraph("d")
r1 = makeRule("r1")
r2 = makeRule("r2")
r3 = makeRule("r3")
r4 = makeRule("r4")

print("DG1")
dg1 = DG()
with dg1.build() as builder:
	for d in [
		makeDer([a], r1, [b]),
		makeDer([a], r2, [b]),
		makeDer([a, a], r1, [b]),
	]:
		builder.addDerivation(d)
print("DG2")
dg2 = DG()
with dg2.build() as builder:
	for d in [
		makeDer([a, a], r2, [b]),
		makeDer([a], r1, [b]),
	]:
		builder.addDerivation(d)
diffDGs(dg1, dg2)
dg1.print()
dg2.print()
