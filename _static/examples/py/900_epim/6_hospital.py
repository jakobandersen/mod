from mod.epim.process import Process, names, Name, process_config
from mod.epim.transform.reduction_dg import ReductionDG
from mod.epim.recursive_process import RecursiveProcess

# Do not print the arguments of an encoded process call:
process_config.print_args = False

# Specify the names to be used:
s, n, pn, j, h, cu, ki, d, x = names("s n pn j h cu ki d x")
free_names = [s, n, ki, cu]

# Define a recursive process:
rp = RecursiveProcess()

# Specify the name, arguments, and behavior of a recursive process:
P = Process().output(s, n).input(n, d).call("Pp", free_names)
rp.add("P", free_names, P)

Pp = Process().input(ki, x) + Process().input(cu, x).call("P", free_names)
rp.add("Pp", free_names, Pp)

J = Process().input(s, pn).output(pn, j).output(cu, j).call("J", free_names)
rp.add("J", free_names, J)

H = Process().input(s, pn).output(pn, h).output(ki, h).call("H", free_names)
rp.add("H", free_names, H)

# Define the process Hospital:
Hospital = P | J | H

# Compute the execution space of Hospital given the recursive process rp:
exec_space = ReductionDG(Hospital, rp)
exec_space.calc()

# Print the resulting derivation graph to the summary PDF:
exec_space.print()
# rst-name: Hospital
# rst: The execution space for the process Hospital. From the execution space, 
# rst: we see that Hospital can end up in a deadlock. This happens specifically,
# rst: when the process P synchronizes with the process H.
