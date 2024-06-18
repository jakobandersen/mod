post.disableInvokeMake()

lTerm = LabelSettings(LabelType.Term, LabelRelation.Specialisation)

# test that multiple different matches of constraints don't increase the count

Graph.fromDFS("[Q]({a}[A])({b}[B])")
ruleTemplate = """rule [
	ruleID "{}"
	left  [ node [ id 0 label "Q" ] ]
	right [ node [ id 0 label "Q({})" ] ]
	constrainAdj [
		id 0 op "{}"
		count {}
		{}
	]
]"""
ops = {'lt': '<', 'leq': '<=', 'eq': '=', 'geq': '>=', 'gt': '>'}
def evalOp(a, op, b):
	if op == '<': return a < b
	if op == '<=': return a <= b
	if op == '=': return a == b
	if op == '>=': return a >= b
	if op == '>': return a > b
	assert False
nodeLabels = {
	'': '',
	'A': 'nodeLabels [ label "_A" ]',
	'AA': 'nodeLabels [ label "_A" label "_A" ]',
	'AB': 'nodeLabels [ label "_A" label "_B" ]',
}
edgeLabels = {
	'': '',
	'a': 'edgeLabels [ label "_a" ]',
	'aa': 'edgeLabels [ label "_a" label "_a" ]',
	'ab': 'edgeLabels [ label "_a" label "_b" ]',
}
valid = set()
for count in range(0, 4):
	for opName, op in ops.items():
		for nlName, nl in nodeLabels.items():
			for elName, el in edgeLabels.items():
				if evalOp(2, op, count):
					valid.add((nlName, elName, op, count))
				name =  ','.join([opName, str(count), f"V{nlName}", f"E{elName}"])
				labels = f"{nl}\n{el}\n"
				Rule.fromGMLString(ruleTemplate.format(name, name, op, count, labels))


dg = DG(graphDatabase=inputGraphs, labelSettings=lTerm)
dg.build().execute(addSubset(inputGraphs) >> inputRules)
dg.print()
found = set()
for vDG in dg.vertices:
	g = vDG.graph
	try:
		v = next(a for a in g.vertices if a.stringLabel.startswith("Q("))
	except StopIteration:
		continue
	l = v.stringLabel
	l = l[2:-1]
	op, count, nl, el = l.split(',')
	op, count, nl, el = ops[op], int(count), nl.strip()[1:], el.strip()[1:]
	found.add((nl, el, op, count))
err = False
for e in sorted(found):
	if e not in valid:
		print("Invalid:", e)
		err = True
for e in sorted(valid):
	if e not in found:
		print("Missing valid:", e)
		err = True
if err:
	assert False
