include("stochsim_xx1_helpers.py")

g1 = graphDFS("[g1]", "g_1")
g2 = graphDFS("[g2]", "g_2")
r1 = Rule.fromDFS("[g1]1>>[g2]1")
r2 = Rule.fromDFS("[g2]1>>[g1]1")

def rate(e):
	return {
		r1: 2,
		r2: 1
	}[next(iter(e.rules))], True

sim = causality.Simulator(
	graphDatabase=inputGraphs,
	expandNetwork=causality.Simulator.ExpandByStrategy(inputRules),
	initialState={g1: 1000},
	draw=causality.Simulator.DrawMassAction(reactionRate=rate)
)

setCallbacks(sim)

trace = sim.simulate(time=1)
trace.print()
