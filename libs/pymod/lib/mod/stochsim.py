import mod

mod._deprecation("The {} module has been removed, use the classes in the causality module instead.".format(__name__))

_deps = ("DrawMassAction", "ExpandByStrategy", "DrawTimeExponential")

def __getattr__(name):
	if name == "Simulator":
		msg = "Simulator has been moved, use causality.Simulator instead."
		mod._deprecation(msg)
		return mod.causality.Simulator
	if name in _deps:
		msg = "{} has been renamed, use causality.Simulator.{} instead.".format(name, name)
		mod._deprecation(msg)
		return getattr(mod.causality.Simulator, name)
	raise AttributeError("module '{}' has no attribute '{}'".format(__name__, name))
