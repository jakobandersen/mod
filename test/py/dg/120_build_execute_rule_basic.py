include("1xx_execute_helpers.py")

g1 = smiles('O', "g1")
g2 = smiles('C', "g2")
g3 = smiles('N', "g3")
r1string = ruleGMLString("""rule [
	ruleID "r1string"
	labelType "string"
	context [ node [ id 0 label "O" ] ]
]""")
r1term = ruleGMLString("""rule [
	ruleID "r1term"
	labelType "term"
	context [ node [ id 0 label "O" ] ]
]""")

exeStrat(DGStrat.makeRule(r1string), [], [], ls=lsString)
exeStrat(DGStrat.makeRule(r1term), [], [], ls=lsTerm)
exeStrat(DGStrat.makeRule(Rule.fromGMLString("rule []")), [], [])
fail(lambda: DGStrat.makeRule(None), "The rule is a null pointer.")
fail(lambda: exeStrat(DGStrat.makeRule(r1string), [], [], ls=lsTerm),
	"Rule 'r1string' has intended label type 'string', but the DG is using 'term'.")
fail(lambda: exeStrat(DGStrat.makeRule(r1term), [], [], ls=lsString),
	"Rule 'r1term' has intended label type 'term', but the DG is using 'string'.")
exeStrat(DGStrat.makeRule(r1string), [], [], ls=lsTerm,
	ignoreRuleLabelTypes=True)
exeStrat(DGStrat.makeRule(r1term), [], [], ls=lsString,
	ignoreRuleLabelTypes=True)
