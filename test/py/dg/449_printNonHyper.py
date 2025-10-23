include("../xxx_helpers.py")
post.enableInvokeMake()

dg = DG()

with dg.build() as b:
	d = Derivation()
	d.left = [Graph.fromSMILES("CO")]
	d.right = [Graph.fromSMILES("CS")]
	d.rule = Rule.fromGMLString('''rule [
		left [ node [ id 0 label "O" ] ]
		right [ node [ id 0 label "S" ] ]
	]''')
	b.addDerivation(d)
	s = "A + hide -> B\n"
	for i in range(1, 4):
		s += "{i} A{i} -> {i} B\n".format(i=i)
	for i in range(1, 4):
		for j in range(1, 4):
			s += "{i} C{i}_{j} + D{i}_{j} -> {j} C{i}_{j} + E\n".format(i=i, j=j)
	b.addAbstract(s)
	dg.printNonHyper()
dg.printNonHyper()
