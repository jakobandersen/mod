include("stochsim_xx1_helpers.py")

g1 = Graph.fromSMILES('O')
g2 = Graph.fromSMILES('C')
g3 = Graph.fromSMILES('N')


print("=" * 80)

def iRate(v):
	assert v.graph == g1
	return 1, True

sim = causality.Simulator(
	graphDatabase=inputGraphs,
	expandNetwork=lambda b, s, u: False,
	initialState={},
	draw=causality.Simulator.DrawMassAction(inputRate=iRate)
)
trace = sim.simulate(iterations=1)
assert len(trace) == 0, trace


print("=" * 80)

def iRate(v):
	assert v.graph == g1
	return 1, True

sim = causality.Simulator(
	graphDatabase=inputGraphs,
	expandNetwork=lambda b, s, u: False,
	initialState={g1: 0},
	draw=causality.Simulator.DrawMassAction(inputRate=iRate)
)
trace = sim.simulate(iterations=1)
assert len(trace) == 1, trace


print("=" * 80)

def iRate(v):
	return {
		g1: 0.4,
		g2: 0.2,
		g3: 0.1
	}[v.graph], True

sim = causality.Simulator(
	graphDatabase=inputGraphs,
	expandNetwork=lambda b, s, u: False,
	initialState={g1: 0, g2: 0, g3: 0},
	draw=causality.Simulator.DrawMassAction(inputRate=iRate)
)

trace = sim.simulate(iterations=1000)
assert len(trace) == 1000, len(trace)
trace.print()
