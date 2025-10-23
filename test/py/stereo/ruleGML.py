sys.exit(0)
def doIt(a):
	a.print()
	a.printStereo()

doIt(Rule.fromGMLString("""rule [
	ruleID "EC 5.1.3.1"
	left [
		node [ id 50 label "H" ]
		node [ id 51 label "H+" ]
		edge [ source 2 target 50 label "-" ]
	]
	context [
		node [ id 0 label "C" ]
		node [ id 1 label "C" ] node [ id 4 label "O" ]
		node [ id 2 label "C" ] node [ id 5 label "O" ] node [ id 6 label "H" ]
		node [ id 3 label "C" ]
		edge [ source 0 target 1 label "-" ]
		edge [ source 1 target 2 label "-" ]
		edge [ source 2 target 3 label "-" ]
		edge [ source 1 target 4 label "=" ]
		edge [ source 2 target 5 label "-" ]
		edge [ source 5 target 6 label "-" ]
	]
	right [
		node [ id 50 label "H+" ]
		node [ id 51 label "H" ]
		edge [ source 2 target 51 label "-" ]
	]
]"""))
doIt(Rule.fromGMLString("""rule [
	ruleID "EC 5.1.3.1"
	left [
		node [ id 50 label "H" ]
		node [ id 51 label "H+" ]
		edge [ source 2 target 50 label "-" ]
	]
	context [
		node [ id 0 label "C" ]
		node [ id 1 label "C" ] node [ id 4 label "O" ]
		node [ id 2 label "C" stereo [ geometry "TetrahedralFree" ] ]
			node [ id 5 label "O" ] node [ id 6 label "H" ]
		node [ id 3 label "C" ]
		edge [ source 0 target 1 label "-" ]
		edge [ source 1 target 2 label "-" ]
		edge [ source 2 target 3 label "-" ]
		edge [ source 1 target 4 label "=" ]
		edge [ source 2 target 5 label "-" ]
		edge [ source 5 target 6 label "-" ]
	]
	right [
		node [ id 50 label "H+" ]
		node [ id 51 label "H" ]
		edge [ source 2 target 51 label "-" ]
	]
]"""))


sys.exit(0)



Rule.fromGMLString("""rule [
ruleID "Diels-alder"
left [
	node [ id 1 label "C" stereo [ order "2, -_b, -_a" ] ]
	node [ id 4 label "C" stereo [ order "3, -_d, -_c" ] ]
	edge [ source 1 target 2 label "=" ]
	edge [ source 2 target 3 label "-" ]
	edge [ source 3 target 4 label "=" ]

	node [ id 5 label "C" stereo [ order "6, -_f, -_e" ] ]
	node [ id 6 label "C" stereo [ order "5, -_g, -_h" ] ]
	edge [ source 5 target 6 label "=" ]
]
context [
	node [ id 2 label "C" stereo [ order "1, -, 3" ]  ]
	node [ id 3 label "C" stereo [ order "4, 2, -" ]  ]
]
right [
	node [ id 1 label "C" stereo [ order "2, 5, -_a, -_b" ] ]
	node [ id 4 label "C" stereo [ order "3, 6, -_c, -_d" ] ]
	node [ id 5 label "C" stereo [ order "6, 1, -_e, -_f" ] ]
	node [ id 6 label "C" stereo [ order "5, 4, -_h, -_g" ] ]
	edge [ source 1 target 2 label "-" ]
	edge [ source 2 target 3 label "=" ]
	edge [ source 3 target 4 label "-" ]
	edge [ source 4 target 5 label "-" ]
	edge [ source 5 target 6 label "-" ]
	edge [ source 6 target 1 label "-" ]
]
]""")
































