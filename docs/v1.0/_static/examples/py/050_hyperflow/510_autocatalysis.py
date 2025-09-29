include("../020_dg/212_dgPredicate.py")
# Use the derivation graph 'dg' already created:
flow = hyperflow.Model(dg)
# Specify which molecules can be fed into the network:
flow.addSource(formaldehyde)
flow.addSource(glycolaldehyde)
# Specify which molecules that can remain in the network:
flow.addSink(glycolaldehyde)
# Enable constraints for autocatalysis:
flow.overallAutocatalysis.enable()
# Specify the minimization criteria:
#  number of unique reactions used
flow.objectiveFunction = isEdgeUsed
# Find a solution:
flow.findSolutions()
# Show solution information in the terminal:
flow.solutions.list()
# Print solutions:
flow.solutions.print()
# rst-name: Finding Autocatalytic Cycles
# rst: Some pathways have a specific higher-order structure, e.g., autocatalysis.
