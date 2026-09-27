include("../xxx_helpers.py")

dg = DG()
dg.build().addAbstract("A -> B")
A = next(v for v in dg.vertices if v.graph.name == "A")
B = next(v for v in dg.vertices if v.graph.name == "B")
e = next(iter(dg.edges))
net = causality.Net(dg)
marking = causality.Marking(net)

def check(t, v, f):
	assert f(t) == f(v)


trace = causality.EventTrace(marking)
view = causality.EventTraceView(trace)

check(trace, view, lambda a: a.dg)

marking.add(A, 1)
check(trace, view, lambda a: a.initialState)
check(trace, view, lambda a: len(a))
check(trace, view, lambda a: a.time)

trace.add(1, causality.EdgeAction(e))
check(trace, view, lambda a: len(a))
check(trace, view, lambda a: a.time)
check(trace, view, lambda a: a[0].time)
check(trace, view, lambda a: isinstance(a[0].action, causality.EdgeAction))
check(trace, view, lambda a: a[0].action.edge)
check(trace, view, lambda a: str(trace[0]))

trace.add(42, causality.OutputAction(B))
check(trace, view, lambda a: len(a))
check(trace, view, lambda a: a.time)
check(trace, view, lambda a: a[1].time)
check(trace, view, lambda a: isinstance(a[1].action, causality.OutputAction))
check(trace, view, lambda a: a[1].action.vertex)
check(trace, view, lambda a: str(trace[1]))
check(trace, view, lambda a: list(a))
check(trace, view, lambda a: str(a))

check(trace, view, lambda a: a)
check(view, trace, lambda a: a)

view2 = causality.EventTraceView(trace)
assert view == view2

view.print()
view.print(causality.EventTracePrinter())
