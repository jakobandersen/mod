r = ruleGMLString("""rule [
	left    [   node [ id 0 stereo "tetrahedral" ]   ] # TetrahedralSym shape
	context [
		# Has explicit informatino in 'L' and 'R', so nothing here.
		node [ id 0 label "C" ]
		# Gets inferred stereo information: TetrahedralSym shape
		node [ id 1 label "C" ]
		# Gets inferred stereo information: Any
		node [ id 2 label "P" ]   node [ id 3 label "S" ]   node [ id 4 label "O" ]

		edge [ source 0 target 1 label "-" ]   edge [ source 0 target 2 label "-" ]
		edge [ source 0 target 3 label "-" ]   edge [ source 0 target 4 label "-" ]
	]
	right   [   node [ id 0 stereo "tetrahedral" ]   ] # TetrahedralSym shape
]""")

g = smiles("[C@](C)(P)(S)(O)")
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
dg.build().execute(addSubset(g) >> r)
# print DPO digram with special settings
p = GraphPrinter()
p.setReactionDefault()
p.withPrettyStereo = True
p.withIndex = True
p.rotation = 225
for e in dg.edges:
	e.print(p, matchColour="")
