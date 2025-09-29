include("../020_dg/212_dgPredicate.py")
# Use the derivation graph 'dg' already created:
flow = hyperflow.Model(dg)
# Specify which molecules can be fed into the network:
flow.addSource(formaldehyde)
flow.addSource(glycolaldehyde)
# Specify which molecules that can remain in the network:
flow.addSink(glycolaldehyde)
# Specify restrictions on the amount of input/output molecules:
flow.addConstraint(inFlow[formaldehyde] == 2)
flow.addConstraint(inFlow[glycolaldehyde] == 1)
flow.addConstraint(outFlow[glycolaldehyde] == 2)
# Disable too large molecules:
for v in dg.vertices:
	if v.graph.vLabelCount("C") > 4:
		flow.addConstraint(vertexFlow[v] == 0)
# Disable "strange" misleading input/output flows:
flow.allowIOReversal = False
# Specify the minimization criteria:
#  number of unique reactions used
flow.objectiveFunction = isEdgeUsed
# Find a solution:
flow.findSolutions()
# Show solution information in the terminal:
flow.solutions.list()
# Print solutions:
flow.solutions.print()
# rst-name: Extra Constraints
# rst: We can add many kinds of constraints. They do not need to be related to
# rst: input/ouput.
