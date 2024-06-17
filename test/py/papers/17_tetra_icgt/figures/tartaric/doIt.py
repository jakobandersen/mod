include("../stereoIsoCommon.py")

smiles("C(C(C(=O)O)O)(C(=O)O)O", name="Tartaric acid")
smiles("[C@@H]([C@H](C(=O)O)O)(C(=O)O)O", name="L-tartaric acid")
smiles("[C@H]([C@@H](C(=O)O)O)(C(=O)O)O", name="D-tartaric acid")
smiles("[C@@H]([C@@H](C(=O)O)O)(C(=O)O)O", name="Meso-tartaric acid")
dg = DG(graphDatabase=inputGraphs,
	labelSettings=LabelSettings(LabelType.Term, LabelRelation.Specialisation, LabelRelation.Specialisation))
dg.build().execute(addSubset(inputGraphs) >> repeat(inputRules))
p = DGPrinter()
p.withRuleName = True
p.withRuleId = False
dg.print(p)
