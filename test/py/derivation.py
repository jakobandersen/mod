post.disableInvokeMake()
g1 = smiles('O', name="g1")
g2 = smiles('C', name="g2")
r = ruleGMLString("""rule [ ruleID "r" context [ node [ id 0 label "O" ] ] ]""")

##############################################################################
d = Derivation()
assert str(d) == "{ }, { }"
assert repr(d) == str(d)
d.left = [g1]
assert d.left == [g1]
assert str(d) == "{ 'g1' }, { }"

d.rule = r
assert d.rule == r
assert str(d) == "{ 'g1' }, 'r', { }"
d.right = [g2]
assert d.right == [g2]
assert str(d) == "{ 'g1' }, 'r', { 'g2' }"

d = Derivation()
d.left = [None]
assert str(d) == "{ null }, { }"
d.right = [None]
assert str(d) == "{ null }, { null }"

##############################################################################
d = Derivation()
assert str(d) == "{ }, { }"
assert repr(d) == str(d)
d.left.append(g1)
assert d.left == [g1]
assert str(d) == "{ 'g1' }, { }"

d.rule = r
assert d.rule == r
assert str(d) == "{ 'g1' }, 'r', { }"
d.right.append(g2)
assert d.right == [g2]
assert str(d) == "{ 'g1' }, 'r', { 'g2' }"

d = Derivation()
d.left.append(None)
assert str(d) == "{ null }, { }"
d.right.append(None)
assert str(d) == "{ null }, { null }"

##############################################################################
d = Derivations()
assert str(d) == "{ } < > { }"
assert repr(d) == str(d)
d.left = [g1]
assert d.left == [g1]
assert str(d) == "{ 'g1' } < > { }"

d.rules = [r]
assert d.rules == [r]
assert str(d) == "{ 'g1' } < 'r' > { }"
d.right = [g2]
assert d.right == [g2]
assert str(d) == "{ 'g1' } < 'r' > { 'g2' }"

d = Derivations()
d.left = [None]
assert str(d) == "{ null } < > { }"
d.rules = [None]
assert str(d) == "{ null } < null > { }"
d.right = [None]
assert str(d) == "{ null } < null > { null }"

##############################################################################
d = Derivations()
assert str(d) == "{ } < > { }"
assert repr(d) == str(d)
d.left.append(g1)
assert d.left == [g1]
assert str(d) == "{ 'g1' } < > { }"

d.rules.append(r)
assert d.rules == [r]
assert str(d) == "{ 'g1' } < 'r' > { }"
d.right.append(g2)
assert d.right == [g2]
assert str(d) == "{ 'g1' } < 'r' > { 'g2' }"

d = Derivations()
d.left.append(None)
assert str(d) == "{ null } < > { }"
d.rules.append(None)
assert str(d) == "{ null } < null > { }"
d.right.append(None)
assert str(d) == "{ null } < null > { null }"

##############################################################################
d = Derivation()
d.left = [g1]
d.rule = r
d.right = [g2]

dd = Derivations(d)
assert str(d) == "{ 'g1' }, 'r', { 'g2' }"
assert str(dd) == "{ 'g1' } < 'r' > { 'g2' }"

##############################################################################
d = Derivation()
d.left.append(g1)
d.rule = r
d.right.append(g2)

dd = Derivations(d)
assert str(d) == "{ 'g1' }, 'r', { 'g2' }"
assert str(dd) == "{ 'g1' } < 'r' > { 'g2' }"
