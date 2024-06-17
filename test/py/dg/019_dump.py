include("xx0_helpers.py")
disableBuildHook()

g = smiles("O")
g1 = smiles('O')
g2 = smiles('O')
r = ruleGMLString("""rule [
	left [ node [ id 0 label "O" ] ]
	right [ node [ id 0 label "S" ] ]
]""")

# DG.dump

fail(lambda: DG().dump(), "Can not dump DG before it is locked.")

dg = DG()
dg.build()

f = dg.dump()
assert f.startswith("out/")
assert f.endswith("_DG.dg")

f = dg.dump('')
assert f.startswith("out/")
assert f.endswith("_DG.dg")

f = dg.dump("out/myFilename.dg")
assert f == "out/myFilename.dg"
fail(lambda: dg.dump("/dev/null/DG.dg"),
	"Can not open file '/dev/null/DG.dg'.")


# DG.load

dg = DG()
dg.build()
f = dg.dump()

fail(lambda: DG.load([None], [], f), "Null pointer in graph database.")
fail(lambda: DG.load([g1, g2], [], CWDPath(f)), "Isomorphic graphs '{}' and '{}' in initial graph database.".format(g1.name, g2.name))
dg = DG.load([g1, g2], [], CWDPath(f), graphPolicy=IsomorphismPolicy.TrustMe)

fail(lambda: DG.load([], [None], CWDPath(f)), "Null pointer in rule database.")
fail(lambda: DG.load([], [], 'doesNotExist.dg'),
	"DG load error: Could not open file",
	err=InputError, isSubstring=True)


dg = DG()
with dg.build() as b:
	b.execute(addSubset(g))
v = next(iter(dg.vertices))
f = dg.dump()

dg2 = DG.load([], [], CWDPath(f))
_compareDGs(dg, dg2, compareData=False)
v2 = next(iter(dg2.vertices))
assert v.graph != v2.graph
assert v.graph.name == v2.graph.name
assert v.graph.isomorphism(v2.graph)

dg3 = DG.load(dg.graphDatabase, [], CWDPath(f))
_compareDGs(dg, dg3)


dg = DG()
with dg.build() as b:
	b.execute(addSubset(g) >> r)
vO = next(v for v in dg.vertices if v.graph == g)
vS = next(v for v in dg.vertices if v.graph != g)
e = next(iter(dg.edges))
f = dg.dump()

dg2 = DG.load(dg.graphDatabase, [], CWDPath(f))
_compareDGs(dg, dg2, compareData=False)
e2 = next(iter(dg2.edges))
assert len(e2.rules) == 1
r2 = next(iter(e2.rules))
assert r != r2
assert r.name == r2.name
assert r.isomorphism(r2)

dg3 = DG.load(dg.graphDatabase, inputRules, CWDPath(f))
_compareDGs(dg, dg3)


# DG.Builder.load

dg = DG()
dg.build()
f = dg.dump()

fail(lambda: DG().build().load([None], CWDPath(f)), "Null pointer in rule database.")
fail(lambda: DG().build().load([], 'doesNotExist.dg'),
	"DG load error: Could not open file ",
	err=InputError, isSubstring=True)

dg2 = DG(labelSettings=LabelSettings(LabelType.Term, LabelRelation.Specialisation))
fail(lambda: dg2.build().load([], CWDPath(f)),
	"DG load error: Mismatch of label settings. This DG has {} but the dump to be loaded has {}.".format(dg2.labelSettings, dg.labelSettings), err=InputError)


dg = DG()
with dg.build() as b:
	b.execute(addSubset(g))
v = next(iter(dg.vertices))
f = dg.dump()

dg2 = DG()
dg2.build().load([], CWDPath(f))
_compareDGs(dg, dg2, compareData=False)
v2 = next(iter(dg2.vertices))
assert v.graph != v2.graph
assert v.graph.name == v2.graph.name
assert v.graph.isomorphism(v2.graph)

dg3 = DG(graphDatabase=dg.graphDatabase)
dg3.build().load([], CWDPath(f))
_compareDGs(dg, dg3)


dg = DG()
with dg.build() as b:
	b.execute(addSubset(g) >> r)
vO = next(v for v in dg.vertices if v.graph == g)
vS = next(v for v in dg.vertices if v.graph != g)
e = next(iter(dg.edges))
f = dg.dump()

dg2 = DG(graphDatabase=dg.graphDatabase)
dg2.build().load([], CWDPath(f))
_compareDGs(dg, dg2, compareData=False)
e2 = next(iter(dg2.edges))
assert len(e2.rules) == 1
r2 = next(iter(e2.rules))
assert r != r2
assert r.name == r2.name
assert r.isomorphism(r2)

dg3 = DG(graphDatabase=dg.graphDatabase)
dg3.build().load(inputRules, CWDPath(f))
_compareDGs(dg, dg3)
