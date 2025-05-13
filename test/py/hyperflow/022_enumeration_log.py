include("xxx_common.py")

makeDG("A -> B")

for v in range(3):
	for iv in range(3):
		print("#" * 60)
		print("# verbosity={}, ilpVerbosity={}".format(v, iv))
		print("#" * 60)
		flow = hyperflow.Model(dg)
		flow.addSource(A)
		flow.addSink(B)
		flow.addConstraint(inFlow[A] <= 1)
		flow.findSolutions(maxNumSolutions=9999, verbosity=v, ilpVerbosity=iv)
		print("Number of solutions found:", len(flow.solutions))
		print()
