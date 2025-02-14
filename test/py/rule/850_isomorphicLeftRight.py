include("xxx_helpers.py")

rId = Rule.fromDFS("[A]0.[B]1>>[A]0.[B]1", "ID")
rSwap = Rule.fromDFS("[A]0.[B]1>>[A]1.[B]0", "swap")
assert rId.isomorphism(rSwap) == 0
assert rId.isomorphicLeftRight(rSwap)

commonChecks(rId)
commonChecks(rSwap)


rId = Rule.fromDFS("[A]0-[A]1.[A]10-[A]11>>[A]0-[A]1.[A]10-[A]11", "ID")
rSwap = Rule.fromDFS("[A]0-[A]1.[A]10-[A]11>>[A]0-[A]10.[A]1-[A]11", "swap")
assert rId.isomorphism(rSwap) == 0
assert rId.isomorphicLeftRight(rSwap)

commonChecks(rId)
commonChecks(rSwap)
