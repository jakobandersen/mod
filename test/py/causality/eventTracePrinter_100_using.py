dg = DG()
dg.build().addAbstract("A -> B  A -> C")
A = next(v for v in dg.vertices if v.graph.name == "A")
B = next(v for v in dg.vertices if v.graph.name == "B")
C = next(v for v in dg.vertices if v.graph.name == "C")
eB = dg.findEdge([A], [B])
assert eB
eC = dg.findEdge([A], [C])
assert eC
net = causality.Net(dg)
marking = causality.Marking(net)
marking.add(A, 200000)
trace = causality.EventTrace(marking)
for i in range(500):
	for j in range(i // 2):
		trace.add(i, causality.EdgeAction(eB))
	for j in range(i):
		trace.add(i, causality.EdgeAction(eC))

post.summarySection("Default")
trace.print()

post.summarySection("2 points per vertex")
p = causality.EventTracePrinter()
p.maxPointsPerVertex = 2
trace.print(p)

post.summarySection("log time")
p = causality.EventTracePrinter()
p.logTime = True
trace.print(p)

post.summarySection("log count")
p = causality.EventTracePrinter()
p.logCount = True
trace.print(p)

post.summarySection("log time, log count")
p = causality.EventTracePrinter()
p.logTime = True
p.logCount = True
trace.print(p)

post.summarySection("options")
p = causality.EventTracePrinter()
p.pushOptions("width=0.5\\textwidth")
trace.print(p)
p.pushOptions("legend columns=1")
trace.print(p)

post.summarySection("vertex visible")
p = causality.EventTracePrinter()
p.pushVertexVisible(lambda v: v.graph.name != "A")
trace.print(p)

post.summarySection("vertex options")
p = causality.EventTracePrinter()
p.maxPointsPerVertex = 10
p.pushVertexOptions(lambda v: "red" if v.graph.name == "A" else "")
trace.print(p)
p.pushVertexOptions(lambda v: "mark=*" if v.graph.name == "A" else "mark=-")
trace.print(p)

post.summarySection("pre-content")
p = causality.EventTracePrinter()
p.setPreContent(R"\draw[fill=blue] (0, 0) rectangle (250, 100);")
trace.print(p)

post.summarySection("post-content")
p = causality.EventTracePrinter()
p.setPostContent(R"\draw[fill=blue] (0, 0) rectangle (250, 100);")
trace.print(p)
