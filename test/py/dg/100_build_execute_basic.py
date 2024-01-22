include("xx0_helpers.py")

g1a = smiles('O', "g1a")
g2a = smiles('C', "g2a")

dg = DG()
res = dg.build().execute(addSubset(g1a) >> addSubset(g2a))
print("list default")
print("-" * 80)
res.list()

print("list with universe")
print("-" * 80)
res.list(withUniverse=True)


print("deprecated dgRuleComp")
dg = dgRuleComp([g1a], addSubset(g2a),
	labelSettings=LabelSettings(LabelType.Term, LabelRelation.Isomorphism))
assert dg.labelSettings.type == LabelType.Term
assert dg.labelSettings.relation == LabelRelation.Isomorphism
assert dg.graphDatabase == [g1a]
dg.calc(printInfo=False)
assert dg.graphDatabase == [g1a, g2a]


fail(lambda: DG().build().execute(None), "Can not convert type '{}' to DGStrat.".format(type(None)), err=TypeError)


dg = DG()
with dg.build() as b:
	for verbosity in (0, 2, 4, 6, 8, 10, 48, 50, 60):
		print("Verbose execute, level {}".format(verbosity))
		print("=" * 80)
		res = b.execute(
			repeat[42](
				revive(
					leftPredicate[lambda d: True](
						rightPredicate[lambda d: False](
							[
								addSubset(g1a),
								addUniverse(g2a)
							]
							>> filterSubset(False)
							>> filterUniverse(lambda g, gs, first: False)
							>> addSubset(inputGraphs)
							>> ruleGMLString("rule [ left [ node [ id 0 label \"O\" ] ] right [ node [ id 0 label \"Q\" ] ] ]")
						)
					)
				)
			) >> repeat(filterUniverse(False)),
			verbosity=verbosity
		)
	print("list")
	print("=" * 80)
	res.list(withUniverse=True)
