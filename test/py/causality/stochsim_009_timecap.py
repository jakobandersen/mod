include("stochsim_xx1_helpers.py")

g = Graph.fromDFS("[A]")
r = Rule.fromDFS("[A]1>>[B]1")


print("Don't go over")
print("=" * 80)
sim = causality.Simulator(
	graphDatabase=inputGraphs,
	expandNetwork=causality.Simulator.ExpandByStrategy(r),
	initialState={g: 2},
	drawTime=lambda _: 9,
)
trace = sim.simulate(time=10, keepNetworkOpen=True)
for e in trace:
	print("  ", e)
assert len(trace) == 1, len(trace)
assert sim.time == 9, sim.time
assert trace.time == 9, trace.time
print("Simulate again")
trace = sim.simulate(time=10)
for e in trace:
	print("  ", e)
assert len(trace) == 2, len(trace)
assert sim.time == 18, sim.time
assert trace.time == 18, trace.time


print("Clamp to given time")
print("=" * 80)
sim = causality.Simulator(
	graphDatabase=inputGraphs,
	expandNetwork=causality.Simulator.ExpandByStrategy(r),
	initialState={g: 2},
	drawTime=lambda _: 9,
)
trace = sim.simulate(time=10, advanceToEndTime=True, keepNetworkOpen=True)
for e in trace:
	print("  ", e)
assert len(trace) == 1, len(trace)
assert sim.time == 10, sim.time
assert trace.time == 9, trace.time
print("Simulate again")
trace = sim.simulate(time=10)
for e in trace:
	print("  ", e)
assert len(trace) == 2, len(trace)
assert sim.time == 19, sim.time
assert trace.time == 19, trace.time


print("Can be exact")
print("=" * 80)
sim = causality.Simulator(
	graphDatabase=inputGraphs,
	expandNetwork=causality.Simulator.ExpandByStrategy(r),
	initialState={g: 3},
	drawTime=lambda _: 5,
)
trace = sim.simulate(time=10)
for e in trace:
	print("  ", e)
assert len(trace) == 2, len(trace)
assert sim.time == 10, sim.time
assert trace.time == 10, trace.time
