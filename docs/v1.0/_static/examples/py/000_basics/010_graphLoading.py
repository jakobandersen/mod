# Load a graph from a SMILES string (only for molecule graphs):
ethanol1 = smiles("CCO", name="Ethanol1")
# Load a graph from a SMILES-like format, called "GraphDFS", but for general graphs:
ethanol2 = graphDFS("[C]([H])([H])([H])[C]([H])([H])[O][H]", name="Ethanol2")
# The GraphDFS format also supports implicit hydrogens:
ethanol3 = graphDFS("CCO", name="Ethanol3")
# The basic graph format is GML:
ethanol4 = graphGMLString("""graph [
	node [ id 0 label "C" ]
	node [ id 1 label "C" ]
	node [ id 2 label "O" ]
	node [ id 3 label "H" ]
	node [ id 4 label "H" ]
	node [ id 5 label "H" ]
	node [ id 6 label "H" ]
	node [ id 7 label "H" ]
	node [ id 8 label "H" ]
	edge [ source 1 target 0 label "-" ]
	edge [ source 2 target 1 label "-" ]
	edge [ source 3 target 0 label "-" ]
	edge [ source 4 target 0 label "-" ]
	edge [ source 5 target 0 label "-" ]
	edge [ source 6 target 1 label "-" ]
	edge [ source 7 target 1 label "-" ]
	edge [ source 8 target 2 label "-" ]
]""", name="Ethanol4")
# They really are all loading the same graph into different objects:
assert ethanol1.isomorphism(ethanol2) == 1
assert ethanol1.isomorphism(ethanol3) == 1
assert ethanol1.isomorphism(ethanol4) == 1
# and they can be visualised:
ethanol1.print()
# All loaded graphs are added to a list 'inputGraphs':
for g in inputGraphs:
	g.print()
# rst-name: Graph Loading
# rst: Molecules are encoded as attributed graphs.
# rst: They can be loaded from SMILES strings, and in general any graph can be
# rst: loaded from a GML specification, or from the SMILES-like format GraphDFS.
