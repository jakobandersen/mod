def check(a):
	data = {42: "C", 1337: "O", 0: "U"}
	for i in range(a.minExternalId, a.maxExternalId + 1):
		v = a.getVertexFromExternalId(i)
		if i in data:
			assert not v.isNull()
			assert v.stringLabel == data[i]
		else:
			assert v.isNull()
	for i, s in data.items():
		v = a.getVertexFromExternalId(i)
		assert not v.isNull()
		assert v.stringLabel == data[i]
		

# GML
a = Graph.fromGMLString("""graph [
	node [ id 42 label "C" ]
	node [ id 1337 label "O" ]
	node [ id 0 label "U" ]
	edge [ source 42 target 1337 label "-" ]
	edge [ source 1337 target 0 label "-" ]
]""")
check(a)

# DFS
a = Graph.fromDFS("C42O1337[U]0")
check(a)

# SMILES
a = Graph.fromSMILES("[C:42][O:1337][U:0]")
check(a)
a = Graph.fromSMILES("[C:0][C:0][C:1]")
assert not a.getVertexFromExternalId(0)
assert not a.getVertexFromExternalId(1)
a = Graph.fromSMILES("[C:0][C:0][C:1]", classPolicy=SmilesClassPolicy.NoneOnDuplicate)
assert not a.getVertexFromExternalId(0)
assert not a.getVertexFromExternalId(1)
try:
	Graph.fromSMILES("[C:0][C:0][C:1]", classPolicy=SmilesClassPolicy.ThrowOnDuplicate)
	assert False
except InputError as e:
	msg = "Error in SMILES conversion: class label 0 is used more than once (2), and the class label policy is throwOnDuplicate."
	assert str(e).endswith(msg)
a = Graph.fromSMILES("[C:0][C:0][C:1]", classPolicy=SmilesClassPolicy.MapUnique)
assert not a.getVertexFromExternalId(0)
assert a.getVertexFromExternalId(1)
