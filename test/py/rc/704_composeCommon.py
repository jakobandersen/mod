include("7xx_expr_common.py")

print("Explicit -------------------------------------------------------------")
handleExp(RCExpComposeCommon(rcExp(ketoEnol_F), rcExp(aldolAdd_F), True, False, False))
print("Semi-implicit --------------------------------------------------------")
handleExp(rcExp(ketoEnol_F) * rcCommon * rcExp(aldolAdd_F))
print("Implicit -------------------------------------------------------------")
handleExp(ketoEnol_F * rcCommon() * aldolAdd_F)
print("Implicit 2 -----------------------------------------------------------")
handleExp(ketoEnol_F * rcCommon * aldolAdd_F)

post.disableInvokeMake()

rc = RCEvaluator(inputRules)
resWithout = rc.eval(ketoEnol_F *rcCommon* ketoEnol_B)
resWith = rc.eval(ketoEnol_F *rcCommon(includeEmpty=True)* ketoEnol_B)
print("|resWithout|:", len(resWithout))
print("|resWith|:", len(resWith))
assert len(resWithout) + 1 == len(resWith)

for maximum in True, False:
	sMax = str(maximum).lower()
	for connected in True, False:
		sConn = str(connected).lower()
		for includeEmpty in True, False:
			sEmpty = str(includeEmpty).lower()
			s = f"{r1} *rcCommon(maximum={sMax}, connected={sConn}, includeEmpty={sEmpty})* {r2}"
			expr = r1 *rcCommon(maximum=maximum, connected=connected, includeEmpty=includeEmpty)* r2
			assert str(expr) == s, (str(expr), s)

			if (maximum, connected, includeEmpty) == (False, True, False):
				expr = r1 *rcCommon* r2
				assert str(expr) == s, (str(expr), s)
				expr = r1 *rcCommon()* r2
				assert str(expr) == s, (str(expr), s)
