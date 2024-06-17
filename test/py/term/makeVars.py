a = graphDFS("[a]")
aa = graphDFS("[_A][_A]")
aaWild = graphDFS("[*]{*}[f(*)]")

p = ruleGMLString("""rule [
	ruleID "A"
	left [
		node [ id 0 label "a" ]
	]
	right [
		node [ id 0 label "_X" ]
		node [ id 1 label "_X" ]
		edge [ source 0 target 1 label "-" ]
	]
]""")
pWild = ruleGMLString("""rule [
	ruleID "AWild"
	left [
		node [ id 0 label "a" ]
	]
	right [
		node [ id 0 label "*" ]
		node [ id 1 label "f(*)" ]
		edge [ source 0 target 1 label "*" ]
	]
]""")
	

dg = DG(graphDatabase=inputGraphs, labelSettings=LabelSettings(LabelType.Term, LabelRelation.Unification))
dg.build().execute(addSubset(inputGraphs) >> inputRules)
dg.print()
post.summarySection("Input Graphs")
for a in inputGraphs:
	a.print()
	a.printTermState()
post.summarySection("Input Rules")
for a in inputRules:
	a.print()
	a.printTermState()
post.summarySection("Vertex Graphs")
for a in set((v.graph for v in dg.vertices)) - set(inputGraphs):
	a.print()
	a.printTermState()
post.summarySection("Derivations")
for e in dg.edges:
	e.print()
