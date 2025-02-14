.. _dg_abstract-desc:

Abstract Derivation Graphs
##########################

Sometimes it is really convenient to quickly write down a few equations to
describe a "derivation graph", without associating actual graphs and rules to
it. That is, only specifying the underlying network.
The network description is a string adhering to the following grammar:

.. productionlist:: dgAbstract
	description: `derivation` { `derivation` }
	derivation: [ "#" `identifier` ] `side` ("->" | "<=>") `side`
	side: `term` { "+" `term` }
	term: [ unsignedInt ] `identifier`
	identifier: any character sequence without spaces

Note that the :token:`~dgAbstract:identifier` definition
in particular means that whitespace is important between coefficients and
identifiers. E.g., ``2 A -> B`` is different from ``2A -> B``.
Each derivation may optionally start with an ID for later getting a handle to it with
:py:meth:`DG.Builder.AddAbstractResult.getEdge`/:cpp:func:`dg::AddAbstractResult::getEdge`.
The prefixed ``#`` character is not part of the ID.
For example, if the description is ``#R1 2 a -> b  #R2 b -> 2 c``, the two hyperedges
can be accessed by giving ``R1`` and ``R2`` as IDs.
If a derivation is bidirectional, e.g., ``#1 a <=> b``, then the ID (here ``1``), gives
the the left-to-right derivation as result. The inverse can then be accessed from that
hyperedge with
:py:attr:`DG.HyperEdge.inverse`/:cpp:func:`dg::DG::HyperEdge::getInverse`.

See also :py:func:`DG.Builder.addAbstract`/:cpp:func:`dg::Builder::addAbstract`.
