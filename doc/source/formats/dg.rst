.. _dg_abstract-desc:

Abstract Derivation Graphs
##########################

Sometimes it is really convenient to quickly write down a few equations to
describe a "derivation graph", without associating actual graphs and rules to
it. That is, only specifying the underlying network.
The network description is a string adhering to the following grammar:

.. productionlist:: dgAbstract
	description: `derivation` { `derivation` }
	derivation: `side` ("->" | "<=>") `side`
	side: `term` { "+" `term` }
	term: [ unsignedInt ] `identifier`
	identifier: any character sequence without spaces

Note that the :token:`~dgAbstract:identifier` definition
in particular means that whitespace is important between coefficients and
identifiers. E.g., ``2 A -> B`` is different from ``2A -> B``.

See also :py:func:`DG.Builder.addAbstract`/:cpp:func:`dg::Builder::addAbstract`.
