include("stochsim_xx1_helpers.py")

g = Graph.fromDFS("[A]")
r = Rule.fromDFS("[A]1>>[B]1")


def createSim(drawTime):
	return causality.Simulator(
		graphDatabase=inputGraphs,
		expandNetwork=causality.Simulator.ExpandByStrategy(r),
		initialState={g: 10},
		drawTime=drawTime,
	)

createSim(None).simulate()
