include("xxx_common.py")

makeFlow("A -> B")

flow.findSolutions(maxNumSolutions=0)
view = flow.implementationView

fail(lambda: view.printExpandedVertex(DG.Vertex()), "Can not print expanded network for a null vertex.")
makeDG("C -> D")
fail(lambda: view.printExpandedVertex(next(iter(dg.vertices))), "Can not print expanded network for a vertex from a different derivation graph.")
fail(lambda: view.printExpandedVertex(C.graph), "Can not print expanded network for a null vertex.")

makeFlow("A -> B")

post.enableInvokeMake()

def printStuff():
	flow.findSolutions()
	s = flow.implementationView.printExpandedVertex(O)
	assert type(s) == str
	flow.implementationView.printExpanded()

post.summarySection("O -> A")
makeFlow("O -> A")
printStuff()

post.summarySection("A -> O")
makeFlow("A -> O")
printStuff()

# Sources/Sinks
# =================================================================
post.summarySection("O -> B, source O")
makeFlow("O -> B")
printStuff()

post.summarySection("A -> O, sink O")
makeFlow("A -> O")
printStuff()

# AllowHyperLoops
# =================================================================
post.summarySection("O -> O, not hyperloops")
makeFlow("O -> O")
assert not flow.allowHyperLoops
printStuff()

post.summarySection("O -> O, hyperloops")
makeFlow("O -> O")
flow.allowHyperLoops = True
printStuff()

# AllowReversal
# =================================================================
post.summarySection("O <=> B, not reversal")
makeFlow("O <=> B")
assert not flow.allowReversal
printStuff()

post.summarySection("O <=> B, reversal")
makeFlow("O <=> B")
flow.allowReversal = True
printStuff()

# AllowIOReversal
# =================================================================
post.summarySection("O -> B, not IO reversal")
makeFlow("O -> B")
flow.allowIOReversal = False
printStuff()

post.summarySection("O -> B, IO reversal")
makeFlow("O -> B")
assert flow.allowIOReversal
printStuff()

# Relaxed
# =================================================================
post.summarySection("Relaxed")
makeFlow("""
	A + B -> O
	B + C -> O
	C + A -> O
""")
flow.relaxed = True
printStuff()

# VertexFilter
# =================================================================
post.summarySection("Vertex filter")
makeFlow("A -> O O -> C")
flow.exclude(O)
printStuff()

# Multiedges
# =================================================================
for i in range(2, 6):
	post.summarySection("%d O -> B, multiedges" % i)
	makeFlow("%d O -> B" % i)
	printStuff()

	post.summarySection("A -> %d O, multiedges" % i)
	makeFlow("A -> %d O" % i)
	printStuff()
