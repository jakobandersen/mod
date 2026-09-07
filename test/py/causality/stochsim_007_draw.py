include("stochsim_xx1_helpers.py")

g = Graph.fromDFS("[A]")
r = Rule.fromDFS("[A]1>>[B]1")


def createSim(draw):
	return causality.Simulator(
		graphDatabase=inputGraphs,
		expandNetwork=causality.Simulator.ExpandByStrategy(r),
		initialState={g: 10},
		drawTime=lambda _: 1,
		draw=draw,
	)


fail(lambda: createSim(lambda _: None), pattern="Creation of Simulator failed. Draw function initialization resulted in null function.")

class Draw(causality.Simulator.DrawFunction):
	pass

createSim(Draw())
fail(lambda: createSim(Draw()).simulate(time=0), err=MethodOverrideError,
	pattern="Error with 'syncSize' in class derived from 'causality.Simulator.DrawFunction': method not found.")


class DrawWithSync(causality.Simulator.DrawFunction):
	def syncSize(self):
		return 42  # return type/value ignored

createSim(DrawWithSync())
fail(lambda: createSim(lambda marking: DrawWithSync()).simulate(time=0), err=MethodOverrideError,
	pattern="Error with 'draw' in class derived from 'causality.Simulator.DrawFunction': method not found.")


class DrawWithSyncDraw(causality.Simulator.DrawFunction):
	def __init__(self, marking):
		super().__init__()
		self.marking = marking
	def syncSize(self):
		return 42  # return type/value ignored
	def draw(self):
		return causality.InputAction(self.marking.net.dg.findVertex(g)), 42

createSim(lambda m: DrawWithSyncDraw(m))
fail(lambda: createSim(lambda m: DrawWithSyncDraw(m)).simulate(iterations=1), err=MethodOverrideError,
	pattern="Error with 'stateUpdated' in class derived from 'causality.Simulator.DrawFunction': method not found.")


def testBoth(makeItem):
	class Draw(causality.Simulator.DrawFunction):
		def __init__(self, marking):
			super().__init__()
			self.marking = marking
		def syncSize(self): pass
		def draw(self):
			return makeItem(self.marking.net.dg), 42
		def stateUpdated(self):
			pass

	createSim(lambda m: Draw(m))
	trace = createSim(lambda m: Draw(m)).simulate(time=0)
	assert len(trace) == 0
	trace = createSim(lambda m: Draw(m)).simulate(time=10)
	assert len(trace) == 10
	for i in range(10):
		e = trace[i]
		assert e.time == i + 1
		assert e.action == makeItem(trace.initialState.net.dg)

testBoth(lambda dg: causality.EdgeAction(next(iter(dg.edges))))
testBoth(lambda dg: causality.InputAction(dg.findVertex(g)))
testBoth(lambda dg: causality.OutputAction(dg.findVertex(g)))


def testReturnType(value, errType, pattern):
	class Draw(causality.Simulator.DrawFunction):
		def syncSize(self): pass
		def draw(self): return value

	createSim(Draw())
	fail(lambda: createSim(lambda _: Draw()).simulate(time=0), err=errType, pattern=pattern)

testReturnType(42, TypeError, "Expecting an object of type tuple; got an object of type int instead")
testReturnType((42, 42), TypeError, "")
