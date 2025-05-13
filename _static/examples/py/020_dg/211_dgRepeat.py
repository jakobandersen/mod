include("../000_basics/050_formoseGrammar.py")
strat = (
	addUniverse(formaldehyde)
	>> addSubset(glycolaldehyde)
	# Iterate the rule application 4 times.
	>> repeat[4](
		inputRules
	)
)
dg = DG(graphDatabase=inputGraphs)
dg.build().execute(strat)
dg.print()
# rst-name: Repetition
# rst: A sub-strategy can be repeated.
