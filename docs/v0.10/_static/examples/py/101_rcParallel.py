include("050_formoseGrammar.py")
rc = rcEvaluator(inputRules)
# The special global object 'rcParallel' is used to make a pseudo-operator:
exp = rcId(formaldehyde) *rcParallel*  rcUnbind(glycolaldehyde)
rules = rc.eval(exp)
for p in rules:
	p.print()
# rst-name: Parallel Composition
# rst: A pair of rules can be merged to a new rule implementing the parallel
# rst: transformation.
