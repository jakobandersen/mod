include("../xxx_helpers.py")
include("../formoseCommon/grammar_H.py")

for a in inputRules:
	a.print()

r1 = Rule.fromDFS("[A]>>[B]", name="r1")
r2 = Rule.fromDFS("[B]>>[C]", name="r2")

config.rc.printMatches = True

def handleExp(exp, printRules=False):
	rc = RCEvaluator(inputRules)
	e = rcExp(exp)
	print("Expression:", e)
	res = rc.eval(e)
	print("Result:        ", res)
	print("Database:      ", rc.ruleDatabase)
	print("Created rules: ", rc.createdRules)
	if printRules:
		for a in rc.ruleDatabase:
			print("  Printing", a)
			a.print()
	post.summarySection("Created Rules")
	for a in rc.createdRules:
		print("  Printing", a)
		a.print()
	print("Printing", rc)
	rc.print()
	inputRules.extend(a for a in rc.createdRules)
