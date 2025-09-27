include("../xxx_helpers.py")

def vertex(id_, r):
	return r.getVertexFromExternalId(id_)

_ = Rule.fromDFS("[A]0>>[A]0")
_.name = "->"

A_B = Rule.fromDFS("[P]2[A]0[X]1>>[P]2[B]0{=}[Y]1")
A_B.name = "A -> B"

B_C = Rule.fromDFS("[Q]2[B]0{=}[Y]1>>[Q]2[C]0{#}[Z]1")
B_C.name = "B -> C"

fail(lambda: RCMatch(None, _), "rFirst is null.")
fail(lambda: RCMatch(_, None), "rSecond is null.")

m = RCMatch(A_B, B_C)
v1 = vertex(0, A_B).right
v2 = vertex(0, B_C).left
v1_1 = vertex(1, A_B).right
v2_1 = vertex(1, B_C).left
v1_2 = vertex(2, A_B).right
v2_2 = vertex(2, B_C).left

assert str(m) == "RCMatch({}, {}, {})".format(m.first, m.second, m.labelSettings)

assert m.first == A_B
assert m.second == B_C
assert m.labelSettings == LabelSettings(LabelType.String, LabelRelation.Specialisation)

fail(lambda: m[Rule.RightGraph.Vertex()],
	"Can not get mapped vertex for a null vertex in the first rule.")
fail(lambda: m[Rule.LeftGraph.Vertex()],
	"Can not get mapped vertex for a null vertex in the second rule.")
fail(lambda: m[vertex(0, _).right],
	"The vertex does not belong to the first rule.")
fail(lambda: m[vertex(0, _).left],
	"The vertex does not belong to the second rule.")
assert m[v1] == Rule.LeftGraph.Vertex(), m[v1]
assert m[v2] == Rule.RightGraph.Vertex(), m[v2]

assert m.size == 0

fail(lambda: m.push(Rule.RightGraph.Vertex(), v2),
	"Can not add null vertex as the first component to the match.")
fail(lambda: m.push(v1, Rule.LeftGraph.Vertex()),
	"Can not add null vertex as the second component to the match.")
fail(lambda: m.push(vertex(0, _).right, v2),
	"The first vertex does not belong to the first rule.")
fail(lambda: m.push(v1, vertex(0, _).left),
	"The second vertex does not belong to the second rule.")

fail(lambda: m.pop(), "Can not pop from empty match.")

m.push(v1, v2)
assert m.size == 1
assert m[v1] == v2
assert v1 == m[v2]

fail(lambda: m.push(v1, v2_1), "First vertex already mapped, to {}".format(v2))
fail(lambda: m.push(v1_1, v2), "Second vertex already mapped, to {}".format(v1))

m.push(v1_1, v2_1)
assert m.size == 2

fail(lambda: m.push(v1_2, v2_2),
	"Match extension infeasible ({} <-> {}).".format(v1_2, v2_2))

r2 = m.compose()
assert r2 is not None
m.compose(verbose=True)
r2m = m.composeWithMaps()
assert r2m is not None
assert r2.isomorphism(r2m.rule)
m.composeWithMaps(verbose=True)

res = m.composeAll()
assert len(res) == 1
assert res[0].isomorphism(r2) == 1
resm = m.composeAllWithMaps()
assert len(resm) == 1
assert resm[0].rule.isomorphism(r2) == 1

res = m.composeAll(maximum=True)
assert len(res) == 1
assert res[0].isomorphism(r2) == 1
resm = m.composeAllWithMaps(maximum=True)
assert len(resm) == 1
assert resm[0].rule.isomorphism(r2) == 1

m.pop()
r1 = m.compose()
assert r1 is not None

res = m.composeAll()
assert len(res) == 2
assert res[0].isomorphism(r1) == 1
assert res[1].isomorphism(r2) == 1
res = m.composeAll(maximum=True)
assert len(res) == 1
assert res[0].isomorphism(r2) == 1

m.pop()
r0 = m.compose()
assert r0 is not None

m.push(v1_1, v2_1)
r1alt = m.compose()
assert r1alt is not None

res = m.composeAll()
assert len(res) == 2
assert res[0].isomorphism(r1alt) == 1
assert res[1].isomorphism(r2) == 1
res = m.composeAll(maximum=True)
assert len(res) == 1
assert res[0].isomorphism(r2) == 1

m.pop()

res = m.composeAll()
assert len(res) == 4, res
assert res[0].isomorphism(r0) == 1
assert res[1].isomorphism(r1) == 1
assert res[2].isomorphism(r2) == 1
assert res[3].isomorphism(r1alt) == 1
res = m.composeAll(maximum=True)
assert len(res) == 1
assert res[0].isomorphism(r2) == 1
