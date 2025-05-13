include("xx5_solutions.py")
include("091_userDefined_var_spec.py")

makeFlow("A -> B")
flow.addSource(A)
flow.addSink(B)
var = flow.addBoolVariable("boolVarName")
flow.addConstraint(var - edgeFlow == 0)
flow.addConstraint(var == 1)
checkSolutions(flow, [[
	(inFlow, A, 1),
	(outFlow, B, 1),
	(edgeFlow, dg.findEdge([A], [B]), 1),
	(hyperflow.VarSumCustom(var.id), var.name, 1),
]], maxNumSolutions=100)


makeFlow("A -> B")
flow.addSource(A)
flow.addSink(B)
var = flow.addIntVariable("intVarName")
flow.addConstraint(var - edgeFlow == 1)
flow.addConstraint(var == 2)
checkSolutions(flow, [[
	(inFlow, A, 1),
	(outFlow, B, 1),
	(edgeFlow, dg.findEdge([A], [B]), 1),
	(hyperflow.VarSumCustom(var.id), var.name, 2),
]], maxNumSolutions=100)


makeFlow("A -> B")
flow.addSource(A)
flow.addSink(B)
var = flow.addFloatVariable("floatVarName")
flow.addConstraint(var - edgeFlow == 0.5)
flow.addConstraint(var == 1.5)
checkSolutions(flow, [[
	(inFlow, A, 1),
	(outFlow, B, 1),
	(edgeFlow, dg.findEdge([A], [B]), 1),
	(hyperflow.VarSumCustom(var.id), var.name, 1.5),
]], maxNumSolutions=100)

makeFlow("A -> B")
flow.addSource(A)
flow.addSink(B)
var = flow.addFloatVariable("floatVarName")
flow.addConstraint(var - edgeFlow == 2.0)
flow.addConstraint(var == 3.0)
checkSolutions(flow, [[
	(inFlow, A, 1),
	(outFlow, B, 1),
	(edgeFlow, dg.findEdge([A], [B]), 1),
	(hyperflow.VarSumCustom(var.id), var.name, 3.0),
]], maxNumSolutions=100)
