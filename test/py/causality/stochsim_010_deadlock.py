include("stochsim_xx1_helpers.py")

g = Graph.fromSMILES('O')

initialState = {g: 500}
sim = causality.Simulator(
	graphDatabase=inputGraphs,
	expandNetwork=lambda b, s, u: False,
	initialState=initialState
)

setCallbacks(sim)

trace = sim.simulate(time=200)
trace.print()
