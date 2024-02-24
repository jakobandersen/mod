post.disableInvokeMake()

# test case for the bug where the constraint is checked in the core graph
# instead of the component graph
rLeft = Rule.fromGMLString("""rule [
	left [
		edge [ source 1 target 2 label "-" ]
	]
	context [
		node [ id 1 label "C" ]
		node [ id 2 label "O" ]
	]
]""")
rRight = Rule.fromGMLString("""rule [
	context [
		node [ id 1 label "C" ]
	]
	constrainAdj [
		id 1
		nodeLabels [ label "O" ]
		count 0 op "="
	]
]""")
rc = rcEvaluator(inputRules)
exp = rc.eval(rLeft *rcSuper(enforceConstraints=True)* rRight)
assert len(exp) > 0
