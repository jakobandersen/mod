lString = LabelSettings(LabelType.String, LabelRelation.Unification)
lTerm = LabelSettings(LabelType.Term, LabelRelation.Unification)

smiles("O")
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
ruleGMLString(ruleTemplate % ("Leq2", "leq2", "<=", "2"))
ruleGMLString(ruleTemplate % ("Eq1", "eq1", "=", "1"))
ruleGMLString(ruleTemplate % ("Eq3", "eq3", "=", "3"))
ruleGMLString(ruleTemplate % ("Geq2", "geq2", ">=", "2"))
length1 = graphGMLString('graph [ %s edge [ source 0 target 3  label "-" ] ]' % graphCommon)
length3 = graphGMLString('graph [ %s ]' % graphCommon)
dg = DG(graphDatabase=inputGraphs, labelSettings=lString)
dg.build().execute(addSubset(inputGraphs) >> inputRules)
dg.print()
dg = DG(graphDatabase=inputGraphs, labelSettings=lString)
dg.build().execute(addSubset(inputGraphs) >> inputRules)
dg.print()




sys.exit(0)


print("Different connected components\n%s" % ('='*70))
ruleGMLString("""rule [
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


