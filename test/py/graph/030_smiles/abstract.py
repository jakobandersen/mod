include("common.py")

def check(p, name, *, checkOutput=True):
	sInput = "[{}]".format(p.format(name))
	a = Graph.fromSMILES(sInput, allowAbstract=True)
	s = next(iter(a.vertices)).stringLabel
	if s != name:
		msg = f"Loading problem, differing label:\nname:   {name}\nactual: {s}"
		assert False, msg
	if checkOutput:
		sOutput = a.smiles
	return a

check("{}", "C")
a = check("{}:42", "C")
assert a.getVertexFromExternalId(42)
config.graph.appendSmilesClass = True
a = check("{}", "C:42", checkOutput=False)
assert a.getVertexFromExternalId(42)
config.graph.appendSmilesClass = False

check("{}", "*")
a = check("{}:42", "*")
assert a.getVertexFromExternalId(42)
config.graph.appendSmilesClass = True
a = check("{}:42", "*")  # class not appended to * as a special case
assert a.getVertexFromExternalId(42)
config.graph.appendSmilesClass = False

check("{}", "abc")
a = check("{}:42", "abc")
assert a.getVertexFromExternalId(42)
config.graph.appendSmilesClass = True
a = check("{}", "abc:42", checkOutput=False)
assert a.getVertexFromExternalId(42)
config.graph.appendSmilesClass = False

check("{}", "abc")
check("{}", "[]")
check("{}", "[def]")
check("{}", "abc[def]")
check("{}", "[def]efg")
check("{}", "abc[def]efg")
check("{}", "abc[def]ghi[jkl]mno")
check("{}", "[abc][def][ghi]")

check("{}", "42")
check("{}", "42Heblah")
check("{}", "He@blah")
check("{}", "HeHblah")
check("{}", "He+blah")
check("{}", "He.blah")


config.graph.useWrongSmilesCanonAlg = True
Graph.fromSMILES("[*]").smiles
fail(lambda: Graph.fromSMILES("C[*]").smiles,
	"Can not use the broken CANGEN canonicalization algorithm with non-molecules.",
	isSubstring=True)
fail(lambda: Graph.fromSMILES("CC[*]").smiles,
	"Can not use the broken CANGEN canonicalization algorithm with non-molecules.",
	isSubstring=True)
config.graph.useWrongSmilesCanonAlg = False


###############################################################################
# Multiple atoms
###############################################################################

def checkMulti(sInput):
	a = Graph.fromSMILES(sInput, allowAbstract=True)
	sOutput = a.smiles
	b = Graph.fromSMILES(sOutput, allowAbstract=True)
	if a.isomorphism(b) == 0:
		print(f"input:  {sInput}")
		print(f"output: {sOutput}")
		a.print()
		b.print()
		post.enableInvokeMake()
		assert False

checkMulti("[13abstract]")
checkMulti("[abstract+]")
checkMulti("[abstract.]")
checkMulti("[abstract][H]")


###############################################################################
# Bad ones
###############################################################################

def checkBad(s, msg):
	fail(lambda: Graph.fromDFS(s).smiles, msg, isSubstring=True)

checkBad("[:]",       "Colon character in vertex label of vertex 0")
checkBad("[foo:]",    "Colon character in vertex label of vertex 0")
checkBad("[:bar]",    "Colon character in vertex label of vertex 0")
checkBad("[foo:bar]", "Colon character in vertex label of vertex 0")

checkBad(R"[\]]",       "Unbalanced square brackets in vertex label of vertex 0")
checkBad(R"[foo\]]",    "Unbalanced square brackets in vertex label of vertex 0")
checkBad(R"[\]bar]",    "Unbalanced square brackets in vertex label of vertex 0")
checkBad(R"[foo\]bar]", "Unbalanced square brackets in vertex label of vertex 0")

checkBad("[[]",       "Open square bracket in vertex label of vertex 0")
checkBad("[foo[]",    "Open square bracket in vertex label of vertex 0")
checkBad("[[bar]",    "Open square bracket in vertex label of vertex 0")
checkBad("[foo[bar]", "Open square bracket in vertex label of vertex 0")

checkBad("[C]{X}[O]", "Can not canonicalise arbitrary edge labels.")
