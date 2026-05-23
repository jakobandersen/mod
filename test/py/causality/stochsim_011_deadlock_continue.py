include("stochsim_xx1_helpers.py")

A = Graph.fromDFS('[A]')
B = Graph.fromDFS('[B]')
r = Rule.fromDFS("[A]>>[B]")

iRateA = 0
def iRate(v):
	if v.graph != A:
		return 0, True
	return iRateA, False

initialState = {A: 10}
sim = causality.Simulator(
	graphDatabase=inputGraphs,
	expandNetwork=causality.Simulator.ExpandByStrategy(r),
	initialState=initialState,
	draw=causality.Simulator.DrawMassAction(inputRate=iRate),
)

setCallbacks(sim)

trace = sim.simulate(keepNetworkOpen=True)
oldLen = len(trace)
trace.print()
iRateA = 10
trace = sim.simulate(iterations=200)
trace.print()
assert oldLen + 200 == len(trace)
