rule [
	ruleID "R2"
	left [
		node [ id 1 label "B2" ]
		node [ id 2 label "B1" ]
	]
	context [
		edge [ source 1 target 2 label "-" ]
	]
	right [
		node [ id 1 label "C2" ]
		node [ id 2 label "C1" ]
	]
]
