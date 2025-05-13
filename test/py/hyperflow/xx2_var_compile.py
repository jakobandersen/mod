post.disableInvokeMake()

dg = DG()
dg.build().addAbstract("A -> B AA <=> BB")
v = next(iter(dg.vertices))
g = v.graph
es = list(dg.edges)
e = es[0]
eForward = es[1]
eBackward = es[2]
assert eForward.inverse == eBackward

dgOther = DG()
dgOther.build().addAbstract("C -> D")
vOther = next(iter(dgOther.vertices))
gOther = vOther.graph
eOther = next(iter(dgOther.edges))

def ok(v, alsoRelaxed, setup):
	flow = hyperflow.Model(dg)
	for a in dg.vertices:
		flow.separateIOInternalTransit(a)
	flow.addConstraint(v == 0)
	setup(flow)
	flow.findSolutions()
	if not alsoRelaxed:
		return
	flow = hyperflow.Model(dg)
	for a in dg.vertices:
		flow.separateIOInternalTransit(a)
	flow.relaxed = True
	flow.addConstraint(v == 0)
	setup(flow)
	flow.findSolutions()

def fail(v, alsoRelaxed, setup, suffix):
	flow = hyperflow.Model(dg)
	for a in dg.vertices:
		flow.separateIOInternalTransit(a)
	flow.addConstraint(v == 0)
	setup(flow)
	try:
		flow.findSolutions()
		assert False
	except LogicError as e:
		if str(e).endswith(suffix):
			pass
		else:
			print("Expected suffix:", suffix)
			raise
	flow = hyperflow.Model(dg)
	for a in dg.vertices:
		flow.separateIOInternalTransit(a)
	flow.relaxed = True
	flow.addConstraint(v == 0)
	if not alsoRelaxed:
		return
	setup(flow)
	try:
		flow.findSolutions()
		assert False
	except LogicError as e:
		if str(e).endswith(suffix):
			pass
		else:
			print("Expected suffix:", suffix)
			raise

def checkVertexName(name, alsoRelaxed, setup):
	ok(hyperflow.VarSumVertex(name), alsoRelaxed, setup)
	ok(hyperflow.VarVertex(name, v), alsoRelaxed, setup)
	fail(hyperflow.VarVertex(name, vOther), alsoRelaxed, setup,
		"is not from the underlying derivation graph.")
	ok(hyperflow.VarVertexGraph(name, g), alsoRelaxed, setup)
	fail(hyperflow.VarVertexGraph(name, gOther), alsoRelaxed, setup,
		"is not represented in the underlying derivation graph.")
	fail(hyperflow.VarSumEdge(name), alsoRelaxed, setup,
		"is not an edge set, but a vertex set.")
	fail(hyperflow.VarEdge(name, e), alsoRelaxed, setup,
		"is not an edge set, but a vertex set.")
	fail(hyperflow.VarEdge(name, eOther), alsoRelaxed, setup,
		"is not an edge set, but a vertex set.")
	fail(hyperflow.VarSumCustom(name), alsoRelaxed, setup,
		"is not a custom set, but a vertex set.")
	fail(hyperflow.VarCustom(name, "varName"), alsoRelaxed, setup,
		"is not a custom set, but a vertex set.")
def checkEdgeName(name, alsoRelaxed, setup):
	fail(hyperflow.VarSumVertex(name), alsoRelaxed, setup,
		"is not a vertex set, but a(n) edge set.")
	fail(hyperflow.VarVertex(name, v), alsoRelaxed, setup,
		"is not a vertex set, but a(n) edge set.")
	fail(hyperflow.VarVertex(name, vOther), alsoRelaxed, setup,
		"is not a vertex set, but a(n) edge set.")
	fail(hyperflow.VarVertexGraph(name, g), alsoRelaxed, setup,
		"is not a vertex set, but a(n) edge set.")
	fail(hyperflow.VarVertexGraph(name, gOther), alsoRelaxed, setup,
		"is not a vertex set, but a(n) edge set.")
	ok(hyperflow.VarSumEdge(name), alsoRelaxed, setup)
	if name == 'isBothReverseUsed':
		fail(hyperflow.VarEdge(name, e), alsoRelaxed, setup,
			"It does not have an inverse.")
		ok(hyperflow.VarEdge(name, eForward), alsoRelaxed, setup)
		ok(hyperflow.VarEdge(name, eBackward), alsoRelaxed, setup)
	else:
		ok(hyperflow.VarEdge(name, e), alsoRelaxed, setup)
	fail(hyperflow.VarEdge(name, eOther), alsoRelaxed, setup,
		"is not from the underlying derivation graph.")
	fail(hyperflow.VarSumCustom(name), alsoRelaxed, setup,
		"is not a custom set, but a edge set.")
	fail(hyperflow.VarCustom(name, "varName"), alsoRelaxed, setup,
		"is not a custom set, but a edge set.")
def checkCustomName(name, alsoRelaxed, setup):
	fail(hyperflow.VarSumVertex(name), alsoRelaxed, setup,
		"is not a vertex set, but a(n) custom set.")
	fail(hyperflow.VarVertex(name, v), alsoRelaxed, setup,
		"is not a vertex set, but a(n) custom set.")
	fail(hyperflow.VarVertex(name, vOther), alsoRelaxed, setup,
		"is not a vertex set, but a(n) custom set.")
	fail(hyperflow.VarVertexGraph(name, g), alsoRelaxed, setup,
		"is not a vertex set, but a(n) custom set.")
	fail(hyperflow.VarVertexGraph(name, gOther), alsoRelaxed, setup,
		"is not a vertex set, but a(n) custom set.")
	fail(hyperflow.VarSumEdge(name), alsoRelaxed, setup,
		"is not an edge set, but a custom set.")
	fail(hyperflow.VarEdge(name, e), alsoRelaxed, setup,
		"is not an edge set, but a custom set.")
	fail(hyperflow.VarEdge(name, eOther), alsoRelaxed, setup,
		"is not an edge set, but a custom set.")
	ok(hyperflow.VarSumCustom(name), alsoRelaxed, setup)


def checkModule(module, setup=lambda f: None):
	for typ, name, alsoRelaxed, solCheck in varRegistry[module]:
		if typ == "vertex":
			checkVertexName(name, alsoRelaxed, setup)
		elif typ == "edge":
			checkEdgeName(name, alsoRelaxed, setup)
		elif typ == "custom":
			checkCustomName(name, alsoRelaxed, setup)
		else:
			assert False
