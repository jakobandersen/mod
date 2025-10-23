include("common.py")

config.graph.smilesCheckAST = False


def check(p, atomId, charge):
	a = Graph.fromSMILES("[{}]".format(p))
	v = a.vertices[0]
	if v.atomId != atomId or v.charge != charge:
		print("AtomId:", v.atomId, "vs.", atomId)
		print("Charge:", v.charge, "vs.", charge)
		print("Label: ", v.stringLabel)
		assert False

check("Fe+++", AtomIds.Fe, 3)
check("Fe++", AtomIds.Fe, 2)
check("Fe+", AtomIds.Fe, 1)
check("Fe---", AtomIds.Fe, -3)
check("Fe--", AtomIds.Fe, -2)
check("Fe-", AtomIds.Fe, -1)

check("Fe+3", AtomIds.Fe, 3)
check("Fe+2", AtomIds.Fe, 2)
check("Fe+1", AtomIds.Fe, 1)
check("Fe-3", AtomIds.Fe, -3)
check("Fe-2", AtomIds.Fe, -2)
check("Fe-1", AtomIds.Fe, -1)
