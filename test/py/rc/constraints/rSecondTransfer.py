r1 = ruleGML("rSecondTransfer_r1.gml")
r2 = ruleGML("rSecondTransfer_r2.gml")

for a in inputRules: a.print()

rc = RCEvaluator(inputRules)
rc.eval(r1 * rcSuper * r2)
rc.print()
for a in rc.createdRules: a.print()
