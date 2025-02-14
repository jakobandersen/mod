include("common.py")

def check(sInput, exp=None):
	a = Graph.fromSMILES(sInput, allowAbstract=True)
	sOutput = a.smiles
	if exp is not None:
		assert sOutput == exp, f"\nexpected: {exp}, \noutput:   {sOutput}\ndfs: {a.graphDFS}"
	b = Graph.fromSMILES(sOutput, allowAbstract=True)
	if a.isomorphism(b) == 0:
		print(f"input:  {sInput}")
		print(f"output: {sOutput}")
		a.print()
		b.print()
		post.enableInvokeMake()
		assert False

check("[H][C][H]", "[CH2]")
check("[C][H]", "[CH]")
check("[C]:[H]", "[C]:[H]")
check("[C]=[H]", "[C]=[H]")
check("[C]#[H]", "[C]#[H]")
check("[C][H][C]", "[C][H][C]")
check("[C]:[H]:[C]", "[C]:[H]:[C]")
check("[C]=[H]=[C]", "[C]=[H]=[C]")
check("[C]#[H]#[C]", "[C]#[H]#[C]")
