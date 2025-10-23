Graph.fromSMILES("C[13CH2]C")
Graph.fromDFS("C[13Q]([H])([H])C")
Graph.fromSMILES("[2H][H]")
Graph.fromSMILES("[H][2H]")
Graph.fromSMILES("[2H]O")
Graph.fromSMILES("[18OH2]")

p = GraphPrinter()
for a in inputGraphs: 
	a.print()
	a.print(p)

a = Graph.fromSMILES("C[13CH2]C")
for v in a.vertices:
	v.printStereo()
a = Graph.fromDFS("C[13Q]([H])([H])C")
for v in a.vertices:
	v.printStereo()

a = Rule.fromGMLString("""rule [
	left [
		node [ id 0 label "13C" ]
		node [ id 4 label "13C" ]
		node [ id 5 label "13Q" ]
	]
	context [
		node [ id 1 label "13C" ]
		node [ id 6 label "13Q" ]
	]
	right [
		node [ id 2 label "13C" ]
		node [ id 4 label "14C" ]
		node [ id 7 label "13Q" ]
	]
]""")
a.print()
