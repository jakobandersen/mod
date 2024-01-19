include("../xxx_helpers.py")

def _compareDGs(dg, dg2, compareData=True):
	assert dg.numVertices == dg2.numVertices
	assert dg.numEdges == dg2.numEdges
	for v, v2 in zip(dg.vertices, dg2.vertices):
		assert v.id == v2.id
		assert v.inDegree == v2.inDegree, "{}, {}".format(v.inDegree, v2.inDegree)
		assert v.outDegree == v2.outDegree
		vOut = list(sorted(e.id for e in v.outEdges))
		v2Out = list(sorted(e.id for e in v2.outEdges))
		assert vOut == v2Out
		vIn = list(sorted(e.id for e in v.inEdges))
		v2In = list(sorted(e.id for e in v2.inEdges))
		assert vIn == v2In
		if compareData:
			assert v.graph == v2.graph
	for e, e2 in zip(dg.edges, dg2.edges):
		assert e.id == e2.id
		assert bool(e.inverse) == bool(e2.inverse)
		if e.inverse:
			assert e.inverse.id == e2.inverse.id
		assert e.numSources == e2.numSources
		assert e.numTargets == e2.numTargets
		eSrc = list(sorted(v.id for v in e.sources))
		e2Src = list(sorted(v.id for v in e2.sources))
		assert eSrc == e2Src
		eTar = list(sorted(v.id for v in e.targets))
		e2Tar = list(sorted(v.id for v in e2.targets))
		assert eTar == e2Tar
		if compareData:
			assert set(e.rules) == set(e2.rules)


def checkDGAfterBuild(dg):
	dumpName = dg.dump()
	dg2 = DG.load(dg.graphDatabase, inputRules, CWDPath(dumpName))
	dumpName2 = dg2.dump()

	_compareDGs(dg, dg2)
	_compareDumps(dumpName, dumpName2)


class BuilderHook:
	def __init__(self, orig):
		self.orig = orig

	def __getattr__(self, name):
		if name == "orig":
			return object.__getattr__(self, name)
		return self.orig.__getattribute__(name)

	def __enter__(self):
		return self.orig.__enter__()

	def __exit__(self, exc_type, exc_val, exc_tb):
		dg = self.orig.dg
		res = self.orig.__exit__(exc_type, exc_val, exc_tb)
		checkDGAfterBuild(dg)
		return res

DGBuild_orig = DG.build
DG.build = lambda dg: BuilderHook(DGBuild_orig(dg))


def disableBuildHook():
	DG.build = DGBuild_orig
