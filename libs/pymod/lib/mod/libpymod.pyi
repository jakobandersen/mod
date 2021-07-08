import enum
from typing import Callable, Generic, Iterable, List, Optional, overload, Tuple, TypeVar, Union

T = TypeVar("T")
U = TypeVar("U")


#-----------------------------------------------------------------------------
# Meta
#-----------------------------------------------------------------------------

# Collection
#-----------------------------------------------------------------------------

class Vec(List[T]): ...

class _VecDGHyperEdge(Vec[DGHyperEdge]): ...
class _VecDGVertex(Vec[DGVertex]): ...
class _VecDGStrat(Vec[DGStrat]): ...
class _VecGraph(Vec[Graph]): ...
class _VecRCExpExp(Vec[RCExpExp]): ...
class _VecRule(Vec[Rule]): ...


# Function
#-----------------------------------------------------------------------------

class _Func_VecGraph:
	def __call__(self) -> Vec[Graph]: ...
class _Func_String:
	def __call__(self) -> str: ...

class _Func_BoolDerivation:
	def __call__(self, d: Derivation) -> bool: ...

class _Func_BoolDGVertex:
	def __call__(self, v: DGVertex) -> bool: ...
class _Func_StringDGVertex:
	def __call__(self, v: DGVertex) -> str: ...

class _Func_BoolDGHyperEdge:
	def __call__(self, e: DGHyperEdge) -> bool: ...
class _Func_StringDGHyperEdge:
	def __call__(self, e: DGHyperEdge) -> str: ...
class _Func_DoubleDGHyperEdge:
	def __call__(self, e: DGHyperEdge) -> float: ...

class _Func_BoolGraph:
	def __call__(self, g: Graph) -> bool: ...
class _Func_IntGraph:
	def __call__(self, g: Graph) -> int: ...

class _Func_StringGraphDGBool:
	def __call__(self, g: Graph, dg: DG, first: bool) -> str: ...

class _Func_BoolGraphDGStratGraphStateBool:
	def __call__(self, g: Graph, gs: DGStrat.GraphState, first: bool) -> bool: ...

class _Func_VoidDGStratGraphState:
	def __call__(self, gs: DGStrat.GraphState) -> None: ...


#-----------------------------------------------------------------------------
# Top-level non-meta
#-----------------------------------------------------------------------------

# Chem
#-----------------------------------------------------------------------------

class BondType(enum.Enum): ...
def _bondTypeToString(self: BondType) -> str: ...


# Config
#-----------------------------------------------------------------------------

class LabelType(enum.Enum):
	String: int
def _LabelType__str__(self: LabelType) -> str: ...

class LabelRelation(enum.Enum):
	Isomorphism: int
def _LabelRelation__str__(self: LabelRelation) -> str: ...

class LabelSettings:
	@overload
	def __init__(self, lt: LabelType, lr: LabelRelation) -> None: ...
	@overload
	def __init__(self, lt: LabelType, lr: LabelRelation, slr: LabelRelation) -> None: ...

class IsomorphismPolicy(enum.Enum):
	Check: int
def _IsomorphismPolicy__str__(self: IsomorphismPolicy) -> str: ...

class SmilesClassPolicy(enum.Enum):
	NoneOnDuplicate: int
	ThrowOnDuplicate: int
	MapUnique: int
def _SmilesClassPolicy__str__(self: SmilesClassPolicy) -> str: ...


def _getAvailableILPSolvers() -> List[str]: ...


class Config:
	class Common:
		def __init__(self) -> None:
			self.ignoreDeprecation: bool = ...

	def __init__(self) -> None:
		self.common = Config.Common()

def getConfig() -> Config: ...


# Error
#-----------------------------------------------------------------------------

class LogicError(Exception): ...


# Derivation
#-----------------------------------------------------------------------------

class Derivation: ...
class Derivations: ...


# Misc
#-----------------------------------------------------------------------------

def rngUniformReal() -> float: ...


#-----------------------------------------------------------------------------
# causality
#-----------------------------------------------------------------------------

# Petri
#-----------------------------------------------------------------------------

class PetriNet:
	def __init__(self, dg: DG) -> None: ...
	def syncSize(self) -> None: ...


class PetriNetMarking:
	def __init__(self, net: PetriNet) -> None: ...
	def syncSize(self) -> None: ...
	def add(self, v: DGVertex, c: int) -> int: ...
	def remove(self, v: DGVertex, c: int) -> int: ...
	def __getitem__(self, v: DGVertex) -> int: ...
	def getAllEnabled(self) -> List[DGHyperEdge]: ...
	def getNonZeroPlaces(self) -> List[DGVertex]: ...
	def getEmptyPostPlaces(self, e: DGHyperEdge) -> List[DGVertex]: ...
	def fire(self, e: DGHyperEdge) -> None: ...


class PetriNetMarkingSet:
	def addIfNotSubset(self, m: PetriNetMarking) -> bool: ...


# Stochsim
#-----------------------------------------------------------------------------

class MassActionKinetics:
	def __init__(self, dg: DG, rate: Callable[[DGHyperEdge], float]) -> None: ...
	def draw(self, possibles: List[DGHyperEdge], m: PetriNetMarking) -> Tuple[DGHyperEdge, float]: ...


class EventTrace:
	def __init__(self, initialState: PetriNetMarking) -> None: ...
	def addEdge(self, time: float, e: DGHyperEdge) -> None: ...
	def addTransfer(self, time: float, v: DGVertex, count: int) -> None: ...


#-----------------------------------------------------------------------------
# dg
#-----------------------------------------------------------------------------

class DG:
	def __init__(self, *, labelSettings: LabelSettings=...,
		graphDatabase: Iterable[Graph]=...,
		graphPolicy: IsomorphismPolicy=...) -> None: ...
	def findVertex(self, g: Graph) -> DGVertex: ...
	@overload
	def findEdge(self, sources: List[DGVertex], targets: List[DGVertex]) -> DGHyperEdge: ...
	@overload
	def findEdge(self, sourcesGraphs: List[Graph], targetGraphs: List[Graph]) -> DGHyperEdge: ...
	def build(self) -> DGBuilder: ...
	def print(self, printer: DGPrinter=..., data: Optional[DGPrintData]=...) -> Tuple[str, str]: ...
	@staticmethod
	def load(graphDatabase: List[Graph], ruleDatabase: List[Rule], file: str, graphPolicy: IsomorphismPolicy=..., verbosity: int=...) -> DG: ...


class DGBuilder:
	def __enter__(self) -> DGBuilder: ...
	def __exit__(self, exc_type, exc_val, exc_tb) -> None: ...

	def addDerivation(self, d: Derivations, graphPolicy: IsomorphismPolicy = ...) -> DGHyperEdge: ...
	def execute(self, strategy: DGStrat, *, verbosity: int=..., ignoreRuleLabelTypes: bool=...) -> DGExecuteResult: ...
	def apply(self, graphs: List[Graph], rule: Rule, onlyProper: bool = ..., verbosity: int = ..., graphPolicy: IsomorphismPolicy = ...) -> List[DGHyperEdge]: ...
	def addAbstract(self, description: str) -> None: ...
	def load(self, ruleDatabase: List[Rule], file: str, verbosity: int = ...) -> None: ...


class DGExecuteResult:
	def list(self, *, withUniverse: bool=...) -> None: ...


class DGVertex:
	graph: Graph


class DGHyperEdge:
	def print(self, printer: GraphPrinter=..., nomatchColour: str=..., matchColour: str=...) -> List[Tuple[str, str]]: ...


class DGPrinter:
	def pushVertexVisible(self, f: Union[Callable[[DGVertex], bool], bool]) -> None: ...
	def pushEdgeVisible(self, f: Union[Callable[[DGHyperEdge], bool], bool]) -> None: ...
	def pushVertexLabel(self, f: Union[Callable[[DGVertex], str], str]) -> None: ...
	def pushEdgeLabel(self, f: Union[Callable[[DGHyperEdge], str], str]) -> None: ...
	def pushVertexColour(self, f: Union[Callable[[DGVertex], str], str], extendToEdges: bool=...) -> None: ...
	def pushEdgeColour(self, f: Union[Callable[[DGHyperEdge], str], str]) -> None: ...
	def setRotationOverwrite(self, f: Union[Callable[[Graph], int], int]) -> None: ...
	def setMirrorOverwrite(self, f: Union[Callable[[Graph], bool], bool]) -> None: ...


class DGPrintData:
	def __init__(self, dg: DG) -> None: ...


class DGStrat:
	class GraphState: ...

	def __rshift__(self, other: DGStrat) -> DGStrat: ...
	def __rrshift__(self, other: DGStrat) -> DGStrat: ...

	@staticmethod
	def makeAddStatic(onlyUniverse: bool, graphs: List[Graph], graphPolicy: IsomorphismPolicy) -> DGStrat: ...
	@staticmethod
	def makeAddDynamic(onlyUniverse: bool, graphsFunc: Callable[[], List[Graph]], graphPolicy: IsomorphismPolicy) -> DGStrat: ...
	@staticmethod
	def makeSequence(strats: List[DGStrat]) -> DGStrat: ...
	@staticmethod
	def makeParallel(strats: List[DGStrat]) -> DGStrat: ...
	@staticmethod
	def makeFilter(alsoUniverse: bool, p: Callable[[Graph, GraphState, bool], bool]) -> DGStrat: ...
	@staticmethod
	def makeExecute(func: Callable[[GraphState], None]) -> DGStrat: ...
	@staticmethod
	def makeRule(rule: Rule) -> DGStrat: ...
	@staticmethod
	def makeLeftPredicate(p: Callable[[Derivation], bool], strat: DGStrat) -> DGStrat: ...
	@staticmethod
	def makeRightPredicate(p: Callable[[Derivation], bool], strat: DGStrat) -> DGStrat: ...
	@staticmethod
	def makeRevive(strat: DGStrat) -> DGStrat: ...
	@staticmethod
	def makeRepeat(limit: int, strat: DGStrat) -> DGStrat: ...


#-----------------------------------------------------------------------------
# graph
#-----------------------------------------------------------------------------

class Graph:
	name: str

	def aut(self, labelSettings: LabelSettings=...) -> GraphAutGroup: ...
	@overload
	def print(self) -> Tuple[str, str]: ...
	@overload
	def print(self, first: GraphPrinter, second: Optional[GraphPrinter]=...) -> Tuple[str, str]: ...
	def getGMLString(self, withCoords: bool=...) -> str: ...
	def printGML(self, withCoords: bool=...) -> str: ...
	def isomorphism(self, host: Graph, maxNumMatches: int=..., labelSettings: LabelSettings=...) -> int: ...
	def monomorphism(self, host: Graph, maxNumMatches: int=..., labelSettings: LabelSettings=...) -> int: ...

	@staticmethod
	def fromGMLString(     s: str) -> Graph: ...
	@staticmethod
	def fromGMLFile(       f: str) -> Graph: ...
	@staticmethod
	def fromGMLStringMulti(s: str) -> List[Graph]: ...
	@staticmethod
	def fromGMLFileMulti(  f: str) -> List[Graph]: ...

	@staticmethod
	def fromDFS(s: str) -> Graph: ...

	@staticmethod
	def fromSMILES(     s: str, allowAbstract: bool = ..., classPolicy: SmilesClassPolicy = ...) -> Graph: ...
	@staticmethod
	def fromSMILESMulti(s: str, allowAbstract: bool = ..., classPolicy: SmilesClassPolicy = ...) -> List[Graph]: ...


class GraphAutGroup: ...


class GraphPrinter: ...



#-----------------------------------------------------------------------------
# rule
#-----------------------------------------------------------------------------

class RCEvaluator:
	def eval(self, exp: RCExpExp, *, verbosity: int=...) -> List[Rule]: ...


class RCExpExp: ...

class RCExpUnion(RCExpExp):
	def __init__(self, es: Vec[RCExpExp]) -> None: ...
class RCExpId(RCExpExp):
	def __init__(self, g: Graph) -> None: ...
class RCExpBind(RCExpExp):
	def __init__(self, g: Graph) -> None: ...
class RCExpUnbind(RCExpExp):
	def __init__(self, g: Graph) -> None: ...

class RCExpComposeCommon(RCExpExp):
	def __init__(self, first: RCExpExp, second: RCExpExp, discardNonchemical: bool=..., maximum: bool=..., connected: bool=..., includeEmpty: bool=...) -> None: ...

class RCExpComposeParallel(RCExpExp):
	def __init__(self, first: RCExpExp, second: RCExpExp, discardNonchemical: bool=...) -> None: ...

class RCExpComposeSub(RCExpExp):
	def __init__(self, first: RCExpExp, second: RCExpExp, discardNonchemical: bool=..., allowPartial: bool=...) -> None: ...

class RCExpComposeSuper(RCExpExp):
	def __init__(self, first: RCExpExp, second: RCExpExp, discardNonchemical: bool=..., allowPartial: bool=..., enforceConstraints: bool=...) -> None: ...


class RCMatch:
	def compose(self, *, verbose: bool=...) -> Rule: ...
	def composeAll(self, *, maximum: bool=..., verbose: bool=...) -> List[Rule]: ...


class Rule(RCExpExp):
	@overload
	def print(self, printCombined: bool=...) -> Tuple[str, str]: ...
	@overload
	def print(self, first: GraphPrinter, second: Optional[GraphPrinter]=..., printCombined: bool=...) -> Tuple[str, str]: ...
	def getGMLString(self, withCoords: bool=...) -> str: ...
	def printGML(self, withCoords: bool=...) -> str: ...
	def isomorphism(self, host: Rule, maxNumMatches: int=..., labelSettings: LabelSettings=...) -> int: ...
	def monomorphism(self, host: Rule, maxNumMatches: int=..., labelSettings: LabelSettings=...) -> int: ...


	def __rshift__(self, other: DGStrat) -> DGStrat: ...

	@staticmethod
	def fromGMLString(s: str, invert: bool=...) -> Rule: ...
	@staticmethod
	def fromGMLFile(f: str, invert: bool=...) -> Rule: ...


def _rcEvaluator(rules: Iterable[Rule], labelSettings: LabelSettings=...) -> RCEvaluator: ...
