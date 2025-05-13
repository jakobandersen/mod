include("xx2_var_compile.py")
include("011_base_var_spec.py")

checkModule("base")


# Detailed checks of transitInternal
# ==================================
include("xxx_common.py")
dg = DG()
dg.build().addAbstract("A -> B")
A = next(v for v in dg.vertices if v.graph.name == "A")
B = next(v for v in dg.vertices if v.graph.name == "B")

# Sum version, but not all available
msg = "Can not compile sum of variable set transitInternalFlow."
msg += " Not all vertices have IO and internal transit flow separated."
msg += " Use the separateIOInternalTransit() method on all vertices"
msg += " before using the sum-version of this specifier."

flow = hyperflow.Model(dg)
flow.addConstraint(transitInternalFlow == 0)
fail(lambda: flow.findSolutions(), msg)

flow = hyperflow.Model(dg)
flow.separateIOInternalTransit(A)
flow.addConstraint(transitInternalFlow == 0)
fail(lambda: flow.findSolutions(), msg)

flow = hyperflow.Model(dg)
flow.separateIOInternalTransit(A)
flow.separateIOInternalTransit(B)
flow.addConstraint(transitInternalFlow == 0)
flow.findSolutions()

# Indexed version
flow = hyperflow.Model(dg)
flow.addConstraint(transitInternalFlow[A] == 0)
msg = "Can not compile indexed variable set transitInternalFlow for vertex {} ({}). It does not have IO and internal transit flow separated. Use the separateIOInternalTransit() method on the vertex first.".format(A, A.graph.name)
fail(lambda: flow.findSolutions(), msg)
