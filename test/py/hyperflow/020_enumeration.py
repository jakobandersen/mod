include("xx5_solutions.py")
verbosity = 2

makeDG("A -> B B -> C B -> D")

print("#" * 80)
print("General")
print("#" * 80)
flow = hyperflow.Model(dg)
flow.addSource(A)
flow.addSink(C)
flow.addSink(D)
flow.addConstraint(inFlow[A] <= 2)
checkSolutions(flow, [
	[], # 0
	[ # 1
		(inFlow, A, 1),
		(outFlow, D, 1),
		(edgeFlow, dg.findEdge([A], [B]), 1),
		(edgeFlow, dg.findEdge([B], [D]), 1),
	],
	[ # 2
		(inFlow, A, 1),
		(outFlow, C, 1),
		(edgeFlow, dg.findEdge([A], [B]), 1),
		(edgeFlow, dg.findEdge([B], [C]), 1),
	],
	[ # 3
		(inFlow, A, 2),
		(outFlow, D, 2),
		(edgeFlow, dg.findEdge([A], [B]), 2),
		(edgeFlow, dg.findEdge([B], [D]), 2),
	],
	[ # 4
		(inFlow, A, 2),
		(outFlow, C, 1),
		(outFlow, D, 1),
		(edgeFlow, dg.findEdge([A], [B]), 2),
		(edgeFlow, dg.findEdge([B], [C]), 1),
		(edgeFlow, dg.findEdge([B], [D]), 1),
	],
	[ # 5
		(inFlow, A, 2),
		(outFlow, C, 2),
		(edgeFlow, dg.findEdge([A], [B]), 2),
		(edgeFlow, dg.findEdge([B], [C]), 2),
]], maxNumSolutions=100)


print("#" * 80)
print("MaxNumSolutions")
print("#" * 80)
flow = hyperflow.Model(dg)
flow.addSource(A)
flow.addSink(C)
flow.addSink(D)
flow.addConstraint(inFlow[A] <= 2)
checkSolutions(flow, [
	[], # 0
	[ # 1
		(inFlow, A, 1),
		(outFlow, D, 1),
		(edgeFlow, dg.findEdge([A], [B]), 1),
		(edgeFlow, dg.findEdge([B], [D]), 1),
	],
	[ # 2
		(inFlow, A, 1),
		(outFlow, C, 1),
		(edgeFlow, dg.findEdge([A], [B]), 1),
		(edgeFlow, dg.findEdge([B], [C]), 1),
]], maxNumSolutions=3)


print("#" * 80)
print("AbsGap")
print("#" * 80)
flow = hyperflow.Model(dg)
flow.addSource(A)
flow.addSink(C)
flow.addSink(D)
flow.addConstraint(inFlow[A] <= 2)
checkSolutions(flow, [
	[], # 0
	[ # 1
		(inFlow, A, 1),
		(outFlow, D, 1),
		(edgeFlow, dg.findEdge([A], [B]), 1),
		(edgeFlow, dg.findEdge([B], [D]), 1),
	],
	[ # 2
		(inFlow, A, 1),
		(outFlow, C, 1),
		(edgeFlow, dg.findEdge([A], [B]), 1),
		(edgeFlow, dg.findEdge([B], [C]), 1),
]], maxNumSolutions=100, absGap=3)

print("=" * 80)
print("AbsGap 2")
flow = hyperflow.Model(dg)
flow.addSource(A)
flow.addSink(C)
flow.addSink(D)
flow.addConstraint(inFlow[A] <= 2)
flow.addConstraint(inFlow[A] >= 1)
checkSolutions(flow, [
	[ # 0
		(inFlow, A, 1),
		(outFlow, C, 1),
		(edgeFlow, dg.findEdge([A], [B]), 1),
		(edgeFlow, dg.findEdge([B], [C]), 1),
	],
	[ # 1
		(inFlow, A, 1),
		(outFlow, D, 1),
		(edgeFlow, dg.findEdge([A], [B]), 1),
		(edgeFlow, dg.findEdge([B], [D]), 1),
]], maxNumSolutions=100, absGap=0)


print("#" * 80)
print("EnumerationVars")
print("#" * 80)
flow = hyperflow.Model(dg)
flow.addSource(A)
flow.addSink(C)
flow.addSink(D)
flow.addConstraint(inFlow[A] <= 2)
flow.addEnumerationVar(isEdgeUsed)
checkSolutions(flow, [
	[], # 0
	[ # 1
		(inFlow, A, 1),
		(outFlow, C, 1),
		(edgeFlow, dg.findEdge([A], [B]), 1),
		(edgeFlow, dg.findEdge([B], [C]), 1),
	],
	[ # 2
		(inFlow, A, 1),
		(outFlow, D, 1),
		(edgeFlow, dg.findEdge([A], [B]), 1),
		(edgeFlow, dg.findEdge([B], [D]), 1),
	],
	[ # 3
		(inFlow, A, 2),
		(outFlow, C, 1),
		(outFlow, D, 1),
		(edgeFlow, dg.findEdge([A], [B]), 2),
		(edgeFlow, dg.findEdge([B], [C]), 1),
		(edgeFlow, dg.findEdge([B], [D]), 1),
]], maxNumSolutions=100)

print("=" * 80)
print("EnumerationVars 2")
flow = hyperflow.Model(dg)
flow.addSource(A)
flow.addSink(C)
flow.addSink(D)
flow.addConstraint(inFlow[A] <= 2)
flow.addEnumerationVar(edgeFlow[dg.findEdge([A], [B])])
checkSolutions(flow, [
	[],
	SolChoice([
		[
			(inFlow, A, 1),
			(outFlow, C, 1),
			(edgeFlow, dg.findEdge([A], [B]), 1),
			(edgeFlow, dg.findEdge([B], [C]), 1),
		],
		[
			(inFlow, A, 1),
			(outFlow, D, 1),
			(edgeFlow, dg.findEdge([A], [B]), 1),
			(edgeFlow, dg.findEdge([B], [D]), 1),
		]
	]),
	SolChoice([
		[
			(inFlow, A, 2),
			(outFlow, C, 2),
			(edgeFlow, dg.findEdge([A], [B]), 2),
			(edgeFlow, dg.findEdge([B], [C]), 2),
		],
		[
			(inFlow, A, 2),
			(outFlow, D, 2),
			(edgeFlow, dg.findEdge([A], [B]), 2),
			(edgeFlow, dg.findEdge([B], [D]), 2),
		]
	])
], maxNumSolutions=100)


print("#" * 80)
print("addTransitEnumeration")
print("#" * 80)
makeFlow("A <=> O B <=> O C <=> O D <=> O")
flow.addSource(A)
flow.addSource(B)
flow.addSink(C)
flow.addSink(D)
flow.addConstraint(inFlow[A] == 1)
flow.addConstraint(inFlow[B] == 1)
flow.addConstraint(outFlow[C] == 1)
flow.addConstraint(outFlow[D] == 1)
flow.allowIOReversal = True
flow.addTransitEnumeration(O)
s = [
	(inFlow, A, 1),
	(inFlow, B, 1),
	(outFlow, C, 1),
	(outFlow, D, 1),
	(edgeFlow, dg.findEdge([A], [O]), 1),
	(edgeFlow, dg.findEdge([B], [O]), 1),
	(edgeFlow, dg.findEdge([O], [C]), 1),
	(edgeFlow, dg.findEdge([O], [D]), 1),
]
checkSolutions(flow, [s, s], maxNumSolutions=100)
flow.implementationView.printExpandedVertex(O.graph)
flow.solutions.print()
