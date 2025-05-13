# A rule (L <- K -> R) is specified by three graph fragments:
# left, context, and right
destroyVertex = ruleGMLString("""rule [
	left [
		node [ id 1 label "A" ]
	]
]""")
createVertex = ruleGMLString("""rule [
	right [
		node [ id 1 label "A" ]
	]
]""")
identity = ruleGMLString("""rule [
	context [
		node [ id 1 label "A" ]
	]
]""")
# A vertex/edge can change label:
labelChange = ruleGMLString("""rule [
	left [
		node [ id 1 label "A" ]
		edge [ source 1 target 2 label "A" ]
	]
	# GML can have Python-style line comments too
	context [
		node [ id 2 label "Q" ]
	]
	right [
		node [ id 1 label "B" ]
		edge [ source 1 target 2 label "B" ]
	]
]""")
# A chemical rule should probably not destroy and create vertices:
ketoEnol = ruleGMLString("""rule [
	left [
		edge [ source 1 target 4 label "-" ]
		edge [ source 1 target 2 label "-" ]
		edge [ source 2 target 3 label "=" ]
	]   
	context [
		node [ id 1 label "C" ]
		node [ id 2 label "C" ]
		node [ id 3 label "O" ]
		node [ id 4 label "H" ]
	]   
	right [
		edge [ source 1 target 2 label "=" ]
		edge [ source 2 target 3 label "-" ]
		edge [ source 3 target 4 label "-" ]
	]   
]""")
# Rules can be printed, but label changing edges are not visualised in K:
ketoEnol.print()
# Add with custom options, like graphs:
p1 = GraphPrinter()
p2 = GraphPrinter()
p1.disableAll()
p1.withTexttt = True
p1.withIndex = True
p2.setReactionDefault()
for p in inputRules:
	p.print(p1, p2)
# Be careful with printing options and non-existing implicit hydrogens:
p1.disableAll()
p1.edgesAsBonds = True
p2.setReactionDefault()
p2.simpleCarbons = True # !!
ketoEnol.print(p1, p2)
# rst-name: Rule Loading
# rst: Rules must be specified in GML format.
