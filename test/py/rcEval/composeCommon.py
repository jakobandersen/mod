include("common.py")

print("Explicit -------------------------------------------------------------")
handleExp(RCExpComposeCommon(rcExp(ketoEnol_F), rcExp(aldolAdd_F), True, True, False))
print("Semi-implicit --------------------------------------------------------")
handleExp(rcExp(ketoEnol_F) * rcCommon * rcExp(aldolAdd_F))
print("Implicit -------------------------------------------------------------")
handleExp(ketoEnol_F * rcCommon() * aldolAdd_F)
print("Implicit 2 -----------------------------------------------------------")
handleExp(ketoEnol_F * rcCommon * aldolAdd_F)

post("disableSummary")
