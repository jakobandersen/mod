include("xxx_common.py")

makeFlow("A -> B")
flow.findSolutions(maxNumSolutions=0)
view = flow.implementationView

f = view.writeMILP()
assert f.endswith(".lp")
f = view.writeMILP("lp")
assert f.endswith(".lp")
f = view.writeMILP("mps")
assert f.endswith(".mps")

fail(lambda: view.writeMILP("foo"), "Writing MILP model from", isSubstring=True)
