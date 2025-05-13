include("xxx_common.py")

fail(lambda: hyperflow.Model(None), "Can not create model from null pointer.")
fail(lambda: hyperflow.Model(None, "blah"), "Can not create model from null pointer.")

dg = DG()
fail(lambda: hyperflow.Model(dg), "Can not create model with unlocked DG.")
b = dg.build()
b.addAbstract("A -> B")
fail(lambda: hyperflow.Model(dg), "Can not create model with unlocked DG.")
del b
_ = hyperflow.Model(dg)
checkDeprecated(lambda: Flow(dg))
checkDeprecated(lambda: Flow(dg, ilpSolver="default"))
checkDeprecated(lambda: dgFlow(dg))


dg = DG()
dg.build()
fail(lambda: hyperflow.Model(dg), "Can not create model with an empty DG.")


dg = DG()
dg.build().addAbstract("A -> B")
for v in dg.vertices:
	globals()[v.graph.name] = v

flow = hyperflow.Model(dg)
assert flow.dg == dg
assert flow.id == 4
assert str(flow) == "HyperflowModel(%d)" % flow.id
assert str(flow) == repr(flow)

assert not flow.specificationLocked
flow.listSpecification()
flow.findSolutions()
assert flow.specificationLocked
flow.findSolutions()
assert flow.specificationLocked

flow = hyperflow.Model(dg)

f = flow.dump()
assert f.startswith("out/")
assert f.endswith("_Flow.flow")

f = flow.dump('')
assert f.startswith("out/")
assert f.endswith("_Flow.flow")

f = flow.dump('out/myFilename.flow')
assert f == 'out/myFilename.flow'
fail(lambda: flow.dump("/dev/null/Flow.flow"),
	"Can not open file '/dev/null/Flow.flow'.")

fail(lambda: hyperflow.Model.load(None, "blah"), "Can not load model with null pointer DG.")
fail(lambda: hyperflow.Model.loadString(None, "blah"), "Can not load model with null pointer DG.")
dg = DG()
fail(lambda: hyperflow.Model.load(dg, "blah"), "Can not load model with unlocked DG.")
fail(lambda: hyperflow.Model.loadString(dg, "blah"), "Can not load model with unlocked DG.")

dg.build().addAbstract("A -> B")
for v in dg.vertices:
	globals()[v.graph.name] = v
f1 = hyperflow.Model(dg)
assert f1.sources == []
f1.addSource(A)
assert f1.sources == [A]
f2 = hyperflow.Model(f1)
assert f2.sources == [A]
assert not f2.specificationLocked
f2.addSource(B)
assert f2.sources == [A, B]
assert f1.sources == [A]

f1.findSolutions()
f3 = hyperflow.Model(f1)
assert f3.sources == [A]
assert not f2.specificationLocked
