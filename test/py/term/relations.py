post.summaryChapter("DG")
print("DG")
print("="*80)
a = graphDFS("[f(a)][f(b)]")
b = ruleGMLString("""rule [
	ruleID "b"
	context [
		node [ id 0 label "f(_X)" ]
		node [ id 1 label "f(_X)" ]
		edge [ source 0 target 1 label "-" ]
	]
]""")
b.printTermState()

dg = DG(graphDatabase=inputGraphs,
	labelSettings=LabelSettings(LabelType.Term, LabelRelation.Unification))
dg.build().execute(addSubset(a) >> b)
p = DGPrinter()
p.graphPrinter.withGraphvizCoords = True
dg.print(p)

def doRelations(xy1, xy2, xx):
	all = [xy1, xy2, xx]
	for a in all:
		a.printTermState()
		for b in all:
			if a != b:
				assert a.isomorphism(b) == 0
			else:
				assert a.isomorphism(b) > 0

	for lr in LabelRelation.values.values():
		if not(xy1.isomorphism(xy2, labelSettings=LabelSettings(LabelType.Term, lr)) > 0):
			print(lr)
			assert False
		if not(xy2.isomorphism(xy1, labelSettings=LabelSettings(LabelType.Term, lr)) > 0):
			print(lr)
			assert False

	assert xy1.isomorphism(xx, labelSettings=LabelSettings(LabelType.Term, LabelRelation.Isomorphism)) == 0
	assert xx.isomorphism(xy1, labelSettings=LabelSettings(LabelType.Term, LabelRelation.Isomorphism)) == 0

	assert xy1.isomorphism(xx, labelSettings=LabelSettings(LabelType.Term, LabelRelation.Unification)) > 0
	assert xx.isomorphism(xy1, labelSettings=LabelSettings(LabelType.Term, LabelRelation.Unification)) > 0

post.summaryChapter("Graph")
print("Graph")
print("="*80)
xy1 = graphDFS("[f(_X, _Y)][a]")
xy2 = graphDFS("[a][f(_A, _B)]")
xx = graphDFS("[a][f(_X, _X)]")
doRelations(xy1, xy2, xx)

post.summaryChapter("Rule")
print("Rule")
print("="*80)
xy1 = RCEvaluator([]).eval(rcId(xy1))[0]
xy2 = RCEvaluator([]).eval(rcId(xy2))[0]
xx = RCEvaluator([]).eval(rcId(xx))[0]
doRelations(xy1, xy2, xx)























