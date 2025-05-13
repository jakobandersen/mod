include("xxx_common.py")
gNotInDG = smiles('O')

dg = DG()
dg.build().addAbstract("""
	srcGraph -> tarGraph
	srcVertex -> tarVertex
""")
for v in dg.vertices:
	globals()[v.graph.name] = v

flow = hyperflow.Model(dg)
assert not flow.overallCatalysis.isEnabled

enabled(lambda: flow.overallCatalysis.forceExistence)
def f(): flow.overallCatalysis.forceExistence = True
enabled(f)

enabled(lambda: flow.overallCatalysis.strictTransit)
def f(): flow.overallCatalysis.strictTransit = True
enabled(f)

flow.allowReversal = True
flow.allowIOReversal = True

flow.overallCatalysis.enable()
assert flow.overallCatalysis.isEnabled
alreadyEnabled(lambda: flow.overallCatalysis.enable())

assert not flow.allowReversal
assert not flow.allowIOReversal

assert flow.overallCatalysis.forceExistence
flow.overallCatalysis.forceExistence = False
assert not flow.overallCatalysis.forceExistence

assert flow.overallCatalysis.strictTransit
flow.overallCatalysis.strictTransit = False
assert not flow.overallCatalysis.strictTransit


print("=" * 80)
print("Check locked specification")
print("=" * 80)
flow = hyperflow.Model(dg)
flow.findSolutions()

enabled(lambda: flow.overallCatalysis.forceExistence)
def f(): flow.overallCatalysis.forceExistence = True
enabled(f)

enabled(lambda: flow.overallCatalysis.strictTransit)
def f(): flow.overallCatalysis.strictTransit = True
enabled(f)

flow = hyperflow.Model(dg)
flow.overallCatalysis.enable()
flow.findSolutions()

def f(): flow.overallCatalysis.forceExistence = True
locked(f)

def f(): flow.overallCatalysis.strictTransit = True
locked(f)


print("=" * 80)
print("Check relaxed mode")
print("=" * 80)
flow = hyperflow.Model(dg)
flow.relaxed = True
flow.overallCatalysis.enable()
fail(lambda: flow.findSolutions(), "OverallCatalysis can not be enabled in relaxed mode.")
