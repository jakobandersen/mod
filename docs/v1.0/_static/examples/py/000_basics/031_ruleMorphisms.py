# A rule with no extra context:
small = ruleGMLString("""rule [
	ruleID "Small"
	left [
		node [ id 1 label "H" ]
		node [ id 2 label "O" ]
		edge [ source 1 target 2 label "-" ]
	]
	right [
		node [ id 1 label "H+" ]
		node [ id 2 label "O-" ]
	]
]""")
# The same rule, with a bit of context:
large = ruleGMLString("""rule [
	ruleID "Large"
	left [
		node [ id 1 label "H" ]
		node [ id 2 label "O" ]
		edge [ source 1 target 2 label "-" ]
	]
	context [
		node [ id 3 label "C" ]
		edge [ source 2 target 3 label "-" ]
	]
	right [
		node [ id 1 label "H+" ]
		node [ id 2 label "O-" ]
	]
]""")
isomorphic = small.isomorphism(large) == 1
print("Isomorphic?", isomorphic)
atLeastAsGeneral = small.monomorphism(large) == 1
print("At least as general?", atLeastAsGeneral)
# rst-name: Rule Morphisms
# rst: Rule objects, like graph objects, have methods for finding morphisms with
# rst: the VF2 algorithms for isomorphism and monomorphism.
# rst: We can therefore easily detect isomorphic rules, 
# rst: and decide if one rule is at least as specific/general as another.
