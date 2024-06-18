include("212_dgPredicate.py")
# Create a printer with default options:
p = DGPrinter()
# Hide "large" molecules: those with > 4 Cs:
p.pushVertexVisible(lambda v: v.graph.vLabelCount("C") <= 4)
# Hide the reactions with the large molceules as well:
def edgePred(e):
	if any(v.graph.vLabelCount("C") > 4 for v in e.sources): return False
	if any(v.graph.vLabelCount("C") > 4 for v in e.targets): return False
	return True
p.pushEdgeVisible(edgePred)
# Add the number of Cs to the molecule labels:
p.pushVertexLabel(lambda v: "#C=" + str(v.graph.vLabelCount("C")))
# Highlight the molecules with 4 Cs:
p.pushVertexColour(lambda v: "blue" if v.graph.vLabelCount("C") == 4 else "")
# Print the network with the customised printer.
dg.print(p)
# rst-name: Advanced Printing
# rst: Reaction networks can become large, and often it is necessary to hide
# rst: parts of the network, or in general change the appearance.
