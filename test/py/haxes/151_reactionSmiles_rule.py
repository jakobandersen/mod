def test(r):
	print("=" * 80)
	print(r.name)
	print("=" * 80)
	res = haxes.reactionSmilesFromRule(r)
	print("  ", res)
	rNew = haxes.ruleFromReactionSmiles(res, add=False)
	iso = rNew.isomorphism(r)
	if iso == 0:
		print("ERROR, not isomorphic")
		print("Original:")
		print(r.getGMLString())
		print("New:")
		print(rNew.getGMLString())
		assert False


include("../formoseCommon/grammar.py")
for r in inputRules:
	test(r)

test(Rule.fromDFS("[C]1>>[N]1"))
test(Rule.fromDFS("[C]1[H]>>[N]1[He]"))
