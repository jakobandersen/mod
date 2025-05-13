include("xxx_common.py")

makeFlow("A -> B")
bVar = flow.addBoolVariable("boolVarName")
iVar = flow.addIntVariable("intVarName")
fVar = flow.addFloatVariable("floatVarName")
flow.findSolutions()
