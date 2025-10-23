
Graph.fromSMILES("[O]")
for a in inputGraphs: a.print()

print("inputGraphs:\t", inputGraphs)

a = Graph.fromDFS("COCC-O-C#N")
a.print()
a = Graph.fromDFS("COOC")
a.print()
a.printGML()
print(a.getGMLString())

print("name", "smiles", "graphDFS", "linearEncoding", sep="\t")
print(a.name, a.smiles, a.graphDFS, a.linearEncoding, sep="\t")

print("isMolecule:\t", a.isMolecule)
#print("energy:\t", a.energy)
a = Graph.fromDFS("COCC=O")
a.cacheEnergy(42)
print("energy:\t", a.energy)
print("exactMass:\t", a.exactMass)

print("vLabelCount:\t", a.vLabelCount("O"))
print("eLabelCount:\t", a.eLabelCount("="))

a.name = "Test"
print("setName:\t", a)
print("inputGraphs:\t", inputGraphs)

# Check eq operator
inputGraphs[:] = []

a = Graph.fromSMILES("C")
print("Energy:", a.energy)

a = Graph.fromSMILES("O=C(O)C(CC(=O)O)C(O)C(=O)O", name="Isocitrate")
aPerm = a.makePermutation()

a = Graph.fromSMILES("C*C")
a.print()
a = Graph.fromSMILES("N[*]N")
a.print()
