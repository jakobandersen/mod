include("xxx_common.py")

dgOther = DG()
dgOther.build()

for i in range(11):
	makeFlow("A -> B")
	flow.addSource(A)
	flow.addSink(B)
	if i == 0:
		flow.addConstraint(edgeFlow == 0)
		flow.addConstraint(edgeFlow >= 1)
		flow.findSolutions()
	else:
		flow.findSolutions(maxNumSolutions=i)
	r = flow.solutions
	assert str(r) == "HyperflowSolutionRange(model={}, size={})".format(flow, r.size)
	assert r.model == flow
	assert r.size == i
	assert len(r) == r.size
	assert sum(1 for s in r) == r.size
	r.list()
	r.print()
	r.print(hyperflow.Printer())

	fail(lambda: r.print(data=DGPrintData(dgOther)), "Print data is for another derivation graph, {}, than the flow solution, {}.".format(dgOther, dg))

	fail(lambda: r[-1], "Index -1 out of bounds.")
	fail(lambda: r[len(r)], "Index {} out of bounds.".format(len(r)))
	j = 0
	for s in r:
		assert s.model == r.model
		assert s.id == j
		assert str(s) == "HyperflowSolution(model={}, id={})".format(s.model, s.id), str(s)
		assert s == r[j]
		assert s.eval(inFlow) == j
		assert s.eval(outFlow) == j
		assert s.eval(edgeFlow) == j
		res = s.eval(edgeFlow)
		assert type(res) == int
		res = s.eval(0.0 * edgeFlow)
		assert type(res) == float
		s.list()
		s.print()
		s.print(hyperflow.Printer())
		fail(lambda: s.print(data=DGPrintData(dgOther)), "Print data is for another derivation graph, {}, than the flow solution, {}.".format(dgOther, dg))
		j += 1
