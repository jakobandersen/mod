include("../xxx_helpers.py")

inc = Rule.fromGMLString("""rule [
	ruleID "inc"
	left [
		node [ id 0 label "NUM(_N)" ]
	]
	right [
		node [ id 0 label "NUM(s(_N))" ]
	]
]""")
inc.printTermState()
a = Graph.fromGMLString("""graph [
	node [ id 0 label "NUM(0)" ]
]""")

rcEval = RCEvaluator(inputRules, labelSettings=LabelSettings(LabelType.Term, LabelRelation.Specialisation))
res = rcEval.eval(rcId(a) *rcSuper* inc, verbosity=20)
for a in res:
	a.printTermState()
	a.print()
