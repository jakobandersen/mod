include("../000_basics/050_formoseGrammar.py")
strat = (
	addUniverse(formaldehyde)
	>> addSubset(glycolaldehyde)
	# Constrain the reactions:
	# No molecules with more than 20 atoms can be created.
	>> rightPredicate[
		lambda derivation: all(g.numVertices <= 20 for g in derivation.right)
	](
		# Iterate until nothing new is found.
		repeat(
			inputRules
		)
	)
)
dg = DG(graphDatabase=inputGraphs)
dg.build().execute(strat)
dg.print()
# rst-name: Application Constraints
# rst: We may want to impose constraints on which reactions are accepted.
# rst: E.g., in formose the molecules should not have too many carbon atoms.
