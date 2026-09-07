include("stochsim_xx1_helpers.py")

A = Graph.fromDFS('[A]')
B = Graph.fromDFS('[B]')
r = Rule.fromDFS("[A]>>[B]")

initialState = {A: 10}
sim = causality.Simulator(
	graphDatabase=inputGraphs,
	expandNetwork=causality.Simulator.ExpandByStrategy(r),
	initialState=initialState,
)

setCallbacks(sim)

trace = sim.simulate()
trace.print()
assert len(trace) == 10
