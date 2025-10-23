def doPrint(a):
	pg = GraphPrinter()
	pm = GraphPrinter()
	pm.setMolDefault()
	a.print(pg, pm)
	pg.withRawStereo = True
	pm.withRawStereo = True
	pg.withIndex = True
	pm.withIndex = True
	a.print(pg, pm)
	

post.summarySection("Context")
a = Rule.fromGMLString("""rule [
	context [
		node [ id 0 label "C" stereo "[1, 2, 3, 4]!" ]
		node [ id 1 label "O" ]
		node [ id 2 label "N" ]
		node [ id 3 label "S" ]
		node [ id 4 label "P" ]
		edge [ source 0 target 1 label "-" ]
		edge [ source 0 target 2 label "-" ]
		edge [ source 0 target 3 label "-" ]
		edge [ source 0 target 4 label "-" ]
	]	
]""")
doPrint(a)
post.summarySection("Left and right")
a = Rule.fromGMLString("""rule [
	left [
		node [ id 0 stereo "[1, 2, 3, 4]!" ]
	]
	context [
		node [ id 0 label "C" ]
		node [ id 1 label "O" ]
		node [ id 2 label "N" ]
		node [ id 3 label "S" ]
		node [ id 4 label "P" ]
		edge [ source 0 target 1 label "-" ]
		edge [ source 0 target 2 label "-" ]
		edge [ source 0 target 3 label "-" ]
		edge [ source 0 target 4 label "-" ]
	]
	right [
		node [ id 0 stereo "[1, 2, 3, 4]!" ]
	]
]""")
doPrint(a)
a = Rule.fromGMLString("""rule [
	left [
		node [ id 0 stereo "[1, 2, 3, 4]!" ]
	]
	context [
		node [ id 0 label "C" ]
		node [ id 1 label "O" ]
		node [ id 2 label "N" ]
		node [ id 3 label "S" ]
		node [ id 4 label "P" ]
		edge [ source 0 target 1 label "-" ]
		edge [ source 0 target 2 label "-" ]
		edge [ source 0 target 3 label "-" ]
		edge [ source 0 target 4 label "-" ]
	]
	right [
		node [ id 0 stereo "[1, 2, 4, 3]!" ]
	]
]""")
doPrint(a)
sys.exit(0)
a = Graph.fromGMLString("""graph [
	node [ id 0 label "C" stereo "[1, 2, 4, 3]!" ]
	node [ id 1 label "O" ]
	node [ id 2 label "N" ]
	node [ id 3 label "S" ]
	node [ id 4 label "P" ]
	edge [ source 0 target 1 label "-" ]
	edge [ source 0 target 2 label "-" ]
	edge [ source 0 target 3 label "-" ]
	edge [ source 0 target 4 label "-" ]
]""")
doPrint(a)

post.summarySection("Lone Pair")
a = Graph.fromGMLString("""graph [
	node [ id 0 label "N" stereo "[1, 2, 3, e]!" ]
	node [ id 1 label "O" ]
	node [ id 2 label "N" ]
	node [ id 3 label "S" ]
	edge [ source 0 target 1 label "-" ]
	edge [ source 0 target 2 label "-" ]
	edge [ source 0 target 3 label "-" ]
]""")
doPrint(a)
a = Graph.fromGMLString("""graph [
	node [ id 0 label "N" stereo "[1, 2, e, 3]!" ]
	node [ id 1 label "O" ]
	node [ id 2 label "N" ]
	node [ id 3 label "S" ]
	edge [ source 0 target 1 label "-" ]
	edge [ source 0 target 2 label "-" ]
	edge [ source 0 target 3 label "-" ]
]""")
doPrint(a)

a = Graph.fromGMLString("""graph [
	node [ id 0 label "N" stereo "[e, 1, 2, 3]!" ]
	node [ id 1 label "O" ]
	node [ id 2 label "N" ]
	node [ id 3 label "S" ]
	edge [ source 0 target 1 label "-" ]
	edge [ source 0 target 2 label "-" ]
	edge [ source 0 target 3 label "-" ]
]""")
doPrint(a)

post.summarySection("H")
a = Graph.fromGMLString("""graph [
	node [ id 0 label "C" stereo "[1, 2, 3, 4]!" ]
	node [ id 1 label "O" ]
	node [ id 2 label "N" ]
	node [ id 3 label "S" ]
	node [ id 4 label "H" ]
	edge [ source 0 target 1 label "-" ]
	edge [ source 0 target 2 label "-" ]
	edge [ source 0 target 3 label "-" ]
	edge [ source 0 target 4 label "-" ]
]""")
doPrint(a)
a = Graph.fromGMLString("""graph [
	node [ id 0 label "C" stereo "[1, 2, 4, 3]!" ]
	node [ id 1 label "O" ]
	node [ id 2 label "N" ]
	node [ id 3 label "S" ]
	node [ id 4 label "H" ]
	edge [ source 0 target 1 label "-" ]
	edge [ source 0 target 2 label "-" ]
	edge [ source 0 target 3 label "-" ]
	edge [ source 0 target 4 label "-" ]
]""")
doPrint(a)

a = Graph.fromGMLString("""graph [
	node [ id 0 label "C" stereo "[1, 2, 3, 4]!" ]
	node [ id 1 label "H" ]
	node [ id 2 label "N" ]
	node [ id 3 label "S" ]
	node [ id 4 label "O" ]
	edge [ source 0 target 1 label "-" ]
	edge [ source 0 target 2 label "-" ]
	edge [ source 0 target 3 label "-" ]
	edge [ source 0 target 4 label "-" ]
]""")
doPrint(a)


post.summarySection("All H")
a = Graph.fromGMLString("""graph [
	node [ id 0 label "C" stereo "[1, 2, 3, 4]!" ]
	node [ id 1 label "H" ]
	node [ id 2 label "H" ]
	node [ id 3 label "H" ]
	node [ id 4 label "H" ]
	edge [ source 0 target 1 label "-" ]
	edge [ source 0 target 2 label "-" ]
	edge [ source 0 target 3 label "-" ]
	edge [ source 0 target 4 label "-" ]
]""")
doPrint(a)
