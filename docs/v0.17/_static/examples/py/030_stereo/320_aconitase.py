water = smiles("O", "H_2O")
cit = smiles("C(C(=O)O)C(CC(=O)O)(C(=O)O)O", name="Cit")
d_icit = smiles("C([C@@H]([C@H](C(=O)O)O)C(=O)O)C(=O)O", name="D-ICit")

aconitase = ruleGMLString("""rule [
	ruleID "Aconitase"
	left [
		# the dehydrated water
		edge [ source 1 target 100 label "-" ]
		edge [ source 2 target 102 label "-" ]
		# the hydrated water
		edge [ source 200 target 202 label "-" ]
	]
	context [
		node [ id 1 label "C" ]
		edge [ source 1 target 2 label "-" ] # goes from - to = to -
		node [ id 2 label "C" ]
		# the dehydrated water
		node [ id 100 label "O" ]
		edge [ source 100 target 101 label "-" ]
		node [ id 101 label "H" ]
		node [ id 102 label "H" ]
		# the hydrated water
		node [ id 200 label "O" ]
		edge [ source 200 target 201 label "-" ]
		node [ id 201 label "H" ]
		node [ id 202 label "H" ]
		# dehydrated C neighbours
		node [ id 1000 label "C" ]
		edge [ source 1 target 1000 label "-" ]
		node [ id 1010 label "O" ]
		edge [ source 1000 target 1010 label "-" ]
		node [ id 1001 label "C" ]
		edge [ source 1 target 1001 label "-" ]
		# hydrated C neighbours
		node [ id 2000 label "C" ]
		edge [ source 2 target 2000 label "-" ]
		node [ id 2001 label "H" ]
		edge [ source 2 target 2001 label "-" ]
	]
	right [
		# The '!' in the end changes it from TetrahedralSym to
		# TetrahedralFixed
		node [ id 1 stereo "tetrahedral[1000, 1001, 202, 2]!" ]
		node [ id 2 stereo "tetrahedral[200, 1, 2000, 2001]!" ]
		# the dehydrated water
		edge [ source 100 target 102 label "-" ]
		# the hydrated water
		edge [ source 1 target 202 label "-" ]
		edge [ source 2 target 200 label "-" ]
	]
]""")

dg = DG(graphDatabase=inputGraphs,
		labelSettings=LabelSettings(
			LabelType.Term,
			LabelRelation.Specialisation,
			LabelRelation.Specialisation))
dg.build().execute(addSubset(cit, water) >> aconitase)
for e in dg.edges:
	p = GraphPrinter()
	p.withColour = True
	e.print(p, matchColour="Maroon")
# rst-name: Stereospecific Aconitase
# rst: Modelling of the reaction performed by the aconitase
# rst: enzyme in the citric acid cycle: citrate to D-isocitrate.
# rst: The rule implements the stereo-specificity of the reaction.
