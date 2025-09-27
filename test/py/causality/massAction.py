include("../xxx_helpers.py")

DMA = causality.Simulator.DrawMassAction

dgOther = DG()
dgOther.build().addAbstract("A -> B")
netOther = causality.Net(dgOther)
markingOther = causality.Marking(netOther)

iRate = lambda v: (0, True)
rRate = lambda e: (1, True)
oRate = lambda v: (0, True)

makc = DMA(inputRate=iRate, reactionRate=rRate, outputRate=oRate)
fail(lambda: makc(None), "The derivation graph is a null pointer.")
fail(lambda: makc(DG()), "The DG neither has an active builder nor is locked yet.")

dg = DG()
dg.build().addAbstract("A -> B")

DMA(inputRate=None,  reactionRate=rRate, outputRate=oRate)(dg)
DMA(inputRate=iRate, reactionRate=None,  outputRate=oRate)(dg)
DMA(inputRate=iRate, reactionRate=rRate, outputRate=None )(dg)

A = next(v for v in dg.vertices if v.graph.name == "A")
B = next(v for v in dg.vertices if v.graph.name == "B")
e = next(iter(dg.edges))
net = causality.Net(dg)
marking = causality.Marking(net)
marking.add(A, 1)

mak = makc(dg)
fail(lambda: mak.draw(markingOther),
	"The marking is not on the underlying derivation graph.")

action, reactivity = mak.draw(marking)
assert action == causality.EdgeAction(e), action
assert reactivity == 1

marking = causality.Marking(net)
action, reactivity = mak.draw(marking)
assert action is None
assert reactivity == 0

mak = DMA(
	inputRate=lambda v: (1, True) if v == B else (0, True),
	reactionRate=rRate,
	outputRate=oRate)(dg)
marking = causality.Marking(net)
action, reactivity = mak.draw(marking)
assert action == causality.InputAction(B), action
assert reactivity == 1

mak = DMA(
	inputRate=iRate,
	reactionRate=rRate,
	outputRate=(1, True))(dg)
marking = causality.Marking(net)
marking.add(B, 1)
action, reactivity = mak.draw(marking)
assert action == causality.OutputAction(B), action
assert reactivity == 1
