.. _graph-smiles:

SMILES
######

The `Simplified molecular-input line-entry system` is a line notation for
molecules. MØD can load most SMILES strings, and converts them internally to
labelled graphs according to a specific :ref:`molecule encoding <mol-enc>`.
For graphs that are sufficiently molecule-like, a SMILES string can be
generated. The generated strings are canonical in the sense that the same
version of MØD will print the same SMILES string for isomorphic molecules.
See [AM-Canon]_ for information on how the canonicalisation algorithm works.

The reading of SMILES strings is based on the `OpenSMILES
<http://www.opensmiles.org/>`_ specification, but with the following
notes/changes.

- Only single SMILES strings are accepted, i.e., not multiple strings separated
  by white-space.
- Up and down bonds are regarded as implicit bonds, i.e., they might represent
  either a sngle bond or an aromatic bond. The stereo information is ignored.
- Atom classes are (mostly) ignored. They can be used to specify unique IDs to
  atoms.
- Wildcard atoms (specified with ``*``) are converted to vertices with label
  ``*``. When inside brackets, only the hydrogen count and atom class is then
  permitted.
- Abstract vertex labels can be specified inside brackets. The bracket must in
  that case only contain the label and an optional class label.
  The label must be a non-empty string without ``:`` and with balanced square
  brackets.
- Charges of magnitude 2 and 3 may be specified with repeated ``-`` and ``+``.
- The bond type ``$`` is currently not allowed.
- Aromaticity can only be specified using the bond type ``:``
  or using the special lower case atoms.
  I.e., ``c1ccccc1`` and ``C1:C:C:C:C:C:1`` represent the same molecule,
  but ``C1=CC=CC=C1`` is a different molecule.
  The lower-case atoms are converted to normal case when used as a label.
- Ring-bonds and branches may appear in mixed order. The normal order is to
  have all ring-bonds first and all branches, e.g., ``C123(O)(N)``.
  The parser accepts them in mixed order, e.g., ``C1(O)2(N)3``.
- Implicit hydrogens are added following a more complicated procedure
  (see below).
- A bracketed atom can have a radical by writing a dot (``.``) between the
  position of the charge and the position of the class.

The written SMILES strings are intended to be canonical and may not conform to
any "prettyness" standards.

Implicit Hydrogen Atoms
=======================

When SMILES strings are written they will use implicit hydrogens whenever they
can be inferred when reading the string back in.
For the purposes of implicit hydrogens we use the following definition of
valence for an atom.
The valence of an atom is the weighted sum of its incident edges, where single
(``-``) and aromatic (``:``) bonds have weight 1, double bounds (``=``) have
weight 2, and triple bonds (``#``) have weight 3.
If an atom has an incident aromatic bond, its valence is increased by 1.
The atoms that can have implicit hydrogens are
B, C, N, O, P, S, F, Cl, Br, and I.
Each have a set of so-called "normal" valences as shown in the following table.
The atoms N and S additionally have certain sets of incident edges that are
also considered "normal", which are also listed in the table.

=============  =====================================================================
Atom           Normal Valences and Neighbourhoods
=============  =====================================================================
B              3
C              4
N              3, 5, :math:`\{-, :, :\}`, :math:`\{-, -, =\}`, :math:`\{:, :, :\}`
O              2
P              3, 5
S              2, 4, 6, :math:`\{:, :\}`
F, Cl, Br, I   1
=============  =====================================================================

If the set of incident edges is listed in the table, then no hydrogens are
added. If the valence is higher than the highest normal valence, then no
hydrogens are added. Otherwise, hydrogens are added until the valence is at the
next higher normal valence.

When writing SMILES strings the inverse procedure is used.
