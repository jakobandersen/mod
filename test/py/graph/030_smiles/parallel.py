include("common.py")

Graph.fromSMILES("C1.C1")
Graph.fromSMILES("C1(.C1)")

fail(lambda: Graph.fromSMILES("C1C1"),
	"SMILES string has parallel bonds at ring closure 1.",
	err=InputError)

fail(lambda: Graph.fromSMILES("C1(C1)"),
	"SMILES string has parallel bonds at ring closure 1.",
	err=InputError)
