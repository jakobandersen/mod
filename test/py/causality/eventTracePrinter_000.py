include("../xxx_helpers.py")

p = causality.EventTracePrinter()
assert p.maxPointsPerVertex == 500
assert not p.logTime
assert not p.logCount

fail(lambda: p.popOptions(), "No options callback to pop.")
fail(lambda: p.popVertexVisible(), "No vertex visible callback to pop.")
fail(lambda: p.popVertexOptions(), "No vertex options callback to pop.")
