include("../xxx_helpers.py")

g = Graph.fromSMILES("O", "Water")
dg = DG()
dg.build().addAbstract("A -> B")
v = next(iter(dg.vertices))
e = next(iter(dg.edges))


vsv = hyperflow.VarSumVertex("vsv")
assert str(vsv) == "vsv"
assert repr(vsv) == "VarSumVertex(%s)" % vsv
assert vsv.id == str(vsv)

vv = hyperflow.VarVertex("vv", v)
assert str(vv) == "vv[%s]" % v
assert repr(vv) == "VarVertex(%s, %s)" % (vv.id, v)
assert vv.id == "vv"
assert vv.vertex == v
try:
	_ = hyperflow.VarVertex("vv", DG.Vertex())
	assert False
except LogicError as ex:
	assert str(ex).endswith("Can not create vertex indexed variable specifier from null-vertex.")

vvg = hyperflow.VarVertexGraph("vvg", g)
assert str(vvg) == "vvg[%s]" % g
assert repr(vvg) == "VarVertexGraph(%s, %s)" % (vvg.id, g)
assert vvg.id == "vvg"
assert vvg.graph == g

vvIdx = vsv[v]
assert type(vvIdx) == hyperflow.VarVertex
assert vvIdx.id == vsv.id
assert vvIdx.vertex == v
vvgIdx = vsv[g]
assert type(vvgIdx) == hyperflow.VarVertexGraph
assert vvgIdx.id == vsv.id
assert vvgIdx.graph == g
try:
	_ = vsv[DG.Vertex()]
	assert False
except LogicError as ex:
	assert str(ex).endswith("Can not create vertex indexed variable specifier from null-vertex.")


vse = hyperflow.VarSumEdge("vse")
assert str(vse) == "vse"
assert repr(vse) == "VarSumEdge(%s)" % vse
assert vse.id == str(vse)

ve = hyperflow.VarEdge("ve", e)
assert str(ve) == "ve[%s]" % e
assert repr(ve) == "VarEdge(%s, %s)" % (ve.id, e)
assert ve.id == "ve"
assert ve.edge == e
try:
	_ = hyperflow.VarEdge("ve", DG.HyperEdge())
	assert False
except LogicError as ex:
	assert str(ex).endswith("Can not create edge indexed variable specifier from null-edge.")

veIdx = vse[e]
assert type(veIdx) == hyperflow.VarEdge
assert veIdx.id == vse.id
assert veIdx.edge == e
try:
	_ = vse[DG.HyperEdge()]
	assert False
except LogicError as ex:
	assert str(ex).endswith("Can not create edge indexed variable specifier from null-edge.")


# TODO: remove when (.) is removed
vvIdxP = checkDeprecated(lambda: vsv(g))
veIdxP = checkDeprecated(lambda: vse(e))
assert str(vvgIdx) == str(vvIdxP)
assert str(veIdx) == str(veIdxP)


vsc = hyperflow.VarSumCustom("vsc")
assert str(vsc) == "vsc"
assert repr(vsc) == "VarSumCustom(%s)" % vsc
assert vsc.id == str(vsc)

vc = hyperflow.VarCustom("vc", "vcName")
assert str(vc) == "vc[vcName]"
assert repr(vc) == "VarCustom(%s, %s)" % (vc.id, vc.name)
assert vc.id == "vc"
assert vc.name == "vcName"

vcIdx = vsc["vcName"]
assert type(vcIdx) == hyperflow.VarCustom
assert vcIdx.id == vsc.id
assert vcIdx.name == "vcName"


# Operators
checkDeprecated(lambda: FlowLinExp())
exp = hyperflow.LinExp()

varTypes = [vsv, vv, vvg, vse, ve, vsc, vc, exp]

def linExp(e):
	res = hyperflow.LinExp(e)
def linCons(e):
	assert type(e) is hyperflow.LinConstraint

for first in varTypes:
	linExp(first)
	linExp(+first)
	linExp(-first)
	res = hyperflow.LinExp()
	res += first
	linExp(res)
	res = hyperflow.LinExp()
	res -= first
	linExp(res)
	linExp(42 * first)
	linExp(first * 42)
	linExp(3.14 * first)
	linExp(first * 3.14)
	linCons(first <= 42)
	linCons(first <= 3.14)
	linCons(first == 42)
	linCons(first == 3.14)
	linCons(first >= 42)
	linCons(first >= 3.14)
	linCons(42 <= first)
	linCons(3.14 <= first)
	linCons(42 == first)
	linCons(3.14 == first)
	linCons(42 >= first)
	linCons(3.14 >= first)
	for second in varTypes:
		linExp(first + second)
		linExp(first - second)


# Var compile, non-existing var specifiers
def check(flow):
	try:
		flow.calc()
		assert False
	except LogicError as e:
		assert str(e).endswith("Variable set not registered.")
for v in varTypes:
	if v == exp: continue
	flow = hyperflow.Model(dg)
	try:
		flow.addConstraint(v == 0)
		assert False
	except LogicError as e:
		assert str(e).endswith("Variable set not registered.")

	flow = hyperflow.Model(dg)
	try:
		flow.objectiveFunction = v
		assert False
	except LogicError as e:
		assert str(e).endswith("Variable set not registered.")
