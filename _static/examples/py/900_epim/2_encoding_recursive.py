from mod.epim.process import Process, names

# MØD print options
printer = GraphPrinter()

# Specify the names to be used:
x, y = names("x y")

# Specify the behavior of a process
# Note, the second argument to call() is 
# the arguments passed to the recursive invocation of "A".
A = Process().input(x, y).call("A", [x])

# Encode A and put a visualisation of the encoding in the summary:
A.encode().print(printer)
# rst-name: A Recursive Encoding
# rst: We can also specify process calls in an encoding.
