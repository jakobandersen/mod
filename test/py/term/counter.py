a = Graph.fromDFS("[f(0)][sd(0)]")
incFirst = Rule.fromGMLString("""rule [
	ruleID "Inc first"
	left [
		node [ id 0 label "f(_X)" ]
	]
	right [
		node [ id 0 label "f(s(_X))" ]
	]
]""")
incSecond = Rule.fromGMLString("""rule [
	ruleID "Inc second"
	left [
		node [ id 0 label "sd(_X)" ]
	]
	context [
		node [ id 1 label "f(s(0))" ]
		edge [ source 0 target 1 label "-" ]
	]
	right [
		node [ id 0 label "sd(s(_X))" ]
	]
]""")
incSecond.printTermState()

dg = DG(graphDatabase=inputGraphs,
	labelSettings=LabelSettings(LabelType.Term, LabelRelation.Specialisation))
dg.build().execute(
	addSubset(a)
	>> repeat[1]([incSecond, incFirst])
	>> incSecond
)
p = DGPrinter()
p.graphPrinter.withGraphvizCoords = True
dg.print(p)
