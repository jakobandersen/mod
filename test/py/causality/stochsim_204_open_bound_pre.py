include("stochsim_20x_polymer.py")

sim = causality.Simulator(
	labelSettings=ls,
	graphDatabase=inputGraphs,
	initialState={A: 0, B: 0},
	expandNetwork=expandAll,
	withSetCompare=False,
	draw=causality.Simulator.DrawMassAction(
		inputRate=lambda v: (1, True) if v in (A, B) else (0, True),
		outputRate=lambda v: (0.01, True),
		reactionRate=(1, True)
	)
)
setCallbacks(everyIter=5000, expand=False)
trace = sim.simulate(iterations=50000)
