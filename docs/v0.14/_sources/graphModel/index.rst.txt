.. cpp:namespace:: mod
.. py:currentmodule:: mod

.. _graph-model:

*******************************
Graph, Rule, and Molecule Model
*******************************

A central component of MØD is the :cpp:class:`graph::Graph`/:py:class:`Graph`
which simplified represents simple graphs with labels on vertices and edges.
However, how the labels are interepreted and how graph morphisms are defined
depends which :cpp:class:`LabelSettings`/:py:class:`LabelSettings` object you
pass to various algorithms.
Another complication is that in the formal mathematics for describing
graph transformation through a :cpp:class:`rule::Rule`/:py:class:`Rule` classes,
we need need a slightly more complicated description of the graphs in order
to make the mathematics work.

We therefore start with the :ref:`simplified graph model <graph-model-basic>`
with the labels being opaque types.
:ref:`Then <graph-model-terms>` we explore the different label settings
which switches the mathematical category of the graphs.
Afterwards the :ref:`rule model <graph-model-rules>` is explained where we
get into the mathematical details for label-encoding which is necessary to
formally make the graph transformation composable.
Lastly we see how MØD interprets graphs as molecule (fragments) in order to
create depictions that resembles ordinary molecule depictions.


.. _graph-model-basic:

Simple Graphs with String Labels
################################

In the simplified model, a graph
:math:`G = (V, E, l\colon V\cup E\rightarrow \Omega)`
is undirected, and neither has loop edges nor parallel edges.
The function :math:`l` assigns a label to each vertex and edge.
A morphism between two graphs :math:`m\colon G\rightarrow H` is a graph morphism
which commutes with the labelling functions.
That is,

- graph morphism: for every edge :math:`(u, v) \in E_G` we must have an
  corresponding edge
  :math:`(m(u), m(v)) \in E_H` (:math:`m` is a graph morphism), and
- vertex labels: each vertex :math:`v \in V_G` is mapped to a vertex with the
  same label, :math:`l_G(v) = l_H(m(v))`.
- edge labels: each edge :math:`e \in E_G` is mapped to a vertex with the
  same label, :math:`l_G(e) = l_H(m(e))`.

The only requirement of :math:`\Omega` is thus that the elements can be
compared for equality.
In MØD this correspons to using
:cpp:any:`LabelType::String`/:py:obj:`LabelType.String`
in the a :cpp:class:`LabelSettings`/:py:class:`LabelSettings` object,
and in the API of PyMØD, this is the default.

.. note:: When using :cpp:any:`LabelType::String`/:py:obj:`LabelType.String`
	the :cpp:any:`LabelRelation`/:py:obj:`LabelRelation` in 
	:cpp:class:`LabelSettings`/:py:class:`LabelSettings` doesn't matter,

.. note:: Technically, in this basic model the stereo setting in
	:cpp:class:`LabelSettings`/:py:class:`LabelSettings` should be disabled as
	well. Though, the stereo model is in an experimental stage and we will
	therefore not explore it further.

A :cpp:class:`graph::Graph`/:py:class:`Graph` is further required to be
connected.


.. _graph-model-terms:

Switching Category --- First-Order Terms as Labels
##################################################

We can imbue the label set :math:`Omega` with extra structure by using
:cpp:any:`LabelType::Term`/:py:obj:`LabelType.Term` instead of
:cpp:any:`LabelType::String`/:py:obj:`LabelType.String`.
The labels are now interpreted as `firt-order terms <https://en.wikipedia.org/wiki/Unification_(computer_science)#First-order_term>`__,
and formally the graph/objects are now in a different
`category <https://en.wikipedia.org/wiki/Category_(mathematics)>`__.

In the graph/rule objects it is the same text strings being used either verbatim
when in string mode or being parsed when in term mode.

The following describes how the strings are parsed in term mode.

- A constant or function symbol is a word that can be matched by the regex
  ``[A-Za-z0-9=#:.+-][A-Za-z0-9=#:.+-_]*``.
  This means that all strings that are :ref:`considered "molecular" <mol-enc>`
  can be reinterpreted as constant symbols.
- A variable symbol is a word that can be matched by the regex
  ``_[A-Za-z0-9=#:.+-][A-Za-z0-9=#:.+-_]*``.
  That is, variable is like a constant/function symbol, but with a ``_``
  prepended.
  An unnamed variable can be specified by the special wildcard symbol ``*``.

  The variable symbols matched by the regex ``_[HT][0-9][0-9]*`` are
  discouraged, as they are reserved for converting terms back into strings.
- Function terms start with a function symbol followed by a parenthesis with a
  comma-separated list of terms. They may contain white-space which is ignored.

If parsing of terms fails an exception of type
:cpp:class:`TermParsingError`/:py:class:`TermParsingError`
is thrown. Thus, essentially any function that takes a
:cpp:class:`LabelSettings`/:py:class:`LabelSettings`
object may throw such an exception.

If a graph/rule was created in term mode and then is used in string mode,
the first-order terms are serialized back into strings:

- Constant/function symbols are serialized verbatim.
- Variable names are generated and will be matchable by the regex
  ``_[HT][0-9][0-9]*``. Any original variable names are not saved.
- Function terms will be serialized with a single space character after each
  comma.


Morphisms
=========

First-order terms are partially ordered, and we thus have additional choice in
how to define morphisms.
This choice is in the API selected through the
:cpp:any:`LabelRelation`/:py:obj:`LabelRelation` objects which offers the three
choices "unification", "specialisation", and "isomorphism".
Given a graph morphism :math:`m\colon G\rightarrow H`
let :math:`V_G = \{v_1, v_2, \dots, v_n\}` be the vertices of :math:`G`
and :math:`E_G = \{e_1, e_2, \dots, e_p\}` be the edges of :math:`G`.
Consider the following two terms:

.. math::

	t_G &= \texttt{assoc}(l_G(v_1), l_G(v_2), \dots, l_G(v_n),
	                      l_G(e_1), l_G(e_2), \dots, l_G(e_p))	\\
	t_H &= \texttt{assoc}(l_H(m(v_1)), l_H(m(v_2)), \dots, l_H(m(v_n)),
	                      l_H(m(e_1)), l_H(m(e_2)), \dots, l_H(m(e_p)))

They lay out all the labels of :math:`G` in :math:`t_G` and then the labels
of the corresponding vertices and edges in :math:`H` in :math:`t_H`.

The graph morphism :math:`m` is in term mode a valid morphism if

- with
  :cpp:any:`LabelRelation::Unification`/:py:obj:`LabelRelation.Unification`
  there exists a most-general unifier between :math:`t_G` and :math:`t_H`.
- with
  :cpp:any:`LabelRelation::Specialisation`/:py:obj:`LabelRelation.Specialisation`
  there exists a most-general unifier between :math:`t_G` and :math:`t_H`,
  and it only binds variables in :math:`t_G` but not :math:`t_H`.
  That is, :math:`t_H` is a specialisation of :math:`t_G`.
- with
  :cpp:any:`LabelRelation::Isomorphism`/:py:obj:`LabelRelation.Isomorphism`
  there exists a most-general unifier between :math:`t_G` and :math:`t_H`,
  and it only binds variables to variables, and that variable mapping is
  a bijection.
  That is, the only difference between :math:`t_G` and :math:`t_H` is the naming
  of the variables they use.

In most cases you should use 
:cpp:any:`LabelRelation::Specialisation`/:py:obj:`LabelRelation.Specialisation`
though, when for example using
:cpp:func:`graph::Graph::isomorphism`/:py:meth:`Graph.isomorphism`
to check if two graphs encode the same mathematical object, you should use
:cpp:any:`LabelRelation::Isomorphism`/:py:obj:`LabelRelation.Isomorphism`.
The funtion name thus refers to the type of the *graph morphism* being found,
while the label settings describes the additional requirements for the labels.

The third option,
:cpp:any:`LabelRelation::Unification`/:py:obj:`LabelRelation.Unification`,
should be used very carefully.
For example, when performing graph transofrmation, e.g., through a
:cpp:class:`dg::DG`/:py:class:`DG`, it may give results that are unexpected.
One use-case is when calling
:cpp:func:`graph::Graph::isomorphism`/:py:meth:`Graph.isomorphism`,
where using
:cpp:any:`LabelRelation::Unification`/:py:obj:`LabelRelation.Unification`
means that you will find graph isomorphisms for which the labels on both graphs
can be specialized such that the graph objects becomes isomorphic.

.. note:: For the graph transformation is it necessary to define a distinguished
	set of morphisms :math:`\mathcal{M}` for the term mode.
	These morphisms are those that are graph monomorphisms, but are label
	isomorphisms on the label part.


.. _graph-model-rules:

Rule Model
##########

A :cpp:class:`rule::Rule`/:py:class:`Rule` represents a
`Double Pushout <https://en.wikipedia.org/wiki/Double_pushout_graph_rewriting>`__
(DPO) graph transformation rule :math:`p = (L\xleftarrow{l}K\xrightarrow{r}R)`.
Specifically, they are in the DPO variant with all morphisms being at least
graph monomorphisms.

Label Change
============

The rules allow both for vertices and edges to change labels, but only the
latter is well-defined in traditional DPO transformation.
Thus, mathematically it is not immediately possible to define the graphs as
above. Consider a vertex changing label in a rule, which label should it
have in :math:`K`?
However, as our graphs are not allowed to have loop edges we can as a trick
simply pretend that whenever a vertex would have a label, we attach a loop edge
to the vertex, and put the "vertex" label on that edge.
Note that all morphisms are required to be at least graph monomorphisms, so a
loop edge can not be created inadvertently.

Avoiding Parallel Edges
=======================

Our graphs are defined to be without parallel edges, which presents a
mathematical problem in that certain pushouts are not allowed.
In a DPO direct derivation we can avoid parallel edges by simply introducing
algorithmic ad-hoc constraint, that if the second pushout would result in
parallel edges in the result graph, then the direct derviation is not defined.

However, in the big picture we can avoid this ad-hoc solution and think of the
rules as having implicit negative application conditions (NACs):

- For each pair of vertices :math:`u, v\in V_K`
  which are not connected in :math:`L`, :math:`(l(u), l(v))\not\in E_L`,
  but are connected in :math:`R`, :math:`(r(u), r(v))\in E_R`,
  there is a NAC on :math:`L` preventing edges :math:`(l(u), l(v))`.
- And symmetrically, for each pair of vertices :math:`u, v\in V_K`
  which are connected in :math:`L`, :math:`(l(u), l(v))\not\in E_L`,
  but are not connected in :math:`R`, :math:`(r(u), r(v))\in E_R`,
  there is a NAC on :math:`R` preventing edges :math:`(l(u), l(v))`.


.. _mol-enc:

Molecule Encoding
#################

There is no strict requirement that graphs/rules encode molecule (fragments),
however several optimizations are in place when they do, and depictions are
prettified based on "moleculeness".
The following describes the encoding of molecules in the
:ref:`basic graph model <graph-model-basic>`, that MØD recognizes.

Edges / Bonds
=============

An edge encodes a chemical bond if and only if its label is listed in the table below.

====== ==================
Label  Interpretation  
====== ==================
``-``  Single bond     
``:``  "Aromatic" bond 
``=``  Double bond     
``#``  Triple bond     
====== ==================

Vertices / Atoms
================

A vertex encodes an atom if and only if its label conforms to the following grammar.

.. productionlist:: molecularVertexLabel
	vertexLabel: [ `isotope` ] `atomSymbol` [ `charge` ] [ `radical` ]
	isotope: unsignedInt
	charge: [ singleDigit ] ('-' | '+')
	radical: '.'
	atomSymbol: an atom symbol with the first letter capitalised

.. note:: In the basic model there are no valence requirements for a graph being recognised as a molecule.
