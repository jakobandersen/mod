include("eventTrace_05x_print_common.py")

dg = DG()
dg.build().addAbstract("A -> B")
A = next(v for v in dg.vertices if v.graph.name == "A")
B = next(v for v in dg.vertices if v.graph.name == "B")
e = dg.findEdge([A], [B])
assert e
net = causality.Net(dg)
k = 5000
for offset in (0, 999, 9999):
	marking = causality.Marking(net)
	marking.add(A, k)
	trace = causality.EventTrace(marking)
	for i in range(1, k):
		trace.add(i + offset, causality.EdgeAction(e))
	tp = causality.EventTracePrinter()
	tp.maxPointsPerVertex = 10
	doPrint(trace, tp=tp, caption=f"Offset {offset}")
