include("../xxx_helpers.py")

msg = "Ignoring stereo information in SMILES. Can not add tetrahedral geometry to vertex ([C@]) with degree 5. Must be 4."

a = smiles("[C@](O)(O)(O)(O)(O)")
lws = a.loadingWarnings
assert len(lws) == 1, lws
assert lws[0] == (msg, True)

a = smiles("[C@](O)(O)(O)(O)(O)", printStereoWarnings=False)
lws = a.loadingWarnings
assert len(lws) == 1, lws
assert lws[0] == (msg, False)

b = a.makePermutation()
fail(lambda: b.loadingWarnings, "Can not get loading warnings. No data from external loading stored.")
