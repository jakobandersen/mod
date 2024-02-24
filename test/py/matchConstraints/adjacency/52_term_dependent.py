post.disableInvokeMake()

# check that the node and edge label unifications must happen at the same time
Graph.fromDFS("[_x]{_x}[C]")
Rule.fromGMLString("""rule [
	left  [ node [ id 0 label "C" ] ]
	right [ node [ id 0 label "U" ] ]
	constrainAdj [
		id 0
		nodeLabels [ label "a" ]
		edgeLabels [ label "b" ]
		op "=" count 1
	]
]""")
dg = DG(graphDatabase=inputGraphs,
	labelSettings=LabelSettings(LabelType.Term, LabelRelation.Specialisation))
dg.build().execute(addSubset(inputGraphs) >> inputRules)
assert dg.numEdges == 0, f"|E| = {dg.numEdges}"
