include("xxx_helpers.py")

print("AtomId")
print("=" * 80)
assert AtomId() == AtomId(0)
fail(lambda: AtomId().symbol, "AtomId::Invalid has no symbol.")
assert int(AtomId()) == 0
assert str(AtomId()) == str(int(AtomId()))

assert AtomId(1) == AtomIds.H
assert AtomId(1) == AtomIds.Hydrogen
assert AtomId(1).symbol == "H"
assert int(AtomId(1)) == 1
assert str(AtomId(1)) == str(int(AtomId(1)))


print("Isotope")
print("=" * 80)
assert Isotope() == Isotope(-1)
assert int(Isotope()) == -1
assert str(Isotope()) == str(int(Isotope()))
assert Isotope() == -1

assert int(Isotope(42)) == 42
assert str(Isotope(42)) == str(int(Isotope(42)))
assert Isotope(42) == 42


print("Charge")
print("=" * 80)
assert Charge() == Charge(0)
assert int(Charge()) == 0
assert str(Charge()) == str(int(Charge()))
assert Charge() == 0

assert int(Charge(2)) == 2
assert str(Charge(2)) == str(int(Charge(2)))
assert Charge(2) == 2


print("AtomData")
print("=" * 80)
assert AtomData().atomId == AtomId()
assert AtomData().isotope == Isotope()
assert AtomData().charge == Charge()
assert AtomData().radical == False

a = AtomData(AtomId(42), Isotope(60), Charge(-9), True)
assert a.atomId == AtomId(42)
assert a.isotope == 60
assert a.charge == -9
assert a.radical == True

b = AtomData(AtomId(42), Isotope(60), Charge(-9), True)
assert a == b

g = Graph.fromDFS("[60Mo9-.]")
v = next(iter(g.vertices))
assert v.atomId == a.atomId
assert v.isotope == a.isotope
assert v.charge == a.charge
assert v.radical == a.radical

assert AtomData() < AtomData(AtomId(1))
assert AtomData(AtomId(1)) > AtomData()


print("BondType")
print("=" * 80)
fail(lambda: str(BondType.Invalid), "Can not print BondType::Invalid.")
assert int(BondType.Single) == 1
assert int(BondType.Aromatic) == 2
assert int(BondType.Double) == 3
assert int(BondType.Triple) == 4

assert str(BondType.Single) == "-"
assert str(BondType.Aromatic) == ":"
assert str(BondType.Double) == "="
assert str(BondType.Triple) == "#"


print("AtomIds")
print("=" * 80)
assert AtomIds.Invalid == AtomId()
