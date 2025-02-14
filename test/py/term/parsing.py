include("../xxx_helpers.py")

ls = LabelSettings(LabelType.Term, LabelRelation.Unification)

g = Graph.fromDFS("[C/][C]")
r = Rule.fromGMLString("""rule [ context [ node [ id 0 label "C/" ] ] ]""")

for a in inputGraphs:
	fail(lambda: a.printTermState(), "Parsing failed", err=TermParsingError,
		isSubstring=True)
for a in inputRules:
	fail(lambda: a.printTermState(), "Parsing failed", err=TermParsingError,
		isSubstring=True)

fail(lambda: DG(graphDatabase=inputGraphs, labelSettings=ls).build().execute(addSubset(lambda: [])),
	"Parsing failed", err=TermParsingError, isSubstring=True)
fail(lambda: DG(graphDatabase=[], labelSettings=ls).build().execute(addSubset(inputGraphs)),
	"Parsing failed", err=TermParsingError, isSubstring=True)
dg = DG(graphDatabase=[], labelSettings=ls)
fail(lambda: dg.build().execute(addSubset(lambda: inputGraphs)),
	"Parsing failed", err=TermParsingError, isSubstring=True)
dg = DG(graphDatabase=[], labelSettings=ls)
fail(lambda: dg.build().execute(inputRules),
	"Parsing failed", err=TermParsingError, isSubstring=True)
fail(lambda: g.isomorphism(g, 1, ls),
	"Parsing failed", err=TermParsingError, isSubstring=True)
fail(lambda: g.monomorphism(g, 1, ls),
	"Parsing failed", err=TermParsingError, isSubstring=True)
fail(lambda: r.isomorphism(r, 1, ls),
	"Parsing failed", err=TermParsingError, isSubstring=True)
fail(lambda: r.monomorphism(r, 1, ls),
	"Parsing failed", err=TermParsingError, isSubstring=True)
fail(lambda: RCEvaluator(inputRules, ls),
	"Parsing failed", err=TermParsingError, isSubstring=True)
rc = RCEvaluator([], ls)
fail(lambda: rc.eval(inputRules *rcSuper* inputRules),
	"Parsing failed", err=TermParsingError, isSubstring=True)
