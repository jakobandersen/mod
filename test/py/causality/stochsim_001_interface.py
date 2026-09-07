include("stochsim_xx1_helpers.py")

g = Graph.fromSMILES('O')
h = Graph.fromSMILES('C')

sim = causality.Simulator(
	graphDatabase=inputGraphs,
	expandNetwork=lambda b, s, u: False,
	initialState={g: 42},
)
assert sim.state(g) == 42
assert sim.state(sim.dg.findVertex(g)) == 42
fail(lambda: sim.state(h), "Can not find vertex with the graph in the underlying derivation graph.")

sim.setOnIterationBegin(None, 1)
fail(lambda: sim.setOnIterationBegin(None, 0), "Can not set callback with non-positive interval.")
fail(lambda: sim.setOnIterationBegin(None, -1), "Can not set callback with non-positive interval.")
sim.onDeadlock = None
sim.onExpand = None
