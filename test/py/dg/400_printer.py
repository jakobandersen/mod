include("../xxx_helpers.py")

# nested GraphPrinter
assert type(DGPrinter().graphPrinter) == GraphPrinter
gp = GraphPrinter()
gp.enableAll()
gp.withIndex = False
assert DGPrinter().graphPrinter == gp
# check that assignment is a true copy
gp = GraphPrinter()
assert not gp.withIndex
p = DGPrinter()
p.graphPrinter = gp
gp.withIndex = True
assert gp.withIndex
assert not p.graphPrinter.withIndex

# And now the rest of the DGPrinter

p = DGPrinter()
assert p.withShortcutEdges
assert p.withGraphImages
assert p.labelsAsLatexMath

# push/pop vertexVisible
fail(lambda: p.popVertexVisible(), "No vertex visible callback to pop.")
# push/pop edgeVisible
fail(lambda: p.popEdgeVisible(), "No edge visible callback to pop.")

assert not p.withShortcutEdgesAfterVisibility
assert p.vertexLabelSep == ", "
assert p.edgeLabelSep == ", "

# push/pop vertexLabel
fail(lambda: p.popVertexLabel(), "No vertex label callback to pop.")
# push/pop edgeLabel
fail(lambda: p.popEdgeLabel(), "No edge label callback to pop.")

assert p.withGraphName
assert not p.withRuleName
assert p.withRuleId
assert not p.withInlineGraphs

# push/pop vertexColour
fail(lambda: p.popVertexColour(), "No vertex colour callback to pop.")
# push/pop edgeColour
fail(lambda: p.popEdgeColour(), "No edge colour callback to pop.")

# setRotationOverwrite
# setMirrorOverwrite

# rendering engine stuff
assert p.graphvizPrefix == ""
assert p.tikzpictureOption == r"scale=\modDGHyperScale"
