include("../xxx_helpers.py")

def setCallbacks(sim):
	def onIterationBegin(s):
		print("Iteration: {}, t={}".format(s.iteration, s.time))
	def onIterationEnd(s, action, timeInc):
		print("New state: {}, t={}, delta t={}".format(s.iteration, s.time, timeInc))
		print("  action={}".format(action))
		return True
	def onDeadlock(s):
		print("Deadlock: {}, t={}".format(s.iteration, s.time))
	def onExpand(s):
		print("Expand:", s.iteration)
	def onExpandAvoided(s):
		print("ExpandAvoided:", s.iteration)

	sim.onIterationBegin = onIterationBegin
	sim.onIterationEnd = onIterationEnd
	sim.onDeadlock = onDeadlock
	sim.onExpand = onExpand
	sim.onExpandAvoided = onExpandAvoided
