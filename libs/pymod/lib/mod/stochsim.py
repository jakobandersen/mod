import mod

mod._deprecation(f"The {__name__} module has been removed, use the classes in the causality module instead.")

_deps = ("DrawMassAction", "ExpandByStrategy", "DrawTimeExponential")

def __getattr__(name):
	if name == "Simulator":
		msg = "Simulator has been moved, use causality.Simulator instead."
		mod._deprecation(msg)
		return mod.causality.Simulator
	if name in _deps:
		msg = f"{name} has been renamed, use causality.Simulator.{name} instead."
		mod._deprecation(msg)
		return getattr(mod.causality.Simulator, name)
	raise AttributeError(f"module '{__name__}' has no attribute '{name}'")
