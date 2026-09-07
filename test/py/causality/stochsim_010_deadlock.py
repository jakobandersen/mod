include("stochsim_xx1_helpers.py")

g = Graph.fromSMILES('O')

initialState = {g: 500}
sim = causality.Simulator(
	graphDatabase=inputGraphs,
	expandNetwork=lambda b, s, u: False,
	initialState=initialState
)

setCallbacks(sim)
trace = sim.simulate()
trace.print()
assert len(trace) == 0

assert _cbs.deadlockHasHappened


initialState = {g: 500}
sim = causality.Simulator(
	graphDatabase=inputGraphs,
	expandNetwork=lambda b, s, u: False,
	initialState=initialState,
	draw=causality.Simulator.DrawMassAction(outputRate=(1, True))
)

setCallbacks(sim)
trace = sim.simulate()
trace.print()
assert len(trace) == 500

assert _cbs.deadlockHasHappened
