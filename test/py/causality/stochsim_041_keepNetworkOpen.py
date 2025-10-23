include("stochsim_xx1_helpers.py")

# With a static network we should be able to simulate multiple times,
# as it never calls the expansion again.

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

sim.simulate(iterations=1000)
sim.simulate(iterations=1000)

# But with a dynamic expansion we should not be able to simulate again,
# unless the network is kept open.

A = Graph.fromDFS("[A]")
Rule.fromDFS("[A]>>[B]")

def makeSim():
	return causality.Simulator(
		graphDatabase=inputGraphs,
		expandNetwork=causality.Simulator.ExpandByStrategy(inputRules),
		initialState={A: 1000},
		draw=causality.Simulator.DrawMassAction()
	)

sim = makeSim()
sim.simulate()
fail(lambda: sim.simulate(), "Can not expand neighbourhood, the network is closed.", isSubstring=True)

sim = makeSim()
sim.simulate(keepNetworkOpen=True)
sim.simulate()
fail(lambda: sim.simulate(), "Can not expand neighbourhood, the network is closed.", isSubstring=True)
