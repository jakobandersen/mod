include("7xx_expr_common.py")

fail(lambda: RCEvaluator([]).eval(None), "The rule is null.")


print("Explicit -----------------------------------------------------------------")
handleExp(rcExp(ketoEnol_F))

print("Implicit -----------------------------------------------------------------")
handleExp(ketoEnol_F)
