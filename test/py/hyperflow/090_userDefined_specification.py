include("xxx_common.py")

makeDG("A -> B")

flow = hyperflow.Model(dg)
bVar = flow.addBoolVariable("boolVarName")
iVar = flow.addIntVariable("intVarName")
fVar = flow.addFloatVariable("floatVarName")
fail(lambda: flow.addIntVariable("boolVarName"),
	"Name for custom variable 'boolVarName' already in use.")
fail(lambda: flow.addIntVariable("intVarName"),
	"Name for custom variable 'intVarName' already in use.")
fail(lambda: flow.addIntVariable("floatVarName"),
	"Name for custom variable 'floatVarName' already in use.")

flow = hyperflow.Model(dg)
bVar = flow.addIntVariable("floatVarName")
iVar = flow.addIntVariable("intVarName")
fVar = flow.addIntVariable("boolVarName")
flow.addConstraint(bVar == 0)
flow.addConstraint(iVar == 0)
flow.addConstraint(fVar == 0)
flow.findSolutions()


print("=" * 80)
print("Check locked specification")
print("=" * 80)
flow = hyperflow.Model(dg)
flow.findSolutions()
locked(lambda: flow.addBoolVariable("boolVarName"))
locked(lambda: flow.addIntVariable("intVarName"))
locked(lambda: flow.addFloatVariable("floatVarName"))


print("=" * 80)
print("Check relaxed mode")
print("=" * 80)
flow = hyperflow.Model(dg)
flow.relaxed = True
bVar = flow.addIntVariable("boolVarName")
iVar = flow.addIntVariable("intVarName")
fVar = flow.addIntVariable("floatVarName")
flow.findSolutions()

flow = hyperflow.Model(dg)
flow.relaxed = True
bVar = flow.addIntVariable("boolVarName")
iVar = flow.addIntVariable("intVarName")
fVar = flow.addIntVariable("floatVarName")
flow.addConstraint(bVar == 0)
flow.addConstraint(iVar == 0)
flow.addConstraint(fVar == 0)
flow.findSolutions()


print("=" * 80)
print("Check variable order")
print("=" * 80)
flow = hyperflow.Model(dg)
vs = "zyxwvutsrqponmlkjihgfedcba"
vsExp = []
for v in vs:
	flow.addBoolVariable(v + 'b')
	flow.addIntVariable(v + 'i')
	flow.addFloatVariable(v + 'f')
	vsExp.extend([v + 'i', v + 'f', v + 'b'])
vsExp.reverse()
flow.listSpecification()

assert len(flow.customBoolVariables) == len(vs)
assert len(flow.customIntVariables) == len(vs)
assert len(flow.customFloatVariables) == len(vs)
vsCand = []
for b, i, f in zip(flow.customBoolVariables, flow.customIntVariables, flow.customFloatVariables):
	vsCand.extend([b.name, f.name, i.name])

assert vsExp == vsCand, "\nExp: {}\nAct: {}".format(vsExp, vsCand)
