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

dg = DG()
dg.build().addAbstract("A -> B")
net = causality.Net(dg)
marking = causality.Marking(net)

DMA(inputRate=None,  reactionRate=rRate, outputRate=oRate)(marking)
DMA(inputRate=iRate, reactionRate=None,  outputRate=oRate)(marking)
DMA(inputRate=iRate, reactionRate=rRate, outputRate=None )(marking)

A = next(v for v in dg.vertices if v.graph.name == "A")
B = next(v for v in dg.vertices if v.graph.name == "B")
e = next(iter(dg.edges))
marking.add(A, 1)

mak = makc(marking)

choice, reactivity = mak.draw()
assert choice.asAction() == causality.EdgeAction(e), choice
assert reactivity == 1

marking = causality.Marking(net)
mak = makc(marking)
choice, reactivity = mak.draw()
assert choice.asAction() is None, choice
assert reactivity == 0

marking = causality.Marking(net)
mak = DMA(
	inputRate=lambda v: (1, True) if v == B else (0, True),
	reactionRate=rRate,
	outputRate=oRate)(marking)
choice, reactivity = mak.draw()
assert choice.asAction() == causality.InputAction(B), choice
assert reactivity == 1

marking = causality.Marking(net)
mak = DMA(
	inputRate=iRate,
	reactionRate=rRate,
	outputRate=(1, True))(marking)
choice, reactivity = mak.draw()
assert choice.asAction() is None
assert reactivity == 0


print("=================")
marking = causality.Marking(net)
marking.add(B, 1)
mak = DMA(
	inputRate=iRate,
	reactionRate=rRate,
	outputRate=(1, True))(marking)
choice, reactivity = mak.draw()
assert choice.asAction() == causality.OutputAction(B), choice
assert reactivity == 1
choice, reactivity = mak.draw()
assert choice.asAction() == causality.OutputAction(B), choice
assert reactivity == 1
