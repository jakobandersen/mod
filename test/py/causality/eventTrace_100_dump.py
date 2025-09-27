include("../xxx_helpers.py")

dgOther = DG()
dgOther.build().addAbstract("A -> B")
netOther = causality.Net(dgOther)

dg = DG()
dg.build().addAbstract("A -> B B -> C")
A = next(v for v in dg.vertices if v.graph.name == "A")
B = next(v for v in dg.vertices if v.graph.name == "B")
C = next(v for v in dg.vertices if v.graph.name == "C")
e1, e2 = dg.edges
net = causality.Net(dg)


# EventTrace.dump

marking = causality.Marking(net)
trace = causality.EventTrace(marking)

f = trace.dump()
assert f.startswith('out/')
assert f.endswith('_EventTrace.eventTrace')

f = trace.dump('')
assert f.startswith('out/')
assert f.endswith('_EventTrace.eventTrace')

f = trace.dump("out/myFilename.eventTrace")
assert f == "out/myFilename.eventTrace"
fail(lambda: trace.dump("/dev/null/EventTrace.eventTrace"),
	"Can not open file '/dev/null/EventTrace.eventTrace'.")


# EventTrace.load

marking = causality.Marking(net)
trace = causality.EventTrace(marking)
f = trace.dump()

fail(lambda: causality.EventTrace.load(None, f),
	"The Petri net is a null pointer.")
fail(lambda: causality.EventTrace.load(net, 'doesNotExist.eventTrace'),
	"EventTrace load error: Could not open file",
	err=InputError, isSubstring=True)

causality.EventTrace.load(net, CWDPath(f))


def check(t):
	f = t.dump()
	t2 = causality.EventTrace.load(net, CWDPath(f))
	if t != t2:
		showDump(CWDPath(f))
	assert len(t) == len(t2), (len(t), len(t2))
	assert t.initialState == t2.initialState, "{}, {}".format(t.initialState, t2.initialState)
	assert t.time == t2.time
	for i in range(len(t)):
		assert t[i] == t2[i], (t[i], t2[i])
	assert t == t2


check(causality.EventTrace(causality.Marking(net)))

m = causality.Marking(net)
m.add(A, 1)
m.add(B, 2)
m.add(C, 3)
check(causality.EventTrace(m))

t = causality.EventTrace(causality.Marking(net))
t.add(0, causality.InputAction(A))
t.add(5, causality.EdgeAction(e1))
t.add(42, causality.EdgeAction(e2))
t.add(60, causality.OutputAction(C))
check(t)


f = t.dump()
dgOther = DG()
dgOther.build().addAbstract("A <=> B + C")
netOther = causality.Net(dgOther)
fail(lambda: causality.EventTrace.load(netOther, CWDPath(f)),
	"EventTrace load error: DG mismatch at vertex 2.",
	err=InputError)
