include("7xx_expr_common.py")

print("Explicit -------------------------------------------------------------")
handleExp(RCExpComposeParallel(rcExp(ketoEnol_F), rcExp(aldolAdd_F)))
print("Semi-implicit --------------------------------------------------------")
handleExp(rcExp(ketoEnol_F) *rcParallel* rcExp(aldolAdd_F))
print("Implicit -------------------------------------------------------------")
handleExp(ketoEnol_F *rcParallel*  aldolAdd_F)

expr = r1 *rcParallel* r2
assert str(expr) == f"{r1} *rcParallel* {r2}"
