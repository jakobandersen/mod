rule [
	ruleID "R1"
	left [
		node [ id 1 label "A1" ]
		node [ id 2 label "A2" ]
	]
	context [
		edge [ source 1 target 2 label "-" ]
	]
	right [
		node [ id 1 label "B1" ]
		node [ id 2 label "B2" ]
	]
]
