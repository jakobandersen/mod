rule [
	ruleID "R2"
	left [
		node [ id 3 label "B2" ]
		node [ id 2 label "B1" ]
	]
	context [
		node [ id 1 label "Q" ]
		edge [ source 3 target 2 label "-" ]
	]
	right [
		node [ id 3 label "C2" ]
		node [ id 2 label "C1" ]
		edge [ source 2 target 1 label "=" ]
	]
	constrainAdj [
		id 1
		count 1
		op "="
		nodeLabels [ label "on_node_Q" ]
	]
]
