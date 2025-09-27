include("stochsim_xx1_helpers.py")


def expandNetwork(b, s, u):
	b.addAbstract("A -> B")
	return False

def inputRate(v):
	if v.graph.name == "A":
		return (200, True)
	else:
		return (0, True)

sim = causality.Simulator(
	graphDatabase=inputGraphs,
	expandNetwork=expandNetwork,
	initialState={},
	draw=causality.Simulator.DrawMassAction(inputRate=inputRate),
)

setCallbacks(sim)

trace = sim.simulate(iterations=1000)
assert sim.dg.numVertices == 2, sim.dg.numVertices
assert sim.dg.numEdges == 1, sim.dg.numEdges
trace.print()
