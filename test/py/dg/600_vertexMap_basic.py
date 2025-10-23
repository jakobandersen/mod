include("../xxx_vertexMap.py")

fail(lambda: DGVertexMapper(DG.HyperEdge()),
	"Can not find vertex maps for null edge.")

O = Graph.fromSMILES("[O]", name="O")
S = Graph.fromSMILES("[S]", name="S")
r = Rule.fromDFS("[O]1>>[S]1")
rCopy = Rule.fromDFS("[O]1>>[S]1")

dg = DG(graphDatabase=inputGraphs)
dg.build().apply([O], r)
assert dg.numEdges == 1
e = next(iter(dg.edges))
maps = DGVertexMapper(e)
assert maps.edge == e
assert list(maps.left) == [O]
assert list(maps.right) == [S]
assert len(maps) == maps.size
assert len(list(maps)) == maps.size
assert maps.size == 1

rule = maps[0].rule
m = maps[0].map
assert rule == r
checkVertexMap(UnionGraph([O]), UnionGraph([S]), m)

vDom = next(iter(m.domain.vertices))
vCodom = next(iter(m.codomain.vertices))
assert m[vDom] == vCodom
assert m.inverse(vCodom) == vDom
