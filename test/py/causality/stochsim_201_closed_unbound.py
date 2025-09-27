include("stochsim_20x_polymer.py")

sim = causality.Simulator(
	labelSettings=ls,
	graphDatabase=inputGraphs,
	expandNetwork=causality.Simulator.ExpandByStrategy(inputRules),
	initialState={A: 1000, B: 1000},
	draw=causality.Simulator.DrawMassAction(reactionRate=(1, True))
)
setCallbacks(everyIter=200, expand=True)
trace = sim.simulate(iterations=1200)
