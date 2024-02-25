include("../000_basics/050_formoseGrammar.py")
rc = rcEvaluator(inputRules)
exp = (
	rcId(glycolaldehyde)
	*rcSuper* ketoEnol_F
	*rcParallel* rcId(formaldehyde)
	*rcSuper(allowPartial=False)* aldolAdd_F
	*rcSuper* ketoEnol_F
	*rcParallel* rcId(formaldehyde)
	*rcSuper(allowPartial=False)* aldolAdd_F
	*rcSuper* ketoEnol_F
	*rcSuper* ketoEnol_B
	*rcSuper* aldolAdd_B
	*rcSuper* ketoEnol_B
	*rcSuper(allowPartial=False)*
	(rcId(glycolaldehyde) *rcParallel* rcId(glycolaldehyde))
)
rules = rc.eval(exp)
for p in rules:
	p.print()
# rst-name: Overall Formose Reaction
# rst: A complete pathway can be composed to obtain the overall rules.
