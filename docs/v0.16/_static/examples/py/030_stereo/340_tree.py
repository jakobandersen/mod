g = smiles("N[C@](O)([C@](S)(P)(O))([C@](S)(P)(O))")
change = ruleGMLString("""rule [
	ruleID "Change"
	left [
		node [ id 0 stereo "tetrahedral" ]
	]
	context [
		node [ id 0 label "*" ]
		node [ id 1 label "*" ]
		node [ id 2 label "*" ]
		node [ id 3 label "*" ]
		node [ id 4 label "*" ]
		edge [ source 0 target 1 label "-" ]
		edge [ source 0 target 2 label "-" ]
		edge [ source 0 target 3 label "-" ]
		edge [ source 0 target 4 label "-" ]
	]
	right [
		node [ id 0 stereo "tetrahedral[1, 2, 3, 4]!" ]
	]
]""")

dg = DG(graphDatabase=inputGraphs,
		labelSettings=LabelSettings(
			LabelType.Term,
			LabelRelation.Specialisation,
			LabelRelation.Specialisation))
dg.build().execute(addSubset(inputGraphs) >> repeat(change))

p = GraphPrinter()
p.setMolDefault()
p.withPrettyStereo = True
change.print(p)
p = DGPrinter()
p.withRuleName = True
p.withRuleId = False
dg.print(p)
# rst-name: Non-trivial Stereoisomers
# rst: Generation of stereoisomers in a non-trivial molecule.
