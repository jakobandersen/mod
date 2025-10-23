post.summarySection("prev, branch, branch, tail")
Graph.fromSMILES("O[C@](N)(P)S").print()
Graph.fromSMILES("O[C@@](N)(P)S").print()
post.summarySection("prev, branch, branch, branch")
Graph.fromSMILES("O[C@](N)(P)(S)").print()
Graph.fromSMILES("O[C@@](N)(P)(S)").print()
post.summarySection("branch, branch, branch, tail")
Graph.fromSMILES("[C@](O)(N)(P)S").print()
Graph.fromSMILES("[C@@](O)(N)(P)S").print()

post.summarySection("prev, H, branch, tail")
Graph.fromSMILES("O[C@H](P)S").print()
Graph.fromSMILES("O[C@@H](P)S").print()
post.summarySection("prev, H, branch, branch")
Graph.fromSMILES("O[C@H](P)(S)").print()
Graph.fromSMILES("O[C@@H](P)(S)").print()
post.summarySection("branch, H, branch, tail")
Graph.fromSMILES("[C@H](N)(P)S").print()
Graph.fromSMILES("[C@@H](N)(P)S").print()

post.summarySection("ring closure")
inputGraphs[:] = []
Graph.fromSMILES("C1O[C@](N)(P)S1", name="1").print()
Graph.fromSMILES("O1[C@](N)(P)SC1", name="2").print()
Graph.fromSMILES("[C@]1(N)(P)SCO1", name="3").print()
Graph.fromSMILES("N[C@]1(SCO1)(P)", name="4").print()
Graph.fromSMILES("O1CS[C@]1(P)(N)", name="5").print()
for a in inputGraphs:
	for b in inputGraphs:
		res = a.isomorphism(b, labelSettings=LabelSettings(LabelType.String, LabelRelation.Isomorphism, LabelRelation.Isomorphism))
		if not res:
			print(a.name, "not isomorphic to", b.name)
			print("{}: {}".format(a.name, a.smiles))
			print("{}: {}".format(b.name, b.smiles))
			assert False

post.summarySection("from OpenSMILES")
Graph.fromSMILES("FC1C[C@](Br)(Cl)CCC1").print()
Graph.fromSMILES("[C@]1(Br)(Cl)CCCC(F)C1").print()
