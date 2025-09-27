include("../xxx_vertexMap.py")

def vertex(id_, r):
	return r.getVertexFromExternalId(id_)

A_B = Rule.fromDFS("[K]5[A]0[X]2>>[K5][B]0[Q]1", "A -> BQ")
B_C = Rule.fromDFS("[M]5[B]0[Q]1>>[M]5[C]0[Y]2", "BQ -> C")
A_B.print()
B_C.print()

m = RCMatch(A_B, B_C)
v1B = vertex(0, A_B)
v2B = vertex(0, B_C)
v1Q = vertex(1, A_B)
v2Q = vertex(1, B_C)
v1X = vertex(2, A_B)
v2Y = vertex(2, B_C)
v1K = vertex(5, A_B)
v2M = vertex(5, B_C)

m.push(v1B.right, v2B.left)
m.push(v1Q.right, v2Q.left)
res = m.composeWithMaps()
assert res
res.rule.print()
checkVertexMap(A_B, B_C, res.mFirstToSecond)
checkVertexMap(A_B, res.rule, res.mFirstToRes)
checkVertexMap(B_C, res.rule, res.mSecondToRes)

assert res.mFirstToSecond[v1B] == v2B
assert res.mFirstToSecond[v1Q] == v2Q
assert not res.mFirstToSecond[v1X]
assert not res.mFirstToSecond[v1K]
assert not res.mFirstToSecond.inverse(v2Y)
assert not res.mFirstToSecond.inverse(v2M)

assert res.mFirstToRes[v1B]
assert res.mSecondToRes[v2B]

assert res.mFirstToRes[v1B] == res.mSecondToRes[v2B]
assert not res.mFirstToRes[v1Q]
assert not res.mSecondToRes[v2Q]
