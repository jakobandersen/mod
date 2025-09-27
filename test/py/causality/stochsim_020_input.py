include("stochsim_xx1_helpers.py")

g1 = smiles('O')
g2 = smiles('C')
g3 = smiles('N')

def iRate(v):
	return {
		g1: 0.4,
		g2: 0.2,
		g3: 0.1
	}[v.graph], True

sim = causality.Simulator(
	graphDatabase=inputGraphs,
	expandNetwork=lambda b, s, u: False,
	initialState={g1: 0, g2: 0, g3: 0},
	draw=causality.Simulator.DrawMassAction(inputRate=iRate)
)

setCallbacks(sim)

trace = sim.simulate(time=1000)
trace.print()
