mol1 = smiles("CC(C)CO")
mol2 = smiles("C(CC)CO")
# Check if there is just one isomorphism between the graphs:
isomorphic = mol1.isomorphism(mol2) == 1
print("Isomorphic?", isomorphic)
# Find the number of automorphisms in the graph,
# by explicitly enumerating all of them:
numAutomorphisms = mol1.isomorphism(mol1, maxNumMatches=2**30)
print("|Aut(G)| =", numAutomorphisms)
# Let's count the number of methyl groups:
methyl = smiles("[CH3]")
# The symmetry of the group it self should not be counted,
# so find the size of the automorphism group of methyl.
numAutMethyl = methyl.isomorphism(methyl, maxNumMatches=2**30)
print("|Aut(methyl)|", numAutMethyl)
# Now find the number of methyl matches,
numMono = methyl.monomorphism(mol1, maxNumMatches=2**30)
print("#monomorphisms =", numMono)
# and divide by the symmetries of methyl.
print("#methyl groups =", numMono / numAutMethyl)
# rst-name: Graph Morphisms
# rst: Graph objects have methods for finding morphisms with the VF2 algorithms
# rst: for isomorphism and monomorphism.
# rst: We can therefore easily detect isomorphic graphs, count automorphisms,
# rst: and search for substructures.
