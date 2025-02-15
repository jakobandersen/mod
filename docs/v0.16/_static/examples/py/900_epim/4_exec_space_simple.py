from mod.epim.transform.reduction_dg import ReductionDG
from mod.epim.process import Process, names

# Specify the names to be used:
x, y, z, w = names("x y z w")

# Specify the behavior of a process:
p1 = Process().input(x, z).output(z, w)
p2 = Process().output(x, y) + Process().output(x, y)
P = (p1 | p2)

# Compute the execution space for P
exec_space = ReductionDG(P)
exec_space.calc()

# Print the resulting derivation graph to the summary PDF:
exec_space.print()
# exec_space.print(True) #prints the entire derivation graph
# rst-name: Compute Execution Spaces
# rst: We can compute the execution space of a process.
