include("xx1_var_spec.py")

def checkIsOverallAutocata(s):
	assert not s.model.relaxed
	for v in s.model.dg.vertices:
		i = s.eval(isOverallAutocata[v])
		fi = s.eval(inFlow[v])
		fo = s.eval(outFlow[v])
		assert (i == 1) == (0 < fi and fi < fo)
registerVar("overallAutocatalysis", "vertex", "isOverallAutocata", False, checkIsOverallAutocata)
