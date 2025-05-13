include("xxx_common.py")
post.enableInvokeMake()

makeDG("""
	A1 <=> B1
	B1 + 2 F1 <=> C1
	C1 <=> A1 + B1

	A2 <=> B2
	B2 <=> C2
	C2 <=> A2
""")

flow = hyperflow.Model(dg)
for a in (F1, A1, A2):
	flow.addSource(a)
for a in (A1, A2):
	flow.addSink(a)
flow.overallAutocatalysis.enable()
flow.overallAutocatalysis.forceExistence = False
flow.overallCatalysis.enable()
flow.overallCatalysis.forceExistence = False
flow.findSolutions(maxNumSolutions=5)

p = hyperflow.Printer()
p.unfilteredFlowColour = "red"
p.pushInEdgeLabel(lambda v: v.graph.name + ", in")
p.pushOutEdgeLabel(lambda v: v.graph.name + ", out")
flow.solutions.print(p)
p.popInEdgeLabel()
p.popOutEdgeLabel()
