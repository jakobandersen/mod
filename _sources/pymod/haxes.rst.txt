.. _py-haxes:

The ``haxes`` module
####################

.. default-domain:: py

.. py:currentmodule:: mod
.. cpp:namespace:: mod

.. versionadded:: 1.1

This module contains various experimental functionality that has been put together pragmatically.
That is that it has not been optimized in any significant way, has not been tested thoroughly or systematically, and it may not handle all corner cases.
The plan is that the functionality should be reimplemented in a proper way at a later stage.

.. py:function:: haxes.ruleFromReactionSmiles(line, name=None, allowAbstract=False, *, invert=False, add=True)

	:returns: a rule loaded from the given reaction SMILES string.
		The paramter ``allowAbstract`` is passed to :func:`Graph.fromSMILES`,
		while the remaining paramters are passed to :func:`Rule.fromGMLString`.
	:rtype: Rule

.. py:function:: haxes.reactionSmilesFromRule(r)

	:param Rule r: the rule to convert to a reaction SMILES string.
	:returns: a reaction SMILES string that encodes the given rule.
	:rtype: str
