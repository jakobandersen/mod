include("../xxx_helpers.py")

def makeDG(s):
	dg = DG()
	dg.build().addAbstract(s)
	globals()['dg'] = dg
	for v in dg.vertices:
		globals()[v.graph.name] = v

def makeFlow(s):
	makeDG(s)
	globals()['flow'] = hyperflow.Model(dg)

def enabled(f):
	fail(f, "Module not enabled.")

def alreadyEnabled(f):
	fail(f, "Module already enabled.")

def locked(f):
	fail(f, "Specification locked.")

def unlocked(f):
	fail(f, "Specification not yet locked.")
