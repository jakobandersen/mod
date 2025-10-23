AcP = Graph.fromSMILES("OP(O)(=O)OC(=O)C", "AcP") # Acetyl-Phosphate
G3P = Graph.fromSMILES("C(C(C=O)O)OP(=O)(O)O", "G3P") # Glyceraldehyde 3-phosphate
E4P = Graph.fromSMILES("OP(O)(=O)OCC(O)C(O)C=O", "E4P") # Erythrose-4-phosphate
R5P = Graph.fromSMILES("OP(O)(=O)OCC(O)C(O)C(O)C=O", "R5P") # Ribose 5-phosphate
Ru5P = Graph.fromSMILES("OCC(=O)C(O)C(O)COP(=O)(O)O", "Ru5P") # Ribulose-5-Phosphate
ribuloseP = Ru5P # TODO: remove at some point
F6P = Graph.fromSMILES("OCC(=O)C(O)C(O)C(O)COP(=O)(O)O", "F6P") # Fructose-6-Phosphate
fructoseP = F6P # TODO: remove at some point
S7P = Graph.fromSMILES("O=P(O)(OCC(O)C(O)C(O)C(O)C(=O)CO)O", "S7P") # Sedoheptulose 7-phosphate
water = Graph.fromSMILES("O", "Water")
Pi = Graph.fromSMILES("O=P(O)(O)O", "Pi")
phosphate = Pi # TODO: remove at some point

aldoKetoF = Rule.fromGMLFile("aldo_keto_backward.gml")
aldoKetoB = Rule.fromGMLFile("aldo_keto_forward.gml")
transKeto = Rule.fromGMLFile("transketolase.gml")
Tkt = transKeto
transAldo = Rule.fromGMLFile("transaldolase.gml")
Tal = transAldo
aldolase = Rule.fromGMLFile("aldolase.gml")
phosphohydro = Rule.fromGMLFile("phosphohydrolase.gml")

aldolaseReal = leftPredicate[lambda d: any(a.vLabelCount("C") > 2 for a in d.left)](aldolase)

rulesGeneric = [aldoKetoF, aldoKetoB, transKeto, transAldo, aldolase, phosphohydro]
rulesReal = [aldoKetoF, aldoKetoB, transKeto, transAldo, aldolaseReal, phosphohydro]
