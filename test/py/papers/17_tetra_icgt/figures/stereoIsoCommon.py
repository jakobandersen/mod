generalise = Rule.fromGMLString("""rule [
	ruleID "Generalize"
	left [
		node [ id 0 stereo "tetrahedral[1, 2, 3, 4]!" ]
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
		node [ id 0 stereo "tetrahedral" ]
	]
]""")
change = Rule.fromGMLString("""rule [
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

