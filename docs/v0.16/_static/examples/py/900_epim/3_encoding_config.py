from mod.epim.gen_image import image_config
from mod.epim.process import Process, names

# Show the names of a process encoding: 
image_config.show_variables = True

# Show the pointer vertices used for encoding process calls:
image_config.show_pointers = True

# Show the special root vertex "go" of an encoding:
image_config.show_root = True

# Hide implementation detail specific vertices such as 
# the vertices with terms t(p) or t(s) with degrees of 2:
image_config.collapse_two_edges = False

# MØD print options
printer = GraphPrinter()

# Specify the names to be used:
x, y, z, w = names("x y z w")

# Specify the behavior of a process:
p1 = Process().input(x, z).output(z, w)
p2 = Process().output(x, y)
P = p1 | p2

# Encode and print P
G = P.encode()
G.print(printer)
# rst-name: Config for Visualization of Encodings
# rst: The graphs of the encoded processes can be filtered and "prettified" using either
# rst: the print options used by MØD for graph visualization or the image config provided
# rst: by EpiM.
