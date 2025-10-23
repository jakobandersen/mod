include("stochsim_xx1_helpers.py")

g1 = Graph.fromDFS("[g1]", "g_1")
g2 = Graph.fromDFS("[g2]", "g_2")
g3 = Graph.fromDFS("[g3]", "g_3")
r1 = Rule.fromDFS("[g1]1>>[g2]1")
r2 = Rule.fromDFS("[g1]1>>[g3]1")

def rate(e):
	return {
		r1: 1,
		r2: 2
	}[next(iter(e.rules))], True

sim = causality.Simulator(
	graphDatabase=inputGraphs,
	expandNetwork=causality.Simulator.ExpandByStrategy(inputRules),
	initialState={g1: 1000},
	draw=causality.Simulator.DrawMassAction(reactionRate=rate)
)

setCallbacks(sim)

trace = sim.simulate()
trace.print()
