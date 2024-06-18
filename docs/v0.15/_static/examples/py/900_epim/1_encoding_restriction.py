from mod.epim.process import Process, names

# MØD print options
printer = GraphPrinter()

# Specify the names to be used:
x, y, z, w = names("x y z w")

# Specify the behavior of a process
p1 = Process().input(x, z).output(z, w)
p2 = Process().restrict(x).output(x, y)
P = p1 | p2

# Encode the process as a MØD graph:
G = P.encode()

# Put a visualisation of the encoding in the summary:
G.print(printer)
# rst-name: A Process Encoding with Restrictions
# rst: We can restrict names of a process.
