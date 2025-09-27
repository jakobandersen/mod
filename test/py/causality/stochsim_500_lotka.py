include("stochsim_xx1_helpers.py")

# Lotka-Volterra System
# 
#       Y -> 2*Y   @ k1=100             replication rate of rabbits
#   Y + Z -> 2*Z   @ k2=10              replication rate of foxes
#       Ø -> Y     @ inflow=5/timestep  basal growth-rate of rabbits
#       Y -> Ø     @ out-flow=0.1*Y     death rate of rabbits
#       Z -> Ø     @ out-flow=0.1*Z     death rate of foxes
#

y = Graph.fromDFS('[Y]', 'Rabbit')
z = Graph.fromDFS('[Z]', 'Fox')

r1 = Rule.fromDFS("[Y]1>>[Y]1.[Y]2")
r1.name="Y -> 2*Y"

r2 = Rule.fromDFS("[Y]1.[Z]2>>[Z]1.[Z]2")
r2.name = "Y + Z -> 2*Z"

def reactionRate(e):
	if r1 in e.rules:
		return 100, True
	if r2 in e.rules:
		return 0.1, True
	else:
		assert False

sim = causality.Simulator(
	graphDatabase=inputGraphs,
	expandNetwork=causality.Simulator.ExpandByStrategy(inputRules),
	initialState={y: 100, z: 10},
	draw=causality.Simulator.DrawMassAction(
			inputRate=lambda v: (5, True) if v.graph == y else (0, True),
			reactionRate=reactionRate,
			outputRate=(5.0, True))
)

#setCallbacks(sim)

trace = sim.simulate(iterations=100000)
print("Printing")
trace.print()

## construct derivation graph from simulation trace
dg = DG()
with dg.build() as b:
	for t in trace:
		if isinstance(t.action, causality.EdgeAction):
			b.addHyperEdge(t.action.edge)
p = DGPrinter()
p.withRuleName = True
dg.print(p)
