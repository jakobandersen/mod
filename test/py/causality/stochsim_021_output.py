include("stochsim_xx1_helpers.py")

g1 = Graph.fromSMILES('O')
g2 = Graph.fromSMILES('C')
g3 = Graph.fromSMILES('N')

def oRate(v):
	return {
		g1: 0.4,
		g2: 0.2,
		g3: 0.1
	}[v.graph], True

sim = causality.Simulator(
	graphDatabase=inputGraphs,
	expandNetwork=lambda b, s, u: False,
	initialState={g1: 1024, g2: 512, g3: 256},
	draw=causality.Simulator.DrawMassAction(outputRate=oRate)
)

setCallbacks(sim)

trace = sim.simulate()
trace.print()
