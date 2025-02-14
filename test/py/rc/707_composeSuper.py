include("7xx_expr_common.py")

print("Explicit -------------------------------------------------------------")
handleExp(RCExpComposeSuper(rcExp(aldolAdd_B), rcExp(ketoEnol_B), True, False))
print("Semi-implicit --------------------------------------------------------")
handleExp(rcExp(aldolAdd_B) * rcSuper * rcExp(ketoEnol_B))
print("Implicit -------------------------------------------------------------")
handleExp(aldolAdd_B * rcSuper() * ketoEnol_B)
print("Implicit 2 -----------------------------------------------------------")
handleExp(aldolAdd_B * rcSuper * ketoEnol_B)

for partial in True, False:
	sPartial = str(partial).lower()
	for constraints in True, False:
		sCon = str(constraints).lower()
		s = f"{r1} *rcSuper(allowPartial={sPartial}, enforceConstraints={sCon})* {r2}"
		expr = r1 *rcSuper(allowPartial=partial, enforceConstraints=constraints)* r2
		assert str(expr) == s, (str(expr), s)

		if (partial, constraints) == (True, False):
			expr = r1 *rcSuper* r2
			assert str(expr) == s, (str(expr), s)
			expr = r1 *rcSuper()* r2
			assert str(expr) == s, (str(expr), s)
