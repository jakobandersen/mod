config.graph.useWrongSmilesCanonAlg = False
config.graph.isomorphismAlg = Config.IsomorphismAlg.Canon
config.graph.smilesCheckAST = True
#config.canon.printStats = True
include("loadGraphs.py")

post.disableInvokeMake()

if not "n" in globals():
	n = 100
print("Permutations:", n)

gCopy = list(inputGraphs)
print("Processing %d graphs" % len(inputGraphs))
for a in gCopy:
	print("Processing graph", a.id, end="")
	try:
		print(" with smiles \"{}\"".format(a.smiles))
	except LogicError as e:
		if str(e).startswith("Graph {} with name '{}' is not a molecule.".format(a.id, a.name)):
			print("\tskipping, not a molecule")
			continue
		else:
			raise
	b = Graph.fromSMILES(a.smiles, allowAbstract=True)
	inputGraphs[:] = [] # let's not keep the copies around, just for good measure
	if a.isomorphism(b) != 1:
		print("ERROR: smiles does not represent same molecule")
		print("a.smiles =", a.smiles)
		print("b.smiles =", b.smiles)
		a.name = "a"
		b.name = "b"
		graphLike = GraphPrinter()
		molLike = GraphPrinter()
		graphLike.withIndex = True
		molLike.setMolDefault()
		molLike.withIndex = True
		a.print(graphLike, molLike)
		b.print(graphLike, molLike)
		post.enableInvokeMake()
		sys.exit(1)
	for i in range(1, n):
		aPerm = a.makePermutation()
		if a.smiles != aPerm.smiles:
			print("ERROR:")
			print(a.smiles)
			print("!=")
			print(aPerm.smiles)
			a.name = "a"
			aPerm.name = "aPerm"
			a.print()
			aPerm.print()
			post.enableInvokeMake()
			sys.exit(1)
		ls = LabelSettings(LabelType.String, LabelRelation.Isomorphism)
		if a.isomorphism(aPerm, labelSettings=ls) != 1:
			print("ERROR in canonicalisation compare")
			sys.exit(1)
