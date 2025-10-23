lString = LabelSettings(LabelType.String, LabelRelation.Specialisation)
lTerm = LabelSettings(LabelType.Term, LabelRelation.Specialisation)

Graph.fromSMILES("O")
print("Same connected component\n%s" % ('='*70))
graphCommon = """
	node [ id 0 label "s" ]
	node [ id 1 label "v" ]
	node [ id 2 label "v" ]
	node [ id 3 label "t" ]
	edge [ source 0 target 1 label "-" ]
	edge [ source 1 target 2 label "-" ]
	edge [ source 2 target 3 label "-" ]
"""
ruleTemplate = """rule [
	ruleID "%s"
	left [
		edge [ source 1 target 2 label "-" ]
	]
	context [
		node [ id 0 label "s" ]
		node [ id 1 label "v" ]
		node [ id 2 label "v" ]
		node [ id 3 label "t" ]
		edge [ source 0 target 1 label "-" ]
		edge [ source 2 target 3 label "-" ]

		node [ id 9 label "O" ]
	]
	right [
		edge [ source 1 target 2 label "%s" ]
	]
	constrainShortestPath [
		source 0 target 3
		op "%s" length %s
	]
]"""
Rule.fromGMLString(ruleTemplate % ("Leq2", "leq2", "<=", "2"))
Rule.fromGMLString(ruleTemplate % ("Eq1", "eq1", "=", "1"))
Rule.fromGMLString(ruleTemplate % ("Eq3", "eq3", "=", "3"))
Rule.fromGMLString(ruleTemplate % ("Geq2", "geq2", ">=", "2"))
length1 = Graph.fromGMLString('graph [ %s edge [ source 0 target 3  label "-" ] ]' % graphCommon)
length3 = Graph.fromGMLString('graph [ %s ]' % graphCommon)
dg = DG(graphDatabase=inputGraphs, labelSettings=lString)
dg.build().execute(addSubset(inputGraphs) >> inputRules)
dg.print()
dg = DG(graphDatabase=inputGraphs, labelSettings=lString)
dg.build().execute(addSubset(inputGraphs) >> inputRules)
dg.print()




sys.exit(0)


print("Different connected components\n%s" % ('='*70))
Rule.fromGMLString("""rule [
	left [
		node [ id 0 label "a" ]
		node [ id 1 label "b" ]
	]
	constrainShortestPath [
		source 0 target 1
		op "=" length 1
	]
]""")
assert False


