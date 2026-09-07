include("../xxx_helpers.py")

class _CallbackState:
	pass

_cbs = _CallbackState()
_cbs.expectedIteration = None
_cbs.deadlockHasHappened = None

def setCallbacks(sim):
	_cbs.expectedIteration = 0
	_cbs.deadlockHasHappened = False
	
	def onIterationBegin(s):
		_cbs.expectedIteration += 1
		assert s.iteration == _cbs.expectedIteration
		print(f"Iteration: {s.iteration}, t={s.time}")
	def onIterationEnd(s):
		assert s.iteration == _cbs.expectedIteration
		print(f"Iteration end: {s.iteration}, t={s.time}, event={s.trace[-1]}")
		return True
	def onDeadlock(s):
		_cbs.deadlockHasHappened = True
		assert s.iteration == _cbs.expectedIteration
		print(f"Deadlock: {s.iteration}, t={s.time}")
	def onExpand(s):
		assert s.iteration == _cbs.expectedIteration
		print("Expand:", s.iteration)
	def onExpandAvoided(s):
		assert s.iteration == _cbs.expectedIteration
		print("ExpandAvoided:", s.iteration)

	sim.setOnIterationBegin(onIterationBegin, 1)
	sim.onIterationEnd = onIterationEnd
	sim.onDeadlock = onDeadlock
	sim.onExpand = onExpand
	sim.onExpandAvoided = onExpandAvoided
