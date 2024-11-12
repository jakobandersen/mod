include("../xxx_helpers.py")

def commonChecks(r):
	# round-trip as GML
	s = r.getGMLString()
	rCopy = Rule.fromGMLString(s)
	assert r.isomorphism(rCopy) != 0
	sCopy = rCopy.getGMLString()
	if s != sCopy:
		print("=" * 80)
		print("Originally generated GML:")
		print(s)
		print("=" * 80)
		print("Generated GML from reloaded GML:")
		print(sCopy)
		assert False


def gmlFail(s, pattern):
	fail(lambda: Rule.fromGMLString("rule [ %s ]" % s),
		pattern=pattern, err=InputError, isSubstring=True)


def dfsFail(s, pattern):
	fail(lambda: Rule.fromDFS(s),
		pattern=pattern, err=InputError, isSubstring=True)


def dfsCheck(dfsInput, gmlInput):
	print("DFS:", dfsInput)
	dfs = Rule.fromDFS(dfsInput, "DFS")
	gml = Rule.fromGMLString("rule [ %s ]" % gmlInput, "GML")
	if dfs.isomorphism(gml) != 1:
		print("DFS Input:", dfs)
		print("GML Input: rule [\n%s\n]" % gmlInput)
		dfs.print()
		gml.print()
		post.enableInvokeMake()
		assert False, "Run mod_post to see rules."

	commonChecks(dfs)
