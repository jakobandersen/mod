include("stochsim_20x_polymer.py")

sim = causality.Simulator(
	labelSettings=ls,
	graphDatabase=inputGraphs,
	initialState={A: 1000, B: 1000},
	expandNetwork=expandAll,
	withSetCompare=False,
)
setCallbacks(everyIter=5000, expand=False)

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
