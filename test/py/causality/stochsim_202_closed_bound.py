include("stochsim_20x_polymer.py")

sim = causality.Simulator(
	labelSettings=ls,
	graphDatabase=inputGraphs,
	initialState={A: 1000, B: 1000},
	expandNetwork=causality.Simulator.ExpandByStrategy(expandIter)
)
setCallbacks(everyIter=5000, expand=True)
trace = sim.simulate(iterations=50000)
