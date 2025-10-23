include("xxx_common.py")
gNotInDG = Graph.fromSMILES('O')
dgNot = DG()
dgNot.build().addAbstract("A -> B")
vNotInDG = next(iter(dgNot.vertices))

dgMany = DG()
dgMany.build().addAbstract("\n".join(
	"A -> {}".format(chr(i)) for i in range(ord('B'), ord('Z') + 1)
))

def lv(l):
	return "".join(e.graph.name for e in l)

def cmpVs(cand):
	assert cand == list(dgMany.vertices), "\nCand: {}\nExp:  {}".format(lv(cand), lv(dgMany.vertices))


dg = DG()
dg.build().addAbstract("""
	srcGraph -> tarGraph
	srcVertex -> tarVertex
""")
for v in dg.vertices:
	globals()[v.graph.name] = v

flow = hyperflow.Model(dg)


assert flow.sources == []
fail(lambda: flow.addSource(gNotInDG), "The graph is not a vertex in the underlying derivation graph.")
flow.addSource(srcVertex)
assert flow.sources == [srcVertex]
fail(lambda: flow.addSource(vNotInDG), "The vertex does not belong to the underlying derivation graph.")
flow.addSource(srcGraph.graph)
assert set(flow.sources) == set([srcGraph, srcVertex])

fMany = hyperflow.Model(dgMany)
for v in list(dgMany.vertices)[::-1]:
	fMany.addSource(v)
cmpVs(fMany.sources)


assert flow.sinks == []
fail(lambda: flow.addSink(gNotInDG), "The graph is not a vertex in the underlying derivation graph.")
flow.addSink(tarVertex)
assert flow.sinks == [tarVertex]
fail(lambda: flow.addSink(vNotInDG), "The vertex does not belong to the underlying derivation graph.")
flow.addSink(tarGraph.graph)
assert set(flow.sinks) == set([tarGraph, tarVertex])

fMany = hyperflow.Model(dgMany)
for v in list(dgMany.vertices)[::-1]:
	fMany.addSink(v)
cmpVs(fMany.sinks)


assert flow.excluded == []
fail(lambda: flow.exclude(gNotInDG), "Can not exclude null vertex.")
flow.exclude(tarVertex)
assert flow.excluded == [tarVertex]
fail(lambda: flow.exclude(vNotInDG), "The vertex does not belong to the underlying derivation graph.")
flow.exclude(tarGraph.graph)
assert set(flow.excluded) == set([tarGraph, tarVertex])

fMany = hyperflow.Model(dgMany)
for v in list(dgMany.vertices)[::-1]:
	fMany.exclude(v)
cmpVs(fMany.excluded)


assert flow.separatedIOInternalTransit == []
fail(lambda: flow.separateIOInternalTransit(gNotInDG), "Can not separate transit edges in null vertex.")
flow.separateIOInternalTransit(tarVertex)
assert flow.separatedIOInternalTransit == [tarVertex]
fail(lambda: flow.separateIOInternalTransit(vNotInDG), "The vertex does not belong to the underlying derivation graph.")
flow.separateIOInternalTransit(tarGraph.graph)
assert set(flow.separatedIOInternalTransit) == set([tarGraph, tarVertex])

fMany = hyperflow.Model(dgMany)
for v in list(dgMany.vertices)[::-1]:
	fMany.separateIOInternalTransit(v)
cmpVs(fMany.separatedIOInternalTransit)


assert not flow.allowHyperLoops
flow.allowHyperLoops = True
assert flow.allowHyperLoops

assert not flow.allowReversal
flow.allowReversal = True
assert flow.allowReversal

assert flow.allowIOReversal
flow.allowIOReversal = False
assert not flow.allowIOReversal

assert not flow.relaxed
flow.relaxed = True
assert flow.relaxed


flow.objectiveFunction = hyperflow.LinExp()

assert len(flow.enumerationVars) == 3
assert [str(v) for v in flow.enumerationVars] == [str(edgeFlow), str(inFlow), str(outFlow)]
flow.addEnumerationVar(inFlow)
assert [str(v) for v in flow.enumerationVars] == [str(inFlow)]


flow = hyperflow.Model(dg)
assert flow.transitEnumeration == []
flow.addTransitEnumeration(srcGraph)
assert flow.transitEnumeration == [srcGraph]
flow.addTransitEnumeration(tarGraph.graph)
assert flow.transitEnumeration == [srcGraph, tarGraph]
fail(lambda: flow.addTransitEnumeration(gNotInDG), "Can not add null vertex for transit enumeration.")
fail(lambda: flow.addTransitEnumeration(vNotInDG), "The vertex does not belong to the underlying derivation graph.")
# deprecated
checkDeprecated(lambda:
	flow.setSolverEnumerateBy(
		transitEnumeration=[srcVertex.graph, tarVertex.graph]))
old = config.common.ignoreDeprecation
config.common.ignoreDeprecation = True
flow.calc()  # to set the things from the deprecated call
config.common.ignoreDeprecation = old
assert flow.transitEnumeration == [srcGraph, tarGraph, srcVertex, tarVertex]


flow = hyperflow.Model(dg)
assert flow.absGap is None
flow.absGap = 0
assert flow.absGap == 0
flow.absGap = -1
assert flow.absGap is None
flow.absGap = 0
assert flow.absGap == 0
flow.absGap = None
assert flow.absGap is None

msg = "Specification not yet locked."
fail(lambda: flow.implementationView.printExpandedVertex(srcVertex), msg)
fail(lambda: flow.implementationView.printExpandedVertex(srcGraph), msg)

hyperflow.Model(dg).findSolutions(maxNumSolutions=0)
hyperflow.Model(dg).findSolutions(maxNumSolutions=1)
hyperflow.Model(dg).findSolutions(maxNumSolutions=0)
def f(): hyperflow.Model(dg).findSolutions(maxNumSolutions=-1)
fail(f, "Too low value for maxNumSolutions (-1). Must be at least 0.")


print("=" * 80)
print("Check locked specification")
print("=" * 80)
flow = hyperflow.Model(dg)
flow.findSolutions()
locked(lambda: flow.addSource(srcVertex))
locked(lambda: flow.addSource(srcGraph.graph))
locked(lambda: flow.addSink(tarVertex))
locked(lambda: flow.addSink(tarGraph.graph))
def f(): flow.allowHyperLoops = True
locked(f)
def f(): flow.allowReversal = True
locked(f)
def f(): flow.allowIOReversal = False
locked(f)
def f(): flow.relaxed = True
locked(f)
fail(lambda: flow.objectiveFunction, "None(mod::Py::AttributeIsNotReadable)", err=Exception)
def f(): flow.addEnumerationVar(inFlow)
locked(f)
def f(): flow.addTransitEnumeration(srcGraph)
locked(f)
def f(): flow.absGap = 0
locked(f)

s = flow.implementationView.printExpandedVertex(srcVertex)
assert type(s) == str
s = flow.implementationView.printExpandedVertex(srcGraph)
assert type(s) == str
