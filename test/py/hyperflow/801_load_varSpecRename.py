include("xxx_common.py")

dg = DG()
dg.build().addAbstract("A -> B")
flow = hyperflow.Model(dg)
for v in dg.vertices:
	flow.separateIOInternalTransit(v)
flow.objectiveFunction = vertexFlow + transitInternalFlow + edgeFlow
flow.findSolutions()
#flow.dump()

dumpName = "801_load_varSpecRename.flow"
showDump(dumpName)
flow2 = hyperflow.Model.load(dg, dumpName)

mod.libpymod._compareModels_only_for_testing(flow, flow2)
