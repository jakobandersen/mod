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
# Disable "strange" misleading input/output flows:
flow.allowIOReversal = False
# Specify the minimization criteria:
#  number of reactions
flow.objectiveFunction = edgeFlow
# Find solutions:
#  at most 10 solutions, any quality
flow.findSolutions(maxNumSolutions=10)
# Show solution information in the terminal:
flow.solutions.list()
# Print solutions:
flow.solutions.print()
# rst-name: Multiple Solutions
# rst: It is often interesting to look for alternate solutions,
# rst: possibly with a sub-optimal objective value.
