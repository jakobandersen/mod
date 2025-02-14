include("../000_basics/050_formoseGrammar.py")
rc = RCEvaluator(inputRules)
exp = rcId(formaldehyde) *rcParallel*  rcId(glycolaldehyde)
exp = exp *rcSuper* ketoEnol_F
rules = rc.eval(exp)
for p in rules:
	p.print()
# rst-name: Supergraph Composition
# rst: A pair of rules can (maybe) be composed using a sueprgraph relation.
