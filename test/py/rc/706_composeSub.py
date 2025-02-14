include("7xx_expr_common.py")

print("Explicit -------------------------------------------------------------")
handleExp(RCExpComposeSub(rcExp(ketoEnol_B), rcExp(aldolAdd_B), True))
print("Semi-implicit --------------------------------------------------------")
handleExp(rcExp(ketoEnol_B) * rcSub * rcExp(aldolAdd_B))
print("Implicit -------------------------------------------------------------")
handleExp(ketoEnol_B * rcSub() * aldolAdd_B)
print("Implicit 2 -----------------------------------------------------------")
handleExp(ketoEnol_B * rcSub * aldolAdd_B)

expr = r1 *rcSub(allowPartial=False)* r2
s = f"{r1} *rcSub(allowPartial=false)* {r2}"
assert str(expr) == s, (str(expr), s)

expr = r1 *rcSub(allowPartial=True)* r2
s = f"{r1} *rcSub(allowPartial=true)* {r2}"
assert str(expr) == s, (str(expr), s)

expr = r1 *rcSub* r2
assert str(expr) == s, (str(expr), s)

expr = r1 *rcSub()* r2
assert str(expr) == s, (str(expr), s)

for partial in True, False:
	sPartial = str(partial).lower()
	s = f"{r1} *rcSub(allowPartial={sPartial})* {r2}"
	expr = r1 *rcSub(allowPartial=partial)* r2
	assert str(expr) == s, (str(expr), s)

	if (partial,) == (True,):
		expr = r1 *rcSub* r2
		assert str(expr) == s, (str(expr), s)
		expr = r1 *rcSub()* r2
		assert str(expr) == s, (str(expr), s)
