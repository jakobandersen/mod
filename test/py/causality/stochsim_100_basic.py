include("stochsim_xx1_helpers.py")

# Do basic API tests, basically just checking the names and shape of functions.

g0 = Graph.fromDFS('[O]')
g1 = Graph.fromDFS('[S]')
r = Rule.fromDFS("[O]1>>[S]1")


def doTest(expander):
	print("=" * 80)
	print("=" * 80)
	initialState = {g0: 500, g1: 0}
	sim = causality.Simulator(
		graphDatabase=inputGraphs,
		expandNetwork=expander,
		initialState=initialState,
		draw=causality.Simulator.DrawMassAction(
			inputRate=lambda v: (2, True) if v.graph == g0 else (0, True),
			reactionRate=(0.002, True),
			outputRate=(0.01, True)
		)
	)

	setCallbacks(sim)

	trace = sim.simulate(time=200)
	print("DG:", sim.dg)
	print("Iteration:", sim.iteration)
	print("Time:", sim.time)
	trace.print()

doTest(causality.Simulator.ExpandByStrategy(inputRules))
doTest(lambda b, s, u: False)
