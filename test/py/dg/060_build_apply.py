include("xx0_helpers.py")

a = Graph.fromSMILES("[C][C][C]", name="ga")
b = Graph.fromSMILES("[O][C][C]")
c = Graph.fromSMILES("[C][O][C]")
aa = Graph.fromSMILES("[C][C][C]", name="gaa")

r = Rule.fromGMLString("""rule [
	left [
		node [ id 0 label "C" ]
	]
	right [
		node [ id 0 label "O" ]
	]
]""")

DG().build().apply([], r)
fail(lambda: DG().build().apply([None], r), "One of the graphs is a null pointer.")
fail(lambda: DG().build().apply([], None), "The rule is a null pointer.")
DG().build().apply([], Rule.fromGMLString("rule []"))
fail(lambda: DG(graphDatabase=[a]).build().apply([aa], r), "Isomorphic graphs. Candidate graph 'gaa' is isomorphic to 'ga' in the graph database.")
fail(lambda: DG(graphDatabase=[]).build().apply([a, aa], r), "Isomorphic graphs. Candidate graph 'gaa' is isomorphic to 'ga' in the graph database.")

dg = DG(graphDatabase=[a, b, c])
with dg.build() as builder:
	print("Apply")
	print("=" * 60)
	res = builder.apply([a], r)
	assert len(res) == 3
	for e in res:
		assert list(e.rules) == [r]
		assert list(v.graph for v in e.sources) == [a]
		assert e.numTargets == 1
	ts = [next(iter(e.targets)).graph for e in res]
	assert ts == [b, c, b]

	print("Assume confluecnce")
	print("=" * 60)
	config.dg.applyAssumeConfluence = True
	res = builder.apply([a], r)
	config.dg.applyAssumeConfluence = False
	assert len(res) == 1
	for e in res:
		assert list(e.rules) == [r]
		assert list(v.graph for v in e.sources) == [a]
		assert e.numTargets == 1
	ts = [next(iter(e.targets)).graph for e in res]
	assert ts == [b]

	print("Limit 0")
	print("=" * 60)
	config.dg.applyLimit = 0
	res = builder.apply([a], r)
	assert len(res) == 0

	print("Limit 1")
	print("=" * 60)
	config.dg.applyLimit = 1
	res = builder.apply([a], r)
	assert len(res) == 1
	for e in res:
		assert list(e.rules) == [r]
		assert list(v.graph for v in e.sources) == [a]
		assert e.numTargets == 1
	ts = [next(iter(e.targets)).graph for e in res]
	assert ts == [b]

	print("Limit 2")
	print("=" * 60)
	config.dg.applyLimit = 2
	res = builder.apply([a], r)
	assert len(res) == 2
	for e in res:
		assert list(e.rules) == [r]
		assert list(v.graph for v in e.sources) == [a]
		assert e.numTargets == 1
	ts = [next(iter(e.targets)).graph for e in res]
	assert ts == [b, c]

	print("Limit 3")
	print("=" * 60)
	config.dg.applyLimit = 3
	res = builder.apply([a], r)
	assert len(res) == 3
	for e in res:
		assert list(e.rules) == [r]
		assert list(v.graph for v in e.sources) == [a]
		assert e.numTargets == 1
	ts = [next(iter(e.targets)).graph for e in res]
	assert ts == [b, c, b]

	print("Limit 4")
	print("=" * 60)
	config.dg.applyLimit = 4
	res = builder.apply([a], r)
	assert len(res) == 3
	for e in res:
		assert list(e.rules) == [r]
		assert list(v.graph for v in e.sources) == [a]
		assert e.numTargets == 1
	ts = [next(iter(e.targets)).graph for e in res]
	assert ts == [b, c, b]

	config.dg.applyLimit = -1

	for verbosity in (2, 10, 20):
		print("Verbosity:", verbosity)
		print("#" * 80)
		builder.apply([a], r, verbosity=verbosity)


print("Multiple components, different")
print("=" * 60)

gO = Graph.fromSMILES('[O]', "gO")
gC = Graph.fromSMILES('[C]', "gC")
gOC = Graph.fromSMILES('[O][C]', "gOC")

r = Rule.fromGMLString("""rule [
	ruleID "Connect O C"
	context [
		node [ id 0 label "O" ]
		node [ id 1 label "C" ]
	]
	right [
		edge [ source 0 target 1 label "-" ]
	]
]""")

dg = DG(graphDatabase=[gO, gC, gOC])
with dg.build() as builder:
	res = builder.apply([gO, gC], r, verbosity=4)
	assert len(res) == 1
	for e in res:
		assert list(e.rules) == [r]
		assert set(v.graph for v in e.sources) == set([gO, gC])
		assert e.numTargets == 1
	ts = [next(iter(e.targets)).graph for e in res]
	assert ts == [gOC]

	res = builder.apply([gO, gC, gC], r, verbosity=4)
	assert len(res) == 0


print("Multiple components, same")
print("=" * 60)

gO = Graph.fromSMILES('[O]', "gO")
gOO = Graph.fromSMILES('[O][O]', "gOO")

r = Rule.fromGMLString("""rule [
	ruleID "Connect O O"
	context [
		node [ id 0 label "O" ]
		node [ id 1 label "O" ]
	]
	right [
		edge [ source 0 target 1 label "-" ]
	]
]""")

dg = DG(graphDatabase=[gO, gOO])
with dg.build() as builder:
	res = builder.apply([gO, gO], r, verbosity=4)
	assert len(res) == 1
	for e in res:
		assert list(e.rules) == [r]
		assert set(v.graph for v in e.sources) == set([gO, gO])
		assert e.numTargets == 1
	ts = [next(iter(e.targets)).graph for e in res]
	assert ts == [gOO]

	res = builder.apply([gO], r, verbosity=4)
	assert len(res) == 0


print("Split to isomorphic")
print("=" * 60)

r = Rule.fromGMLString("""rule [
	ruleID "Split to isomorphms"
	left [
		edge [ source 1 target 2 label "-" ]
	]
	context [
		node [ id 1 label "Q" ]
		node [ id 2 label "Q" ]
	]
]""")
g1 = Graph.fromDFS("[Q][Q]", "QQ")
g2 = Graph.fromDFS("[Q]", "Q")

dg = DG(graphDatabase=[g1, g2])
with dg.build() as builder:
	res = builder.apply([g1], r, verbosity=4)
	assert len(res) == 2
	for e in res:
		assert list(e.rules) == [r]
		assert set(v.graph for v in e.sources) == set([g1])
		assert e.numTargets == 2
		ts = [v.graph for v in e.targets]
		assert ts == [g2, g2]


print("Empty result")
gC = Graph.fromSMILES('[C]', "gC")
rRemove = Rule.fromGMLString('rule [ left [ node [ id 0 label "C" ] ] ]')
dg = DG()
with dg.build() as builder:
	res = builder.apply([gC], rRemove, verbosity=4)
	assert len(res) == 0
