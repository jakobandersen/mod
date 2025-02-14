include("../xxx_helpers.py")

# Rules
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# Let ABC = {A, B, C}
# Let ABCe = {A, B, C, <no edge>}
# ==== ==== ==================================
# L    R    Label variations                  
# ==== ==== ==================================
# (empty)                                     
# ---- ---- ----------------------------------
# O         ABC                               
#      O    ABC                               
# O    O    ABC                               
# $    $    ABC x ABC                         
# ---- ---- ----------------------------------
# O-O       ABC x ABC x ABC                   
# O-O' O    ABC x ABC x ABC                   
# O-$  $    ABC x ABC x ABC x ABC             
#      O-O  ABC x ABC x ABC                   
# O    O-O' ABC x ABC x ABC                   
# $    O-$  ABC x ABC x ABC x ABC             
# ---- ---- ----------------------------------
# O-O' O-O' ABC x ABC x ABCe x ABC            
# O-$  O-$  ABC x ABC x ABC x ABCe x ABC      
# $-$' $-$' ABC x ABC x ABC x ABC x ABCe x ABC
# ==== ==== ==================================
# Different basic rules:
# L   R
# === ===
# (empty)
# A
#     A
# A   A
# A   B
# AEX
# AEX A
# AEX A X
#     AEX
# A   AEX
# A X AEX
# AEX AEX
# AEX AFX


def _check_0_vertexOverlap(c):
	print()
	print("0-vertex overlap")
	print("#" * 80)
	# Try all parallel compositions between all types of rules.
	# L   R
	# === ===
	# (empty)
	# A
	#     A
	# A   A
	# A   B
	rs1 = (">>", "[A]1>>", ">>[A]1", "[A]1>>[A]1", "[A]1>>[B]1",
	# AEX
	# AEX A
	# AEX A X
		"[A]1{E}[X]2>>",
		"[A]1{E}[X]2>>[A]1",
		"[A]1{E}[X]2>>[A]1.[X]2",
	#     AEX
	# A   AEX
	# A X AEX
		"         >>[A]1{E}[X]2",
		"[A]1     >>[A]1{E}[X]2",
		"[A]1.[X]2>>[A]1{E}[X]2",
	# AEX AEX
	# AEX AFX
		"[A]1{E}[X]2>>[A]1{E}[X]2",
		"[A]1{E}[X]2>>[A]1{F}[X]2")
	# and another copy, with different IDs so we can construct a combined res
	rs2 = (">>", "[A]3>>", ">>[A]3", "[A]3>>[A]3", "[A]3>>[B]3",
		"[A]3{E}[X]4>>",
		"[A]3{E}[X]4>>[A]3",
		"[A]3{E}[X]4>>[A]3.[X]4",
		"           >>[A]3{E}[X]4",
		"[A]3       >>[A]3{E}[X]4",
		"[A]3.  [X]4>>[A]3{E}[X]4",
		"[A]3{E}[X]4>>[A]3{E}[X]4",
		"[A]3{E}[X]4>>[A]3{F}[X]4")

	for r1 in rs1:
		for r2 in rs2:
			r2L, r2R = r2.split(">>")
			res = r1
			if res.lstrip()[0] == ">":
				res = r2L + res
			elif r2L.strip() != "":
				res = r2L + "." + res
			if res.rstrip()[-1] == ">":
				res += r2R
			elif r2R.strip() != "":
				res += "." + r2R
			c(r1, r2, {}, res)


def _check_1_vertexOverlap(c):
	print()
	print("1-vertex overlap")
	print("#" * 80)
	o = {1: 1}

	# ---------------------------------------------------
	# L   R   L   R    L   R    Note
	# === === === ===  === ===  =========================
	# (empty)                   no overlaps
	# A                         no overlaps

	# ---------------------------------------------------
	# L   R   L   R    L   R    Note
	# === === === ===  === ===  =========================
	print("   >>A   with everything")
	print("-" * 80)
	r1 = ">>[A]1"
	#     A   (empty)           no overlaps
	#     A   A         (empty)
	c(r1, "[A]1>>", o, ">>")
	#     A       A             no overlaps
	#     A   A   A        A
	#     A   A   B        B
	c(r1, "[A]1>>[A]1", o, ">>[A]")
	c(r1, "[A]1>>[B]1", o, ">>[B]")
	#     A   AEX               dangling condition
	#     A   AEX A             dangling condition
	#     A   AEX A X           dangling condition
	c(r1, "[A]1{E}[X]2>>",          o, None)
	c(r1, "[A]1{E}[X]2>>[A]1",      o, None)
	c(r1, "[A]1{E}[X]2>>[A]1.[X]2", o, None)
	#     A       AEX           no overlaps
	#     A   A   AEX      AEX
	#     A   A X AEX    X AEX
	c(r1, "[A]1     >>[A]1{E}[X]2", o, "    >>[A]{E}[X]")
	c(r1, "[A]1.[X]2>>[A]1{E}[X]2", o, "[X]2>>[A]{E}[X]2")
	#     A   AEX AEX           dangling condition
	#     A   AEX AFX           dangling condition
	c(r1, "[A]1{E}[X]2>>[A]1{E}[X]2", o, None)
	c(r1, "[A]1{E}[X]2>>[A]1{F}[X]2", o, None)

	# ---------------------------------------------------
	# L   R   L   R    L   R    Note
	# === === === ===  === ===  =========================
	print("A  >>A   with everything")
	print("-" * 80)
	r1 = "[A]1>>[A]1"
	# A   A   (empty)           no overlaps
	# A   A   A        A
	c(r1, "[A]1>>", o, "[A]1>>")
	# A   A       A             no overlaps
	# A   A   A   A    A   A
	# A   A   A   B    A   B
	c(r1, "[A]1>>[A]1", o, "[A]1>>[A]1")
	c(r1, "[A]1>>[B]1", o, "[A]1>>[B]1")
	# A   A   AEX      AEX
	# A   A   AEX A    AEX A
	# A   A   AEX A X  AEX A X
	c(r1, "[A]1{E}[X]2>>",          o, "[A]1{E}[X]2>>")
	c(r1, "[A]1{E}[X]2>>[A]1",      o, "[A]1{E}[X]2>>[A]1")
	c(r1, "[A]1{E}[X]2>>[A]1.[X]2", o, "[A]1{E}[X]2>>[A]1.[X]2")
	# A   A       AEX           no overlaps
	# A   A   A   AEX  A   AEX
	# A   A   A X AEX  A X AEX
	c(r1, "[A]1     >>[A]1{E}[X]2", o, "[A]1     >>[A]1{E}[X]")
	c(r1, "[A]1.[X]2>>[A]1{E}[X]2", o, "[A]1.[X]2>>[A]1{E}[X]2")
	# A   A   AEX AEX  AEX AEX
	# A   A   AEX AFX  AEX AFX
	c(r1, "[A]1{E}[X]2>>[A]1{E}[X]2", o, "[A]1{E}[X]2>>[A]1{E}[X]2")
	c(r1, "[A]1{E}[X]2>>[A]1{F}[X]2", o, "[A]1{E}[X]2>>[A]1{F}[X]2")

	# ---------------------------------------------------
	# L   R   L   R    L   R    Note
	# === === === ===  === ===  =========================
	print("Q  >>A   with everything")
	print("-" * 80)
	r1 = "[Q]1>>[A]1"
	# Q   A   (empty)           no overlaps
	# Q   A   A        Q
	c(r1, "[A]1>>", o, "[Q]1>>")
	# Q   A       A             no overlaps
	# Q   A   A   A    Q   A
	# Q   A   A   B    Q   B
	c(r1, "[A]1>>[A]1", o, "[Q]1>>[A]1")
	c(r1, "[A]1>>[B]1", o, "[Q]1>>[B]1")
	# Q   A   AEX      QEX
	# Q   A   AEX A    QEX A
	# Q   A   AEX A X  QEX A X
	c(r1, "[A]1{E}[X]2>>",          o, "[Q]1{E}[X] >>")
	c(r1, "[A]1{E}[X]2>>[A]1",      o, "[Q]1{E}[X] >>[A]1")
	c(r1, "[A]1{E}[X]2>>[A]1.[X]2", o, "[Q]1{E}[X]2>>[A]1.[X]2")
	# Q   A       AEX           no overlaps
	# Q   A   A   AEX  Q   AEX
	# Q   A   A X AEX  Q X AEX
	c(r1, "[A]1     >>[A]1{E}[X]2", o, "[Q]1     >>[A]1{E}[X]")
	c(r1, "[A]1.[X]2>>[A]1{E}[X]2", o, "[Q]1.[X]2>>[A]1{E}[X]2")
	# Q   A   AEX AEX  QEX AEX
	# Q   A   AEX AFX  QEX AFX
	c(r1, "[A]1{E}[X]2>>[A]1{E}[X]2", o, "[Q]1{E}[X]2>>[A]1{E}[X]2")
	c(r1, "[A]1{E}[X]2>>[A]1{F}[X]2", o, "[Q]1{E}[X]2>>[A]1{F}[X]2")

	# AEX                       no overlaps

	# ---------------------------------------------------
	# L   R   L   R    L   R    Note
	# === === === ===  === ===  =========================
	print("AEX>>A   with everything")
	print("-" * 80)
	r1 = "[A]1{E}[X]>>[A]1"
	# AEX A   (empty)           no overlaps
	# AEX A   A        AEX
	c(r1, "[A]1>>", o, "[A]{E}[X]>>")
	# AEX A       A             no overlaps
	# AEX A   A   A    AEX A
	# AEX A   A   B    AEX B
	c(r1, "[A]1>>[A]1", o, "[A]1{E}[X]>>[A]1")
	c(r1, "[A]1>>[B]1", o, "[A]1{E}[X]>>[B]1")
	# AEX A   AEY      A(EY)EX
	# AEX A   AEY A    A(EY)EX A
	# AEX A   AEY A Y  A(EY)EX A Y
	c(r1, "[A]1{E}[Y]2>>",          o, "[A]1({E}[Y] ){E}[X]>>")
	c(r1, "[A]1{E}[Y]2>>[A]1",      o, "[A]1({E}[Y] ){E}[X]>>[A]1")
	c(r1, "[A]1{E}[Y]2>>[A]1.[Y]2", o, "[A]1({E}[Y]2){E}[X]>>[A]1.[Y]2")
	# AEX A       AEY           no overlaps
	# AEX A   A   AEY  AEX     AEY
	# AEX A   A Y AEY  A( Y)EX AEY
	c(r1, "[A]1     >>[A]1{E}[Y]2", o, "[A]1       {E}[X]>>[A]1{E}[Y]")
	c(r1, "[A]1.[Y]2>>[A]1{E}[Y]2", o, "[A]1(.[Y]2){E}[X]>>[A]1{E}[Y]2")
	# AEX A   AEY AEY  A(EY)EX AEY
	# AEX A   AEY AFY  A(EY)EX AFY
	c(r1, "[A]1{E}[Y]2>>[A]1{E}[Y]2", o, "[A]1({E}[Y]2){E}[X]>>[A]1{E}[Y]2")
	c(r1, "[A]1{E}[Y]2>>[A]1{F}[Y]2", o, "[A]1({E}[Y]2){E}[X]>>[A]1{F}[Y]2")

	# ---------------------------------------------------
	# L   R   L   R    L   R    Note
	# === === === ===  === ===  =========================
	print("AEX>>A X with everything")
	print("-" * 80)
	r1 = "[A]1{E}[X]2>>[A]1.[X]2"
	# AEX A X (empty)           no overlaps
	# AEX A X A        AEX   X
	c(r1, "[A]1>>", o, "[A]{E}[X]2>>[X]2")
	# AEX A X     A             no overlaps
	# AEX A X A   A    AEX A X
	# AEX A X A   B    AEX B X
	c(r1, "[A]1>>[A]1", o, "[A]1{E}[X]2>>[A]1.[X]2")
	c(r1, "[A]1>>[B]1", o, "[A]1{E}[X]2>>[B]1.[X]2")
	# AEX A X AEY      A(EY)EX   X
	# AEX A X AEY A    A(EY)EX A X
	# AEX A X AEY A Y  A(EY)EX A Y X
	c(r1, "[A]1{E}[Y]2>>",          o, "[A]1({E}[Y] ){E}[X]3>>          [X]3")
	c(r1, "[A]1{E}[Y]2>>[A]1",      o, "[A]1({E}[Y] ){E}[X]3>>[A]1     .[X]3")
	c(r1, "[A]1{E}[Y]2>>[A]1.[Y]2", o, "[A]1({E}[Y]2){E}[X]3>>[A]1.[Y]2.[X]3")
	# AEX A X     AEY           no overlaps
	# AEX A X A   AEY  AEX     AEY   X
	# AEX A X A Y AEY  A( Y)EX AEY   X
	c(r1, "[A]1     >>[A]1{E}[Y]2", o, "[A]1       {E}[X]3>>[A]1{E}[Y] .[X]3")
	c(r1, "[A]1.[Y]2>>[A]1{E}[Y]2", o, "[A]1(.[Y]2){E}[X]3>>[A]1{E}[Y]2.[X]3")
	# AEX A X AEY AEY  A(EY)EX AEY   X
	# AEX A X AEY AFY  A(EY)EX AFY   X
	c(r1, "[A]1{E}[Y]2>>[A]1{E}[Y]2", o, "[A]1({E}[Y]2){E}[X]3>>[A]1{E}[Y]2.[X]3")
	c(r1, "[A]1{E}[Y]2>>[A]1{F}[Y]2", o, "[A]1({E}[Y]2){E}[X]3>>[A]1{F}[Y]2.[X]3")

	# ---------------------------------------------------
	# L   R   L   R    L   R    Note
	# === === === ===  === ===  =========================
	print("   >>AEX with everything")
	print("-" * 80)
	r1 = ">>[A]1{E}[X]2"
	#     AEX (empty)           no overlaps
	#     AEX A                 dangling condition
	c(r1, "[A]1>>", o, None)
	#     AEX     A             no overlaps
	#     AEX A   A        AEX
	#     AEX A   B        BEX
	c(r1, "[A]1>>[A]1", o, ">>[A]{E}[X]")
	c(r1, "[A]1>>[B]1", o, ">>[B]{E}[X]")
	#     AEX AEY               dangling condition
	#     AEX AEY A             dangling condition
	#     AEX AEY A Y           dangling conditoin
	c(r1, "[A]1{E}[Y]2>>",          o, None)
	c(r1, "[A]1{E}[Y]2>>[A]1",      o, None)
	c(r1, "[A]1{E}[Y]2>>[A]1.[Y]2", o, None)
	#     AEX     AEY           no overlaps
	#     AEX A   AEY      A(EY)EX
	#     AEX A Y AEY    Y A(EY)EX
	c(r1, "[A]1     >>[A]1{E}[Y]2", o, "    >>[A]({E}[Y] ){E}[X]")
	c(r1, "[A]1.[Y]2>>[A]1{E}[Y]2", o, "[Y]2>>[A]({E}[Y]2){E}[X]")
	#     AEX AEY AEY           dangling condition
	#     AEX AEY AFY           dangling condition
	c(r1, "[A]1{E}[Y]2>>[A]1{E}[Y]2", o, None)
	c(r1, "[A]1{E}[Y]2>>[A]1{F}[Y]2", o, None)

	# ---------------------------------------------------
	# L   R   L   R    L   R    Note
	# === === === ===  === ===  =========================
	print("A  >>AEX with everything")
	print("-" * 80)
	r1 = "[A]1>>[A]1{E}[X]2"
	# A   AEX (empty)           no overlaps
	# A   AEX A                 dangling condition
	c(r1, "[A]1>>", o, None)
	# A   AEX     A             no overlaps
	# A   AEX A   A    A   AEX
	# A   AEX A   B    A   BEX
	c(r1, "[A]1>>[A]1", o, "[A]1>>[A]1{E}[X]")
	c(r1, "[A]1>>[B]1", o, "[A]1>>[B]1{E}[X]")
	# A   AEX AEY               dangling condition
	# A   AEX AEY A    AEY AEX
	# A   AEX AEY A Y  AEY A( Y)EX
	c(r1, "[A]1{E}[Y]2>>",          o, None)
	c(r1, "[A]1{E}[Y]2>>[A]1",      o, "[A]1{E}[Y] >>[A]1       {E}[X]")
	c(r1, "[A]1{E}[Y]2>>[A]1.[Y]2", o, "[A]1{E}[Y]2>>[A]1(.[Y]2){E}[X]")
	# A   AEX     AEY           no overlaps
	# A   AEX A   AEY  A   A(EY)EX
	# A   AEX A Y AEY  A Y A(EY)EX
	c(r1, "[A]1     >>[A]1{E}[Y]2", o, "[A]1     >>[A]1({E}[Y] ){E}[X]")
	c(r1, "[A]1.[Y]2>>[A]1{E}[Y]2", o, "[A]1.[Y]2>>[A]1({E}[Y]2){E}[X]")
	# A   AEX AEY AEY  AEY A(EY)EX
	# A   AEX AEY AFY  AEY A(FY)EX
	c(r1, "[A]1{E}[Y]2>>[A]1{E}[Y]2", o, "[A]1{E}[Y]2>>[A]1({E}[Y]2){E}[X]")
	c(r1, "[A]1{E}[Y]2>>[A]1{F}[Y]2", o, "[A]1{E}[Y]2>>[A]1({F}[Y]2){E}[X]")

	# ---------------------------------------------------
	# L   R   L   R    L   R    Note
	# === === === ===  === ===  =========================
	print("A X>>AEX with everything")
	print("-" * 80)
	r1 = "[A]1.[X]2>>[A]1{E}[X]2"
	# A X AEX (empty)           no overlaps
	# A X AEX A                 dangling condition
	c(r1, "[A]1>>", o, None)
	# A X AEX     A             no overlaps
	# A X AEX A   A    A X AEX
	# A X AEX A   B    A X BEX
	c(r1, "[A]1>>[A]1", o, "[A]1.[X]2>>[A]1{E}[X]2")
	c(r1, "[A]1>>[B]1", o, "[A]1.[X]2>>[B]1{E}[X]2")
	# A X AEX AEY               dangling condition
	# A X AEX AEY A    AEY X AEX
	# A X AEX AEY A X  AEY X  A( Y)EX
	c(r1, "[A]1{E}[Y]2>>",          o, None)
	c(r1, "[A]1{E}[Y]2>>[A]1",      o, "[A]1{E}[Y] .[X]3>>[A]1       {E}[X]3")
	c(r1, "[A]1{E}[Y]2>>[A]1.[Y]2", o, "[A]1{E}[Y]2.[X]3>>[A]1(.[Y]2){E}[X]3")
	# A X AEX     AEY           no overlaps
	# A X AEX A   AEY  A X   A(EY)EX
	# A X AEX A Y AEY  A Y X A(EY)EX
	c(r1, "[A]1     >>[A]1{E}[Y]2", o, "[A]1     .[X]3>>[A]1({E}[Y] ){E}[X]3")
	c(r1, "[A]1.[Y]2>>[A]1{E}[Y]2", o, "[A]1.[Y]2.[X]3>>[A]1({E}[Y]2){E}[X]3")
	# A X AEX AEY AEY  AEY X A(EY)EX
	# A X AEX AEY AFY  AEY X A(FY)EX
	c(r1, "[A]1{E}[Y]2>>[A]1{E}[Y]2", o, "[A]1{E}[Y]2.[X]3>>[A]1({E}[Y]2){E}[X]3")
	c(r1, "[A]1{E}[Y]2>>[A]1{F}[Y]2", o, "[A]1{E}[Y]2.[X]3>>[A]1({F}[Y]2){E}[X]3")

	# ---------------------------------------------------
	# L   R   L   R    L   R    Note
	# === === === ===  === ===  =========================
	print("AEX>>AEX with everything")
	print("-" * 80)
	r1 = "[A]1{E}[X]2>>[A]1{E}[X]2"
	# AEX AEX (empty)           no overlaps
	# AEX AEX A                 dangling condition
	c(r1, "[A]1>>", o, None)
	# AEX AEX     A             no overlaps
	# AEX AEX A   A    AEX AEX
	# AEX AEX A   B    AEX BEX
	c(r1, "[A]1>>[A]1", o, "[A]1{E}[X]2>>[A]1{E}[X]2")
	c(r1, "[A]1>>[B]1", o, "[A]1{E}[X]2>>[B]1{E}[X]2")
	# AEX AEX AEY               dangling condition
	# AEX AEX AEY A    A(EY)EX AEX
	# AEX AEX AEY A Y  A(EY)EX A( Y)EX
	c(r1, "[A]1{E}[Y]2>>",          o, None)
	c(r1, "[A]1{E}[Y]2>>[A]1",      o, "[A]1({E}[Y] ){E}[X]3>>[A]1       {E}[X]3")
	c(r1, "[A]1{E}[Y]2>>[A]1.[Y]2", o, "[A]1({E}[Y]2){E}[X]3>>[A]1(.[Y]2){E}[X]3")
	# AEX AEX     AEY           no overlaps
	# AEX AEX A   AEY  AEX     A(EY)EX
	# AEX AEX A Y AEY  A( Y)EX A(EY)EX
	c(r1, "[A]1     >>[A]1{E}[Y]2", o, "[A]1       {E}[X]3>>[A]1({E}[Y] ){E}[X]3")
	c(r1, "[A]1.[Y]2>>[A]1{E}[Y]2", o, "[A]1(.[Y]2){E}[X]3>>[A]1({E}[Y]2){E}[X]3")
	# AEX AEX AEY AEY  A(EY)EX A(EY)EX
	# AEX AEX AEY AFY  A(EY)EX A(FY)EX
	c(r1, "[A]1{E}[Y]2>>[A]1{E}[Y]2", o, "[A]1({E}[Y]2){E}[X]3>>[A]1({E}[Y]2){E}[X]3")
	c(r1, "[A]1{E}[Y]2>>[A]1{F}[Y]2", o, "[A]1({E}[Y]2){E}[X]3>>[A]1({F}[Y]2){E}[X]3")


	# ---------------------------------------------------
	# L   R   L   R    L   R    Note
	# === === === ===  === ===  =========================
	print("AQX>>AEX with everything")
	print("-" * 80)
	r1 = "[A]1{Q}[X]2>>[A]1{E}[X]2"
	# AQX AEX (empty)           no overlaps
	# AQX AEX A                 dangling condition
	c(r1, "[A]1>>", o, None)
	# AQX AEX     A             no overlaps
	# AQX AEX A   A    AQX AEX
	# AQX AEX A   B    AQX BEX
	c(r1, "[A]1>>[A]1", o, "[A]1{Q}[X]2>>[A]1{E}[X]2")
	c(r1, "[A]1>>[B]1", o, "[A]1{Q}[X]2>>[B]1{E}[X]2")
	# AQX AEX AEY               dangling condition
	# AQX AEX AEY A    A(EY)QX AEX
	# AQX AEX AEY A Y  A(EY)QX A( Y)EX
	c(r1, "[A]1{E}[Y]2>>",          o, None)
	c(r1, "[A]1{E}[Y]2>>[A]1",      o, "[A]1({E}[Y] ){Q}[X]3>>[A]1       {E}[X]3")
	c(r1, "[A]1{E}[Y]2>>[A]1.[Y]2", o, "[A]1({E}[Y]2){Q}[X]3>>[A]1(.[Y]2){E}[X]3")
	# AQX AEX     AEY           no overlaps
	# AQX AEX A   AEY  AQX     A(EY)EX
	# AQX AEX A Y AEY  A( Y)QX A(EY)EX
	c(r1, "[A]1     >>[A]1{E}[Y]2", o, "[A]1       {Q}[X]3>>[A]1({E}[Y] ){E}[X]3")
	c(r1, "[A]1.[Y]2>>[A]1{E}[Y]2", o, "[A]1(.[Y]2){Q}[X]3>>[A]1({E}[Y]2){E}[X]3")
	# AQX AEX AEY AEY  A(EY)QX A(EY)EX
	# AQX AEX AEY AFY  A(EY)QX A(FY)EX
	c(r1, "[A]1{E}[Y]2>>[A]1{E}[Y]2", o, "[A]1({E}[Y]2){Q}[X]3>>[A]1({E}[Y]2){E}[X]3")
	c(r1, "[A]1{E}[Y]2>>[A]1{F}[Y]2", o, "[A]1({E}[Y]2){Q}[X]3>>[A]1({F}[Y]2){E}[X]3")


def _check_2_vertexOverlap(c):
	print()
	print("2-vertex overlap")
	print("#" * 80)
	o = {1: 1, 2: 2}

	# ---------------------------------------------------
	# L   R   L   R    L   R    Note
	# === === === ===  === ===  =========================
	# (empty)                   no overlaps
	# A                         no overlaps
	#     A                     no overlaps
	# A   A                     no overlaps
	# A   B                     no overlaps
	# AEX                       no overlaps
	# AEX A                     no overlaps

	# ---------------------------------------------------
	# L   R   L   R    L   R    Note
	# === === === ===  === ===  =========================
	print("AEX>>A X with everything")
	print("-" * 80)
	r1 = "[A]1{E}[X]2>>[A]1.[X]2"
	# AEX A X (empty)           no overlaps
	# AEX A X A                 no overlaps
	# AEX A X     A             no overlaps
	# AEX A X A   A             no overlaps
	# AEX A X A   B             no overlaps
	# AEX A X AEX               parallel edge in L
	# AEX A X AEX A             parallel edge in L
	# AEX A X AEX A X           parallel edge in L
	c(r1, "[A]1{E}[X]2>>",          o, None)
	c(r1, "[A]1{E}[X]2>>[A]1",      o, None)
	c(r1, "[A]1{E}[X]2>>[A]1.[X]2", o, None)
	# AEX A X     AEX           no overlaps
	# AEX A X A   AEX           no overlaps
	# AEX A X A X AEX  AEX AEX
	c(r1, "[A]1.[X]2>>[A]1{E}[X]2", o, "[A]1{E}[X]2>>[A]1{E}[X]2")
	# AEX A X AEX AEX           parallel edge in L
	# AEX A X AEX AFX           parallel edge in L
	c(r1, "[A]1{E}[X]2>>[A]1{E}[X]2", o, None)
	c(r1, "[A]1{E}[X]2>>[A]1{F}[X]2", o, None)

	# ---------------------------------------------------
	# L   R   L   R    L   R    Note
	# === === === ===  === ===  =========================
	#     AEX
	print("   >>AEX with everything")
	print("-" * 80)
	r1 = ">>[A]1{E}[X]2"
	#     AEX (empty)           no overlaps
	#     AEX A                 no overlaps
	#     AEX     A             no overlaps
	#     AEX A   A             no overlaps
	#     AEX A   B             no overlaps
	#     AEX AEX      (empty)
	#     AEX AEX A        A
	#     AEX AEX A X      A X
	c(r1, "[A]1{E}[X]2>>",          o, ">>")
	c(r1, "[A]1{E}[X]2>>[A]1",      o, ">>[A]")
	c(r1, "[A]1{E}[X]2>>[A]1.[X]2", o, ">>[A].[X]")
	#     AEX     AEX           no overlaps
	#     AEX A   AEX           no overlaps
	#     AEX A X AEX           parallel edge in R
	c(r1, "[A]1.[X]2>>[A]1{E}[X]2", o, None)
	#     AEX AEX AEX      AEX
	#     AEX AEX AFX      AFX
	c(r1, "[A]1{E}[X]2>>[A]1{E}[X]2", o, ">>[A]{E}[X]")
	c(r1, "[A]1{E}[X]2>>[A]1{F}[X]2", o, ">>[A]{F}[X]")

	# ---------------------------------------------------
	# L   R   L   R    L   R    Note
	# === === === ===  === ===  =========================
	print("A  >>AEX with everything")
	print("-" * 80)
	r1 = "[A]1>>[A]1{E}[X]2"
	# A   AEX (empty)           no overlaps
	# A   AEX A                 no overlaps
	# A   AEX     A             no overlaps
	# A   AEX A   A             no overlaps
	# A   AEX A   B             no overlaps
	# A   AEX AEX      A
	# A   AEX AEX A    A   A
	# A   AEX AEX A X  A   A X
	c(r1, "[A]1{E}[X]2>>",          o, "[A]1>>")
	c(r1, "[A]1{E}[X]2>>[A]1",      o, "[A]1>>[A]1")
	c(r1, "[A]1{E}[X]2>>[A]1.[X]2", o, "[A]1>>[A]1.[X]")
	# A   AEX     AEX           no overlaps
	# A   AEX A   AEX           no overlaps
	# A   AEX A X AEX           parallel edge in R
	c(r1, "[A]1.[X]2>>[A]1{E}[X]2", o, None)
	# A   AEX AEX AEX  A   AEX
	# A   AEX AEX AFX  A   AFX
	c(r1, "[A]1{E}[X]2>>[A]1{E}[X]2", o, "[A]1>>[A]1{E}[X]")
	c(r1, "[A]1{E}[X]2>>[A]1{F}[X]2", o, "[A]1>>[A]1{F}[X]")

	# ---------------------------------------------------
	# L   R   L   R    L   R    Note
	# === === === ===  === ===  =========================
	print("A X>>AEX with everything")
	print("-" * 80)
	r1 = "[A]1.[X]2>>[A]1{E}[X]2"
	# A X AEX (empty)           no overlaps
	# A X AEX A                 no overlaps
	# A X AEX     A             no overlaps
	# A X AEX A   A             no overlaps
	# A X AEX A   B             no overlaps
	# A X AEX AEX      A X
	# A X AEX AEX A    A X A
	# A X AEX AEX A X  A X A X
	c(r1, "[A]1{E}[X]2>>",          o, "[A]1.[X]2>>")
	c(r1, "[A]1{E}[X]2>>[A]1",      o, "[A]1.[X]2>>[A]1")
	c(r1, "[A]1{E}[X]2>>[A]1.[X]2", o, "[A]1.[X]2>>[A]1.[X]2")
	# A X AEX     AEX           no overlaps
	# A X AEX A   AEX           no overlaps
	# A X AEX A X AEX           parallel edge in R
	c(r1, "[A]1.[X]2>>[A]1{E}[X]2", o, None)
	# A X AEX AEX AEX  A X AEX
	# A X AEX AEX AFX  A X AFX
	c(r1, "[A]1{E}[X]2>>[A]1{E}[X]2", o, "[A]1.[X]2>>[A]1{E}[X]2")
	c(r1, "[A]1{E}[X]2>>[A]1{F}[X]2", o, "[A]1.[X]2>>[A]1{F}[X]2")

	# ---------------------------------------------------
	# L   R   L   R    L   R    Note
	# === === === ===  === ===  =========================
	print("AEX>>AEX with everything")
	print("-" * 80)
	r1 = "[A]1{E}[X]2>>[A]1{E}[X]2"
	# AEX AEX (empty)           no overlaps
	# AEX AEX A                 no overlaps
	# AEX AEX     A             no overlaps
	# AEX AEX A   A             no overlaps
	# AEX AEX A   B             no overlaps
	# AEX AEX AEX      AEX
	# AEX AEX AEX A    AEX A
	# AEX AEX AEX A X  AEX A X
	c(r1, "[A]1{E}[X]2>>",          o, "[A]1{E}[X]2>>")
	c(r1, "[A]1{E}[X]2>>[A]1",      o, "[A]1{E}[X]2>>[A]1")
	c(r1, "[A]1{E}[X]2>>[A]1.[X]2", o, "[A]1{E}[X]2>>[A]1.[X]2")
	# AEX AEX     AEX           no overlaps
	# AEX AEX A   AEX           no overlaps
	# AEX AEX A X AEX           parallel edge in R
	c(r1, "[A]1.[X]2>>[A]1{E}[X]2", o, None)
	# AEX AEX AEX AEX  AEX AEX
	# AEX AEX AEX AFX  AEX AFX
	c(r1, "[A]1{E}[X]2>>[A]1{E}[X]2", o, "[A]1{E}[X]2>>[A]1{E}[X]2")
	c(r1, "[A]1{E}[X]2>>[A]1{F}[X]2", o, "[A]1{E}[X]2>>[A]1{F}[X]2")

	# ---------------------------------------------------
	# L   R   L   R    L   R    Note
	# === === === ===  === ===  =========================
	print("AQX>>AEX with everything")
	print("-" * 80)
	r1 = "[A]1{Q}[X]2>>[A]1{E}[X]2"
	# AQX AEX (empty)           no overlaps
	# AQX AEX A                 no overlaps
	# AQX AEX     A             no overlaps
	# AQX AEX A   A             no overlaps
	# AQX AEX A   B             no overlaps
	# AQX AEX AEX      AQX
	# AQX AEX AEX A    AQX A
	# AQX AEX AEX A X  AQX A X
	c(r1, "[A]1{E}[X]2>>",          o, "[A]1{Q}[X]2>>")
	c(r1, "[A]1{E}[X]2>>[A]1",      o, "[A]1{Q}[X]2>>[A]1")
	c(r1, "[A]1{E}[X]2>>[A]1.[X]2", o, "[A]1{Q}[X]2>>[A]1.[X]2")
	# AQX AEX     AEX           no overlaps
	# AQX AEX A   AEX           no overlaps
	# AQX AEX A X AEX           parallel edge in R
	c(r1, "[A]1.[X]2>>[A]1{E}[X]2", o, None)
	# AQX AEX AEX AEX  AQX AEX
	# AQX AEX AEX AFX  AQX AFX
	c(r1, "[A]1{E}[X]2>>[A]1{E}[X]2", o, "[A]1{Q}[X]2>>[A]1{E}[X]2")
	c(r1, "[A]1{E}[X]2>>[A]1{F}[X]2", o, "[A]1{Q}[X]2>>[A]1{F}[X]2")


def doChecks(c):
	_check_0_vertexOverlap(c)
	_check_1_vertexOverlap(c)
	_check_2_vertexOverlap(c)
