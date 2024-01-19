# A few imports for type annotations.
from typing import List, Tuple

btToOrder = {
	BondType.Single: 1,
	BondType.Double: 2,
	BondType.Triple: 3,
	BondType.Aromatic: 0,
}

# Our rudimentary chemical valence rules.
maxValence = {
	'H': 1,
	'C': 4,
	'O': 2,
}


def valence(v: Graph.Vertex) -> int:
	return sum(btToOrder[e.bondType] for e in v.incidentEdges)


def checkRule(r: Rule, verbose: bool = False) -> bool:
	if verbose:
		print("Checking Rule", r)
	for v in r.vertices:
		if valence(v.left) != valence(v.right):
			# this shouldn't happen if the input rules are chemical
			raise ValueError("degree change")

		if valence(v.left) > maxValence[v.left.stringLabel]:
			if verbose:
				print("Bad Rule, ---> chemical degree constraint")
			return False
	return True


CompRes = Tuple[List[Rule], List[Rule]]  # alias for type annotation


# Partitions the rules into those that are chemical, according to the
# rules above, and those that are not.
def checkRules(rules: List[Rule]) -> CompRes:
	good = []
	bad = []

	for r in rules:
		try:
			res = checkRule(r)
		except ValueError:
			r.print()
			raise
		if res:
			good.append(r)
		else:
			bad.append(r)
	return good, bad


def compose(r1: Rule, r2: Rule, rc: RCEvaluator) -> Tuple[CompRes, CompRes]:
	# Set up the composition operator we use, i.e., all overlaps.
	# See also http://jakobandersen.github.io/mod/pymod/rule/Composition.html
	comp = rcCommon(connected=False, maximum=False)
	# Note: this operator does not enumerate the empty overlap, so below we will
	#       perform the parallel composition explicitly.

	# Enable debug visualisation of each successful composition.
	# The visualisations will be inserted in the summary.
	# They form the basis of the figures in Appendix F.
	config.rc.printMatches = True
	# Print vertex indices in those visualilzations.
	config.rc.matchesWithIndex = True
	# But skip the compositions that are not chemical, according to the same
	# scheme shown above (but implemented internally as well for this purpose).
	config.rc.printMatchesOnlyHaxChem = True

	# Compute the composition of r1 with r2.
	res12 = checkRules(
		rc.eval(rcExp([
			r1 *rcParallel* r2,  # the empty overlap
			r1 *comp* r2,        # all other overlaps
		]))
	)

	# And now the same, but with the rules in the opposite order.
	res21 = checkRules(
		rc.eval(rcExp([
			r2 *rcParallel* r1,
			r2 *comp* r1,
		]))
	)

	# In the summary there will be a section "RC Matches" for each composition
	# operator, with the results of it. Thus, there will be 4 such sections:
	# - For r1 *rcParallel* r2,
	# - for r1 *comp* r2,
	# - for r2 *rcParallel* r1, and
	# - for r2 *comp* r1.
	#
	# Note: these figures are created before rule isomorphism checks in the
	#       evaluator, and the rule names/IDs are thus different from those
	#       in the sections printed by handleResult().
	return res12, res21


# Take the reuslt of compose() and populate the summary with figures.
def handleResult(r1: Rule, r2: Rule, res: Tuple[CompRes, CompRes],
		*, printInput: bool = True, printGood: bool = True,
		printBad: bool = False) -> None:
	# Set up settings for customizing the depiction of graphs.
	# See http://jakobandersen.github.io/mod/pymod/graph/Printer.html#mod.GraphPrinter
	p = GraphPrinter()
	p.setReactionDefault()
	p.collapseHydrogens = False
	pIndex = GraphPrinter()
	pIndex.setReactionDefault()
	pIndex.withIndex = True

	def printRules(rs, txt):
		post.summaryChapter(txt)
		if printGood:
			post.summarySection("Good")
			for r in rs[0]:
				r.print(p)
		if printBad:
			post.summarySection("Bad")
			for r in rs[1]:
				r.print(p)

	# Insert visualizations of the input into the summary.
	if printInput:
		post.summaryChapter("Input")
		r1.print(p)
		r2.print(p)
	# Insert visualisations of the results into the summary.
	printRules(res[0], "r{}, r{}".format(str(r1.id), str(r2.id)))
	printRules(res[1], "r{}, r{}".format(str(r2.id), str(r1.id)))

	# Print the data for the table from Appendix E.
	allRules = set(res[0][0])
	allRules.update(res[1][0])
	count12 = {r: 0 for r in allRules}
	count21 = {r: 0 for r in allRules}
	for r in res[0][0]:
		count12[r] += 1
	for r in res[1][0]:
		count21[r] += 1
	diff = {r: count21[r] - count12[r] for r in allRules}

	print("numRules12:", len(res[0][0]))
	print("numRules21:", len(res[1][0]))
	print("{:<20}: {} - {} = {}".format(
		"composed rule name", "count 21", "count 12", "diff"))
	for r in sorted(allRules, key=lambda r: r.id):
		print("{:<20}: {} - {} = {: d}".format(
			str(r), count21[r], count12[r], diff[r]))


###############################################
# The main script using the functionality above
###############################################

def compute(r1: Rule, r2: Rule, rc: RCEvaluator):
	# In order to evaluate rule composition expressions we need an evaluator
	# object.
	# See also # http://jakobandersen.github.io/mod/pymod/rule/Composition.html#mod.rcEvaluator
	# This object will also perform isomorphism checks between rules, such that
	# all rules it creates are unique up to isomorphism. Thus, if two Rule
	# objects do not compare equal, then they are not isomorphic by this
	# invariant.

	# Use our machinery from above:
	post.summaryChapter("UseBoostCommonSubgraph = True")
	print("UseBoostCommonSubgraph = True")
	print("=" * 80)
	config.rc.useBoostCommonSubgraph = True
	res = compose(r1, r2, rc)
	handleResult(r1, r2, res)

	post.summaryChapter("UseBoostCommonSubgraph = False")
	print("UseBoostCommonSubgraph = False")
	print("=" * 80)
	config.rc.useBoostCommonSubgraph = False
	res = compose(r1, r2, rc)
	handleResult(r1, r2, res)

	post.disableInvokeMake()  # it takes quite a while due to all the matches

# First load our two rules:
# - an identity rule with the left-hand side of the Aldol Addition rule, and
# - the Aldol Addition rule.

aldolAdd_F_id = ruleGMLString("""
rule [
	#ruleID "Aldol Addition ->, id"
	context [
		edge [ source 1 target 2 label "=" ]
		edge [ source 2 target 3 label "-" ]
		edge [ source 3 target 4 label "-" ]
		edge [ source 5 target 6 label "=" ]
		node [ id 1 label "C" ]
		node [ id 2 label "C" ]
		node [ id 3 label "O" ]
		node [ id 4 label "H" ]
		node [ id 5 label "O" ]
		node [ id 6 label "C" ]
	]
]
""")
aldolAdd_F_id_noEdge = ruleGMLString("""
rule [
	#ruleID "Aldol Addition ->, id"
	context [
		edge [ source 1 target 2 label "=" ]
		edge [ source 2 target 3 label "-" ]
		edge [ source 3 target 4 label "-" ]
		edge [ source 5 target 6 label "=" ]

		edge [ source 6 target 1 label ":" ]
		node [ id 1 label "C" ]
		node [ id 2 label "C" ]
		node [ id 3 label "O" ]
		node [ id 4 label "H" ]
		node [ id 5 label "O" ]
		node [ id 6 label "C" ]
	]
]
""")

aldolAdd_F = ruleGMLString("""
rule [
	#ruleID "Aldol Addition ->"
	left [
		edge [ source 1 target 2 label "=" ]
		edge [ source 2 target 3 label "-" ]
		edge [ source 3 target 4 label "-" ]
		edge [ source 5 target 6 label "=" ]
	]
	context [
		node [ id 1 label "C" ]
		node [ id 2 label "C" ]
		node [ id 3 label "O" ]
		node [ id 4 label "H" ]
		node [ id 5 label "O" ]
		node [ id 6 label "C" ]
	]
	right [
		edge [ source 1 target 2 label "-" ]
		edge [ source 2 target 3 label "=" ]
		edge [ source 5 target 6 label "-" ]

		edge [ source 4 target 5 label "-" ]
		edge [ source 6 target 1 label "-" ]
	]
]
""")
aldolAdd_F_noEdge = ruleGMLString("""
rule [
	#ruleID "Aldol Addition ->"
	left [
		edge [ source 1 target 2 label "=" ]
		edge [ source 2 target 3 label "-" ]
		edge [ source 3 target 4 label "-" ]
		edge [ source 5 target 6 label "=" ]

		edge [ source 6 target 1 label ":" ]
	]
	context [
		node [ id 1 label "C" ]
		node [ id 2 label "C" ]
		node [ id 3 label "O" ]
		node [ id 4 label "H" ]
		node [ id 5 label "O" ]
		node [ id 6 label "C" ]
	]
	right [
		edge [ source 1 target 2 label "-" ]
		edge [ source 2 target 3 label "=" ]
		edge [ source 5 target 6 label "-" ]

		edge [ source 4 target 5 label "-" ]
		edge [ source 6 target 1 label "-" ]
	]
]
""")


rc = rcEvaluator(inputRules)

post.summaryChapter("No no-edge 'constraints'")
print("No no-edge 'constraints'")
print("#" * 80)
compute(aldolAdd_F_id, aldolAdd_F, rc)

post.summaryChapter("No-edge 'constraints'")
print("No-edge 'constraints'")
print("#" * 80)
compute(aldolAdd_F_id_noEdge, aldolAdd_F_noEdge, rc)

#post.enableInvokeMake()
