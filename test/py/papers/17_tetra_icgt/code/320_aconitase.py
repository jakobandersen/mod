water = smiles("O", "H_2O")
cit = smiles("C(C(=O)O)C(CC(=O)O)(C(=O)O)O", name="Cit")
d_icit = smiles("C([C@@H]([C@H](C(=O)O)O)C(=O)O)C(=O)O", name="D-ICit")

aconitase = ruleGMLString("""rule [
	left [
		# the dehydrated water
		edge [ source 1 target 100 label "-" ]   edge [ source 2 target 102 label "-" ]
		# the hydrated water
		edge [ source 200 target 202 label "-" ]
	]
	context [
		node [ id 1 label "C" ]
		edge [ source 1 target 2 label "-" ] # goes from - to = to -
		node [ id 2 label "C" ]
		# the dehydrated water
		node [ id 100 label "O" ]   node [ id 101 label "H" ]   node [ id 102 label "H" ]
		edge [ source 100 target 101 label "-" ]
		# the hydrated water
		node [ id 200 label "O" ]   node [ id 201 label "H" ]   node [ id 202 label "H" ]
		edge [ source 200 target 201 label "-" ]
		# dehydrated C neighbours
		node [ id 1000 label "C" ]   node [ id 1010 label "O" ]   node [ id 1001 label "C" ]
		edge [ source 1 target 1000 label "-" ]   edge [ source 1000 target 1010 label "-" ]
		edge [ source 1 target 1001 label "-" ]
		# hydrated C neighbours
		node [ id 2000 label "C" ]   node [ id 2001 label "H" ]
		edge [ source 2 target 2000 label "-" ]   edge [ source 2 target 2001 label "-" ]
	]
	right [
		# The '!' in the end changes it from TetrahedralSym to
		# TetrahedralFixed
		node [ id 1 stereo "tetrahedral[1000, 1001, 202, 2]!" ]
		node [ id 2 stereo "tetrahedral[200, 1, 2000, 2001]!" ]
		# the dehydrated water
		edge [ source 100 target 102 label "-" ]
		# the hydrated water
		edge [ source 1 target 202 label "-" ]   edge [ source 2 target 200 label "-" ]
	]
]""")

dg = DG(graphDatabase=inputGraphs,
	# seldctino of attributes and morphisms for matching
	labelSettings=LabelSettings(
		# use terms as labels, instead of strings
		LabelType.Term,
		# term morphisms may be specialisations
		LabelRelation.Specialisation,
		# use stereo information,
		# with specialisation in the morphisms
		LabelRelation.Specialisation)
)
dg.build().execute(addSubset(cit, water) >> aconitase)
for e in dg.edges:
	p = GraphPrinter()
	p.withColour = True
	e.print(p, matchColour="Maroon")
