include("../000_basics/050_formoseGrammar.py")
glycolaldehyde.print()
# A graph G can be used to construct special rules:
# (\emptyset <- \emptyset -> G)
bindExp = rcBind(glycolaldehyde)
# (G <- \emptyset -> \emptyset)
unbindExp = rcUnbind(glycolaldehyde)
# (G <- G -> G)
idExp = rcId(glycolaldehyde)
# These are really rule composition expressions that have to be evaluated:
rc = RCEvaluator(inputRules)
# Each expression results in a lists of rules:
bindRules = rc.eval(bindExp)
unbindRules = rc.eval(unbindExp)
idRules = rc.eval(idExp)
post.summarySection("Bind Rules")
for p in bindRules:
	p.print()
post.summarySection("Unbind Rules")
for p in unbindRules:
	p.print()
post.summarySection("Id Rules")
for p in idRules:
	p.print()
# rst-name: Unary Operators
# rst: Special rules can be constructed from graphs.
