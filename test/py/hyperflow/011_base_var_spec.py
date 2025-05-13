include("xx1_var_spec.py")

for vName in ["inFlow", "outFlow"]:
	registerVar("base", "vertex", vName, True, lambda s: None)
registerVar("base", "edge", "edgeFlow", True, lambda s: None)

def checkVertex(s):
	for v in s.model.dg.vertices:
		f = s.eval(vertexFlow[v])
		fCheck = s.eval(inFlow[v])
		for e in v.inEdges:
			fCheck += s.eval(edgeFlow[e])
		if f != fCheck:
			print("relaxed:", s.model.relaxed)
			print("f:      ", f)
			print("fCheck: ", fCheck)
		assert f == fCheck
def checkTransitInternal(s):
	for v in s.model.dg.vertices:
		pass
		# TODO
def checkIsInUsed(s):
	assert not s.model.relaxed
	for v in s.model.dg.vertices:
		i = s.eval(isInUsed[v])
		fi = s.eval(inFlow[v])
		assert (i == 0) == (fi == 0)
def checkIsOutUsed(s):
	assert not s.model.relaxed
	for v in s.model.dg.vertices:
		o = s.eval(isOutUsed[v])
		fo = s.eval(outFlow[v])
		assert (o == 0) == (fo == 0)
def checkIsInLessOut(s):
	assert not s.model.relaxed
	for v in s.model.dg.vertices:
		i = s.eval(isInLessOut[v])
		fi = s.eval(inFlow[v])
		fo = s.eval(outFlow[v])
		assert (i == 1) == (fi < fo)
def checkIsInGreaterOut(s):
	assert not s.model.relaxed
	for v in s.model.dg.vertices:
		i = s.eval(isInGreaterOut[v])
		fi = s.eval(inFlow[v])
		fo = s.eval(outFlow[v])
		assert (i == 1) == (fi > fo)
def checkIsInOutZero(s):
	assert not s.model.relaxed
	for v in s.model.dg.vertices:
		i = s.eval(isInOutZero[v])
		fi = s.eval(inFlow[v])
		fo = s.eval(outFlow[v])
		assert (i == 1) == (fi == 0 and fo == 0)
def checkIsVertexUsed(s):
	assert not s.model.relaxed
	for v in s.model.dg.vertices:
		i = s.eval(isVertexUsed[v])
		f = s.eval(vertexFlow[v])
		assert (i == 0) == (f == 0)
registerVar("base", "vertex", "vertexFlow", True, checkVertex)
registerVar("base", "vertex", "transitInternalFlow", True, checkTransitInternal)
registerVar("base", "vertex", "isInUsed", False, checkIsInUsed)
registerVar("base", "vertex", "isOutUsed", False, checkIsOutUsed)
registerVar("base", "vertex", "isInLessOut", False, checkIsInLessOut)
registerVar("base", "vertex", "isInGreaterOut", False, checkIsInGreaterOut)
registerVar("base", "vertex", "isInOutZero", False, checkIsInOutZero)
registerVar("base", "vertex", "isVertexUsed", False, checkIsVertexUsed)

def checkIsEdgeUsed(s):
	assert not s.model.relaxed
	for e in s.model.dg.edges:
		i = s.eval(isEdgeUsed[e])
		f = s.eval(edgeFlow[e])
		assert (i == 0) == (f == 0)
def checkIsBothReverseUsed(s):
	assert not s.model.relaxed
	for e in s.model.dg.edges:
		if not e.inverse: continue
		i = s.eval(isBothReverseUsed[e])
		f1 = s.eval(edgeFlow[e])
		f2 = s.eval(edgeFlow[e.inverse])
		if not ( (i == 1) == (f1 != 0 and f2 != 0) ):
			print("i: ", i)
			print("f1:", f1)
			print("f2:", f2)
		assert (i == 1) == (f1 != 0 and f2 != 0)
registerVar("base", "edge", "isEdgeUsed", False, checkIsEdgeUsed)
registerVar("base", "edge", "isBothReverseUsed", False, checkIsBothReverseUsed)
