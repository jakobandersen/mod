from mod.epim.process import Process, names

# MØD print options
printer = GraphPrinter()

# Specify the names to be used:
x, y, z, w = names("x y z w")

# Specify the behavior of a process
p1 = Process().input(x, z).output(z, w)
p2 = Process().output(x, y)
P = p1 | p2

# Encode the process as a MØD graph:
G = P.encode()

# Put a visualisation of the encoding in the summary:
G.print(printer)
# rst-name: A Basic Process Encoding
# rst: These examples shows the basic usage of EpiM.
# rst: After each run a PDF summary is compiled.
# rst: We let in(x) and out(x) denote the input and output operators of a 
# rst: process respectively.
