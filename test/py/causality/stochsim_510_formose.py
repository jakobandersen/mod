include("../formoseCommon/grammar.py")

def reactionRate(e):
	if ketoEnol_F in e.rules or ketoEnol_B in e.rules:
		return 10, True
	elif aldolAdd_F in e.rules or aldolAdd_B in e.rules:
		return 1, True
	else:
		assert False

sim = causality.Simulator(
	graphDatabase=inputGraphs,
	expandNetwork=causality.Simulator.ExpandByStrategy(
		rightPredicate[lambda d: all(g.vLabelCount("C") <= 6 for g in d.right)](
			inputRules
		)
	),
	initialState={formaldehyde: 1000, glycolaldehyde: 100},
	draw=causality.Simulator.DrawMassAction(reactionRate=reactionRate)
)

expands = 0
expandsAvoided = 0
def onIterationBegin(s):
	global expands
	global expandsAvoided
	print(f"Iteration: {s.iteration}   time = {s.time}   |V| = {s.dg.numVertices}")
	if expands > 0:
		print("\tExpands:", expands)
		expands = 0
	if expandsAvoided > 0:
		print("\tExpandsAvoided:", expandsAvoided)
		expandsAvoided = 0

def onExpand(s):
	global expands
	expands += 1
	#print("\tExpanding")

def onExpandAvoided(s):
	global expandsAvoided
	expandsAvoided += 1


sim.setOnIterationBegin(onIterationBegin, 10000)
sim.onExpand = onExpand
sim.onExpandAvoided = onExpandAvoided


def run():
	return sim.simulate(iterations=200000)

if True:
	import cProfile
	fName = "out/cProfile.profile"
	profile = cProfile.Profile()
	profile.run("trace = run()")
	profile.dump_stats(fName)
	profile.print_stats()
else:
	trace = run()
