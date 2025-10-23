include("1xx_execute_helpers.py")

rSplit = Rule.fromGMLString("""rule [
	ruleID "Split to isomorphms"
	left [
		edge [ source 1 target 2 label "-" ]
	]
	context [
		node [ id 1 label "Q" ]
		node [ id 2 label "Q" ]
	]
]""")
g1 = Graph.fromDFS("[Q][Q]", "QQ")
g2 = Graph.fromDFS("[Q]", "Q")
exeStrat(addSubset(g1) >> rSplit, [g2], [g1, g2], graphDatabase=[g1, g2], verbosity=10)
