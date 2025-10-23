include("../xxx_helpers.py")

dgOther = DG()
dgOther.build().addAbstract("A -> B")
vOther = next(iter(dgOther.vertices))
eOther = next(iter(dgOther.edges))
netOther = causality.Net(dgOther)
markingOther = causality.Marking(netOther)

dg = DG()
dg.build().addAbstract("A -> B")
A = next(v for v in dg.vertices if v.graph.name == "A")
B = next(v for v in dg.vertices if v.graph.name == "B")
e = next(iter(dg.edges))
net = causality.Net(dg)
marking = causality.Marking(net)

fail(lambda: causality.EventTrace(marking).add(0, causality.EdgeAction()),
	"Can not add edge event with null edge.")
fail(lambda: causality.EventTrace(marking).add(0, causality.EdgeAction(eOther)),
	"The hyperedge in the edge action does not belong to the underlying derivation graph.")

fail(lambda: causality.EventTrace(marking).add(42, causality.OutputAction()),
	"Can not add output event with null vertex.")
fail(lambda: causality.EventTrace(marking).add(42, causality.OutputAction(vOther)),
	"The vertex in the output action does not belong to the underlying derivation graph.")

trace = causality.EventTrace(marking)
assert trace.initialState.net.dg == trace.dg
fail(lambda: trace.add(-1, causality.InputAction(A)),
	"Can not add event with time", isSubstring=True)
trace.add(0,  causality.InputAction(A))
fail(lambda: trace.add(-1, causality.InputAction(A)),
	"Can not add event with time", isSubstring=True)
trace.add(1,  causality.InputAction(A))
fail(lambda: trace.add(0, causality.InputAction(A)),
	"Can not add event with time", isSubstring=True)
trace.add(1,  causality.InputAction(A))

trace = causality.EventTrace(marking)
marking.add(A, 1)
assert trace.initialState[A] == 0
assert len(trace) == 0
assert trace.time == 0, trace.time
trace.add(1, causality.EdgeAction(e))
assert len(trace) == 1
assert trace[0].time == 1
assert trace.time == 1
assert isinstance(trace[0].action, causality.EdgeAction), trace[0].action
assert trace[0].action.edge == e
assert str(trace[0]) == "Event{{1, EdgeAction{{{}}}}}".format(e), str(trace[0])
trace.add(42, causality.OutputAction(B))
assert len(trace) == 2
assert trace[1].time == 42
assert trace.time == 42
assert isinstance(trace[1].action, causality.OutputAction), trace[1].action
assert trace[1].action.vertex == B
assert str(trace[1]) == "Event{{42, OutputAction{{{}}}}}".format(B)
assert list(trace) == [trace[0], trace[1]]
assert str(trace) == "EventTrace{{{}, [{}]}}".format(trace.initialState, ", ".join(str(e) for e in trace))


marking = causality.Marking(net)
marking.add(A, 1)
trace = causality.EventTrace(marking)
trace.add(1, causality.EdgeAction(e))
trace.add(42, causality.OutputAction(B))
m = trace.initialState
assert m[A] == 1
assert m[B] == 0
trace[0].action.applyTo(m)
assert m[A] == 0
assert m[B] == 1
trace[1].action.applyTo(m)
assert m[A] == 0
assert m[B] == 0
