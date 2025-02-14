include("7xx_expr_common.py")

fail(lambda: RCExpUnion(rcExp([])), "An RC Union expression must be instantiated with a non-empty list of subexpressions.")


print("Explicit --------------------------------------------------------------------")
handleExp(rcExp((ketoEnol_F, ketoEnol_B)))

print("Implicit --------------------------------------------------------------------")
handleExp((ketoEnol_F, ketoEnol_B))

print("Input rules -----------------------------------------------------------------")
handleExp(inputRules)
