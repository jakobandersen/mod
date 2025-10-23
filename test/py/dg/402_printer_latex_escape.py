include("../xxx_helpers.py")
post.enableInvokeMake()

dg = DG()
with dg.build() as b:
	d = Derivation()
	d.left = [Graph.fromSMILES("CO", name="g#_x")]
	d.right = [Graph.fromSMILES("CS")]
	d.rule = Rule.fromGMLString('''rule [
		ruleID "r#_x"
		left [ node [ id 0 label "O" ] ]
		right [ node [ id 0 label "S" ] ]
	]''')
	b.addDerivation(d)

p = DGPrinter()
p.withRuleName = True

post.summarySection("labelsAsLatexMath=false")
p.labelsAsLatexMath = True
dg.print(p)

post.summarySection("labelsAsLatexMath=true")
p.labelsAsLatexMath = True
dg.print(p)
