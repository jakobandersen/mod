include("0xx_base_checks.py")
initialVerbose = False

def c(s1, s2, mId, sExpected, ls):
	print("- first:   ", s1)
	print("  second:  ", s2)
	print("  matchId: ", mId)
	print("  expected:", sExpected)
	r1 = Rule.fromDFS(s1)
	r2 = Rule.fromDFS(s2)
	rExpected = None if sExpected is None else Rule.fromDFS(sExpected)
	m = RCMatch(r1, r2, labelSettings=ls)
	for i1, i2 in mId.items():
		m.push(r1.getVertexFromExternalId(i1).right,
				r2.getVertexFromExternalId(i2).left)

	rs = [r1, r2]
	if rExpected is not None:
		rs.append(rExpected)

	rRes = m.compose(verbose=initialVerbose)
	if rExpected is None:
		if rRes is not None:
			post.summarySection("Check Error, Result")
			print("ERROR expected None, but got rule")
			print("Composing again with verbose=True")
			rRes.name = "rRes"
			rRes.print()
			m.compose(verbose=True)
			post.enableInvokeMake()
			assert False		
	else:
		if rRes is None:
			post.summarySection("Check Error, Result")
			print("ERROR expected rule, but got None")
			print("Composing again with verbose=True")
			rExpected.name = "rExpected"
			rExpected.print()
			m.compose(verbose=True)
			post.enableInvokeMake()
			assert False
		if rRes.isomorphism(rExpected, labelSettings=ls) != 1:
			post.summarySection("Check Error, Result")
			print("ERROR non-isomorphic result")
			print("Composing again with verbose=True")
			rRes.name = "rRes"
			rExpected.name = "rExpected"
			rRes.print()
			rExpected.print()
			m.compose(verbose=True)
			post.enableInvokeMake()
			assert False

def testSettings(ls):
	print("#" * 80)
	print(ls)
	print("#" * 80)
	doChecks(lambda *args: c(*args, ls=ls))

testSettings(LabelSettings(LabelType.String, LabelRelation.Specialisation))
testSettings(LabelSettings(LabelType.Term, LabelRelation.Specialisation))
#testSettings(LabelSettings(LabelType.String, LabelRelation.Isomorphism, LabelRelation.Isomorphism))
