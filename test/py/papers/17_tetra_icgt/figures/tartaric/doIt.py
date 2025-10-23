include("../stereoIsoCommon.py")

Graph.fromSMILES("C(C(C(=O)O)O)(C(=O)O)O", name="Tartaric acid")
Graph.fromSMILES("[C@@H]([C@H](C(=O)O)O)(C(=O)O)O", name="L-tartaric acid")
Graph.fromSMILES("[C@H]([C@@H](C(=O)O)O)(C(=O)O)O", name="D-tartaric acid")
Graph.fromSMILES("[C@@H]([C@@H](C(=O)O)O)(C(=O)O)O", name="Meso-tartaric acid")
dg = DG(graphDatabase=inputGraphs,
	labelSettings=LabelSettings(LabelType.Term, LabelRelation.Specialisation, LabelRelation.Specialisation))
dg.build().execute(addSubset(inputGraphs) >> repeat(inputRules))
p = DGPrinter()
p.withRuleName = True
p.withRuleId = False
dg.print(p)
