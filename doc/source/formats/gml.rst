.. _gml:

GML
###

MØD uses the Graph Modelling Language (GML) for general specification of graphs
and rules.
The parser recognises most of the published `specification
<http://www.fim.uni-passau.de/fileadmin/files/lehrstuhl/brandenburg/projekte/gml/gml-technical-report.pdf>`__,
with regard to syntax. The specific grammar is as follows.

.. productionlist:: gml
   GML: (`key` `value`)*
   key: `identifier`
   value: int
        : float
        : quoteEscapedString
        : `list`
   list: '[' (`key` `value`)* ']'
   identifier: a word matching the regex "[a-zA-Z][a-zA-Z0-9]*"

A ``quoteEscapedString`` is zero or more characters surrounded by double
quotation marks.  To include a ``\"`` character it must be escaped.  Tabs,
newlines, and backslashses can be written as ``\t``, ``\n``, and ``\\``.
GML code may have line comments, starting with ``#``.
They are ignored during parsing.


.. _graph-gml:

Graph
=====

A graph can be specified as :ref:`GML` by giving a list of vertices and edges
with the key ``graph``.
The following grammar exemplifies the required key-value structure.

.. productionlist:: gml
   graphGML: 'graph [' (`node` | `edge`)* ']'
   node: 'node [ id' int 'label' quoteEscapedString ']'
   edge: 'edge [ source' int 'target' int 'label' quoteEscapedString ']'

Note though that list elements can appear in any order.


.. _rule-gml:

Rule
====

A rule :math:`(L\leftarrow K\rightarrow R)` in :ref:`GML` format is specified
as three graph fragments; ``left``, ``context``, and ``right``.
From those
:math:`L` is constructed as ``left`` :math:`\cup` ``context``,
:math:`R` as ``right`` :math:`\cup` ``context``, and
:math:`K` as ``context`` :math:`\cup` (``left`` :math:`\cap` ``right``).
Each graph fragment is specified as a list of vertices and edges, similar to a
graph in GML format.
The key-value structure is exemplified by the following grammar.

.. productionlist:: gml
   ruleGML: 'rule ['
          :    [ 'ruleID' quoteEscapedString ]
          :    [ 'labelType "' `labelType` '"' ]
          :    [ `leftSide` ]
          :    [ `context` ]
          :    [ `rightSide` ]
          :    `matchConstraint`*
          : ']'
   labelType: 'string' | 'term'
   leftSide: 'left [' (`node` | `edge`)* ']'
   context: 'context [' (`node` | `edge`)* ']'
   rightSide: 'right [' (`node` | `edge`)* ']'
   matchConstraint: `adjacency` | `labelAny`
   adjacency: 'constrainAdj ['
            :    'id' int
            :    [ 'nodeLabels [' `labelList` ']' ]
            :    [ 'edgeLabels [' `labelList` ']' ]
            :    'op "' `op` '"'
            :    'count' unsignedInt
            : ']'
   labelAny: 'constrainLabelAny ['
           :    'label' quoteEscapedString
           :    'labels [' `labelList` ']'
           : ']'
   labelList: ('label' quoteEscapedString)*
   op: '<' | '<=' | '=' | '>=' | '>'

Note though that list elements can appear in any order.

For details on the constraints, see :ref:`rule-constraints`.


A Note on Term Labels
---------------------

As described in :ref:`graph-model-terms` it is possible to interpret the
ordinary vertex and edges labels as first-order terms.
When using the label ``*`` it will be interpreted as an unnamed term variable. 
Consider the rule::

	rule [
		left  [ node [ id 0 label "*" ] ]
		right [ node [ id 0 label "*" ] ]
	]

In string mode this is simply an identity rule, but in term mode each ``*``
is interpreted as an unnamed variable. Be careful that in this case the
two labels are interepreted as *the same* variable. That is, it is equivalent
to::

	rule [
		left  [ node [ id 0 label "_A" ] ]
		right [ node [ id 0 label "_A" ] ]
	]

If you wish to replace any vertex label with an explicit new variable, you can
write it as::

	rule [
		left  [ node [ id 0 label "_A" ] ]
		right [ node [ id 0 label "_B" ] ]
	]


