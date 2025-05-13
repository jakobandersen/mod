include("xxx_common.py")

makeFlow("A -> B")
flow.findSolutions(maxNumSolutions=0)
view = flow.implementationView

print(f"numMILPVariables:   {view.numMILPVariables}")
print(f"numMILPConstraints: {view.numMILPConstraints}")
