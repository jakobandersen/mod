include("common.py")

fail(lambda: Graph.fromSMILES("C-1CCCC=1"),
	"Error in SMILES conversion: ring closure 1 can not be both '-' and '='.",
	err=InputError)
