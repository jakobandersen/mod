include("xxx_common.py")

checkDeprecated(lambda: FlowPrinter())

p = hyperflow.Printer()
assert type(p.dgPrinter) == DGPrinter
assert p.withFlowLabels

fail(lambda: p.popInEdgeLabel(), "No in-edge label callback to pop.")
fail(lambda: p.popOutEdgeLabel(), "No out-edge label callback to pop.")

assert not p.printUnfiltered
assert p.printFiltered
assert p.unfilteredFlowColour == "blue"
