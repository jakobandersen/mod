include("../xxx_graphInterface.py")

g = Graph.fromDFS("CN1C=NC2=C1C(=O)N(C(=O)N2C)C{blah}[blah]:[blah]")

checkLabelledGraph(g, string="'{}'".format(g.name),
	vertexString="GraphVertex", edgeString="GraphEdge")
