post.disableInvokeMake()

lString = LabelSettings(LabelType.String, LabelRelation.Specialisation)
lTerm = LabelSettings(LabelType.Term, LabelRelation.Specialisation)

Graph.fromDFS("[Q]({a}[A])({b}[B])({a}[C])({b}[C])({c}[C])({c}[C])")
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
	'A': 'nodeLabels [ label "A" ]',
	'B': 'nodeLabels [ label "B" label "B" ]', # make sure duplicates don't do anything
	'C': 'nodeLabels [ label "C" ]',
	'AB': 'nodeLabels [ label "A" label "B" ]',
	'AC': 'nodeLabels [ label "A" label "C" ]',
	'BC': 'nodeLabels [ label "B" label "C" ]',
	'ABC': 'nodeLabels [ label "A" label "B" label "C" ]',
}
edgeLabels = {
	'': '',
	'a': 'edgeLabels [ label "a" ]',
	'b': 'edgeLabels [ label "b" label "b" ]',
	'c': 'edgeLabels [ label "c" ]',
	'ab': 'edgeLabels [ label "a" label "b" ]',
	'ac': 'edgeLabels [ label "a" label "c" ]',
	'bc': 'edgeLabels [ label "b" label "c" ]',
	'abc': 'edgeLabels [ label "a" label "b" label "c" ]',
}
for count in range(10):
	for opName, op in ops.items():
		for nlName, nl in nodeLabels.items():
			for elName, el in edgeLabels.items():
				name =  ','.join([opName, str(count), f"V{nlName}", f"E{elName}"])
				labels = f"{nl}\n{el}\n"
				Rule.fromGMLString(ruleTemplate.format(name, name, op, count, labels))

err = False

def doDG(name, lSettings):
	print(name + "\n" + "="*50)
	dg = DG(graphDatabase=inputGraphs, labelSettings=lSettings)
	dg.build().execute(addSubset(inputGraphs) >> inputRules)
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
		candCount = 0
		for e in v.incidentEdges:
			if len(el) != 0 and e.stringLabel not in el:
				continue
			u = e.target
			if len(nl) != 0 and e.target.stringLabel not in nl:
				continue
			candCount += 1	
		if(not evalOp(candCount, op, count)):
			print(f"Error: '{nl}' '{el}' = {candConut} {op} {count}")
			err = True

doDG("String", lString)
doDG("Term", lTerm)

if err:
	assert False
