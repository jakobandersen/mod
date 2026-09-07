include("../formoseCommon/grammar.py")

def expand(b, s, u):
	b.execute(addSubset(formaldehyde, glycolaldehyde) >> repeat(rightPredicate[lambda d: all(g.vLabelCount("C") <= 6 for g in d.right)](
			inputRules
	)))
	return False


def reactionRate(e):
	if ketoEnol_F in e.rules or ketoEnol_B in e.rules:
		return 10, True
	elif aldolAdd_F in e.rules or aldolAdd_B in e.rules:
		return 1, True
	else:
		assert False

sim = causality.Simulator(
	graphDatabase=inputGraphs,
	expandNetwork=expand,
	initialState={formaldehyde: 1000, glycolaldehyde: 100},
	draw=causality.Simulator.DrawMassAction(reactionRate=reactionRate)
)

def onIterationBegin(s):
	print(f"Iteration: {s.iteration:7}   time = {s.time:7.4f}")

sim.setOnIterationBegin(onIterationBegin, 10000)

trace = sim.simulate(iterations=200000)
