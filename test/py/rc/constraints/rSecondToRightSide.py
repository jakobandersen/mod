r1 = Rule.fromGMLFile("rSecondToRightSide_r1.gml")
r2 = Rule.fromGMLFile("rSecondToRightSide_r2.gml")

for a in inputRules: a.print()

rc = RCEvaluator(inputRules)
rc.eval(r1 * rcSuper * r2)
rc.print()
for a in rc.createdRules: a.print()
