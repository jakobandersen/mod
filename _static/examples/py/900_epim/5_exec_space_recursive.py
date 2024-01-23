from mod.epim.transform.reduction_dg import ReductionDG
from mod.epim.process import Process, names
from mod.epim.recursive_process import RecursiveProcess

# Specify the names to be used:
x, y, z = names("x y z")

# Define a recursive process:
rp = RecursiveProcess()

# Specify the name, arguments, and behavior of a recursive process:
A = Process().input(x, y).call("A", [y])
rp.add("A", [x], A)

B = Process().output(x, x).call("B", [x])
rp.add("B", [x], B)

# Specify the behavior of a process:
P = A | B

# Compute the execution space for P provided with the recursive process rp:
exec_space = ReductionDG(P, rp)
exec_space.calc()

# Print the resulting derivation graph to the summary PDF:
exec_space.print()
# rst-name: Process Execution Spaces with Recursive Processes
# rst: We can compute the execution space of processes defined with recursive processes.
