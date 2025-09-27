include("stochsim_xx1_helpers.py")

# SIR Model
# 
# S + I -> 2 I @ k1=0.01
# I -> R       @ k2=0.001 
#

suseptible = graphDFS('[S]', 'suseptible')
infected = graphDFS('[I]', 'infected')
recovered = graphDFS('[R]', 'recovered')

r1 = Rule.fromDFS("[S]1.[I]2>>[I]1.[I]2")
r1.name = "S + I -> 2 I"

r2 = Rule.fromDFS("[I]1>>[R]1")
r2.name = "I -> R"

def reactionRate(e):
	if r1 in e.rules:
		return 0.001, True
	elif r2 in e.rules:
		return 0.1, True
	else:
		assert False

sim = causality.Simulator(
	graphDatabase=inputGraphs,
	expandNetwork=causality.Simulator.ExpandByStrategy(inputRules),
	initialState={suseptible: 200, infected: 2},
	draw=causality.Simulator.DrawMassAction(reactionRate=reactionRate)
)

trace = sim.simulate()
trace.print()
