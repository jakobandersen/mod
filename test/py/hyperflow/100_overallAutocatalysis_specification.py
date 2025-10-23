include("xxx_common.py")
gNotInDG = Graph.fromSMILES('O')

dg = DG()
dg.build().addAbstract("""
	srcGraph -> tarGraph
	srcVertex -> tarVertex
""")
for v in dg.vertices:
	globals()[v.graph.name] = v

flow = hyperflow.Model(dg)
assert not flow.overallAutocatalysis.isEnabled

enabled(lambda: flow.overallAutocatalysis.forceExistence)
def f(): flow.overallAutocatalysis.forceExistence = True
enabled(f)

enabled(lambda: flow.overallAutocatalysis.strictTransit)
def f(): flow.overallAutocatalysis.strictTransit = True
enabled(f)

enabled(lambda: flow.overallAutocatalysis.bfsExclusive)
def f(): flow.overallAutocatalysis.bfsExclusive = True
enabled(f)

flow.allowReversal = True
flow.allowIOReversal = True

flow.overallAutocatalysis.enable()
assert flow.overallAutocatalysis.isEnabled
alreadyEnabled(lambda: flow.overallAutocatalysis.enable())

assert not flow.allowReversal
assert not flow.allowIOReversal

assert flow.overallAutocatalysis.forceExistence
flow.overallAutocatalysis.forceExistence = False
assert not flow.overallAutocatalysis.forceExistence

assert flow.overallAutocatalysis.strictTransit
flow.overallAutocatalysis.strictTransit = False
assert not flow.overallAutocatalysis.strictTransit

assert flow.overallAutocatalysis.bfsExclusive
flow.overallAutocatalysis.bfsExclusive = False
assert not flow.overallAutocatalysis.bfsExclusive


print("=" * 80)
print("Check locked specification")
print("=" * 80)
flow = hyperflow.Model(dg)
flow.findSolutions()

enabled(lambda: flow.overallAutocatalysis.forceExistence)
def f(): flow.overallAutocatalysis.forceExistence = True
enabled(f)

enabled(lambda: flow.overallAutocatalysis.strictTransit)
def f(): flow.overallAutocatalysis.strictTransit = True
enabled(f)

enabled(lambda: flow.overallAutocatalysis.bfsExclusive)
def f(): flow.overallAutocatalysis.bfsExclusive = True
enabled(f)

flow = hyperflow.Model(dg)
flow.overallAutocatalysis.enable()
flow.findSolutions()

def f(): flow.overallAutocatalysis.forceExistence = True
locked(f)

def f(): flow.overallAutocatalysis.strictTransit = True
locked(f)

def f(): flow.overallAutocatalysis.bfsExclusive = True
locked(f)


print("=" * 80)
print("Check relaxed mode")
print("=" * 80)
flow = hyperflow.Model(dg)
flow.relaxed = True
flow.overallAutocatalysis.enable()
fail(lambda: flow.findSolutions(), "OverallAutocatalysis can not be enabled in relaxed mode.")
