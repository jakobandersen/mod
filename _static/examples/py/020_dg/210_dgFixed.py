include("../000_basics/050_formoseGrammar.py")
# Reaction networks are expaned using a strategy:
strat = (
	# A molecule can be active or passive during evaluation.
	addUniverse(formaldehyde) # passive
	>> addSubset(glycolaldehyde) # active
	# Aach reaction must have a least 1 active educt.
	>> inputRules
)
# We call a reaction network a 'derivation graph'.
dg = DG(graphDatabase=inputGraphs)
dg.build().execute(strat)
# They can also be visualised.
dg.print()
# rst-name: Rule Application
# rst: Transformation rules (reaction patterns) can be applied to graphs
# rst: (molecules) to create new graphs (molecules). The transformations
# rst: (reactions) implicitly form a directed (multi-)hypergraph (chemical
# rst: reaction network).
