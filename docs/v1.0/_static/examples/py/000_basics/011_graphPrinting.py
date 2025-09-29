# Our test graph, representing the molecule caffeine:
g = smiles('Cn1cnc2c1c(=O)n(c(=O)n2C)C')
# ;ake an object to hold our settings:
p = GraphPrinter()
# First try visualising without any prettifications:
p.disableAll()
g.print(p)
# Now make chemical edges look like bonds, and put colour on atoms.
# Also put the "charge" part of vertex labels in superscript:
p.edgesAsBonds = True
p.raiseCharges=True
p.withColour = True
g.print(p)
# We can also "collapse" normal hydrogen atoms into the neighbours,
# and just show a count:
p.collapseHydrogens = True
g.print(p)
# And finally we can make "internal" carbon atoms simple lines:
p.simpleCarbons = True
g.print(p)
# There are also options for adding indices to the vertices,
# and modify the rendering of labels and edges:
p2 = GraphPrinter()
p2.disableAll()
p2.withTexttt = True
p2.thick = True
p2.withIndex = True
# We can actually print two different versions at the same time:
g.print(p2, p)
# rst-name: Printing Graphs/Molecules
# rst: The visualisation of graphs can be "prettified" using special printing
# rst: options.
# rst: The changes can make the graphs look like normal molecule visualisations.
