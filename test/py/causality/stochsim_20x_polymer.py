post.disableInvokeMake()

A = Graph.fromDFS("[A]", "A")
B = Graph.fromDFS("[B]", "B")
rAdd = Rule.fromGMLString("""rule [
	labelType "term"
	context [
		node [ id 0 label "*" ]
		node [ id 1 label "*" ]
	]
	right [
		edge [ source 0 target 1 label "-" ]
	]
	constrainAdj [ id 0 op "<=" count 1 ]
	constrainAdj [ id 1 op "<=" count 1 ]
]""")
rDel = Rule.fromDFS("[*]1[*]2>>[*]1.[*]2")

ls = LabelSettings(LabelType.Term, LabelRelation.Specialisation)

expandIter = [
	rDel,
	leftPredicate[lambda d: all(g.numVertices <= 3 for g in d.left)](rAdd)
]

def expandAll(b, s, u):
	b.execute(addSubset(A, B) >> repeat(expandIter))
	return False


_expand = True
expands = 0
expandsAvoided = 0
def onIterationBegin(s):
	global expands
	global expandsAvoided
	print(f"Iteration: {s.iteration:>5}   time = {s.time:>6,.2f}   |V| = {s.dg.numVertices:>5}")
	if not _expand:
		return
	if expands > 0:
		print("\tExpands:", expands)
		expands = 0
	if expandsAvoided > 0:
		print("\tExpandsAvoided:", expandsAvoided)
		expandsAvoided = 0

def onExpand(i):
	global expands
	expands += 1
	#print("\tExpanding")

def onExpandAvoided(i):
	global expandsAvoided
	expandsAvoided += 1

def setCallbacks(*, everyIter: int, expand: bool):
	global _expand
	_expand = expand
	sim.setOnIterationBegin(onIterationBegin, everyIter)
	if expand:
		sim.onExpand = onExpand
		sim.onExpandAvoided = onExpandAvoided

