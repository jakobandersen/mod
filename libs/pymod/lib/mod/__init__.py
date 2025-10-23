
import collections.abc
import ctypes
import inspect
import math
import sys
from typing import (
	Any, Callable, cast, Dict, Iterable, List, Optional, Sequence,
	TextIO, Tuple, Type, Union
)

_redirected = False

_oldFlags = sys.getdlopenflags()
sys.setdlopenflags(_oldFlags | ctypes.RTLD_GLOBAL)
from . import libpymod  # noqa
from .libpymod import *  # noqa
from . import causality  # noqa
from . import hyperflow  # noqa
from .hyperflow.vars import *  # type: ignore # noqa
from . import post  # noqa
sys.setdlopenflags(_oldFlags)

# from http://mail.python.org/pipermail/tutor/2003-November/026645.html
class _Unbuffered:
	def __init__(self, stream: TextIO) -> None:
		self.stream = stream
	def write(self, data: Any) -> None:
		self.stream.write(data)
		self.stream.flush()
	def __getattr__(self, attr: str) -> Any:
		return getattr(self.stream, attr)
sys.stdout = _Unbuffered(sys.stdout)

def _NoNew__setattr__(self: Any, name: str, value: Any) -> None:
	if hasattr(self, "_frozen") and self._frozen:
		msg = "Can not modify object '%s' of type '%s'. It has been frozen." % (self, type(self))
		raise AttributeError(msg)
	if hasattr(self, name):
		object.__setattr__(self, name, value)
	else:
		msg = "Can not create new attribute '%s' on object '%s' of type '%s'." % (name, self, type(self))
		msg += "\ndir(" + str(self) + "):\n"
		for name in dir(self):
			msg += "\t" + name + "\n"
		raise AttributeError(msg)


def _fixClass(name: str, c: Any, indent: int) -> None:
	if not name.startswith("_Func_"):
		c.__setattr__ = _NoNew__setattr__

	if name.startswith("_Func_") or name.startswith("_Vec") or name.startswith("Var"):
		c.__hash__ = None
	elif name.endswith("Vertex"):
		assert c.__hash__ is not None and c.__hash__ != object.__hash__
	elif issubclass(c, int):
		# Enums fall into this.
		assert c.__hash__ != object.__hash__
	elif c.__hash__ == object.__hash__:
		c.__hash__ = None

	if not (name.startswith("_Func_") or name.startswith("_Vec") or name.startswith("Var")):
		if name.endswith("Vertex") or name.endswith("Edge"):
			assert c.__bool__ is not None

	for a in inspect.getmembers(c, inspect.isclass):
		if a[0] == "__class__":
			continue
		_fixClass(a[0], a[1], indent + 1)

def _fixModule(modObj):
	classes = inspect.getmembers(modObj, inspect.isclass)
	for c in classes:
		_fixClass(c[0], c[1], 0)

_fixModule(libpymod)
_fixModule(causality)
_fixModule(hyperflow)
_fixModule(post)

#----------------------------------------------------------

def _deprecation(msg: str) -> None:
	if config.common.ignoreDeprecation:
		print("WARNING: {} Use config.common.ignoreDeprecation = False to make this an exception.".format(msg))
	else:
		raise DeprecationWarning("{} Use config.common.ignoreDeprecation = True to make this just a warning.".format(msg))

#----------------------------------------------------------
# Script Inclusion Support
#----------------------------------------------------------

class CWDPath:
	def __init__(self, f: str) -> None:
		self.f = f

_filePrefixes: List[str] = []

def prefixFilename(name: str) -> str:
	if isinstance(name, CWDPath):
		return name.f
	if len(name) == 0 or name[0] == '/':
		return name
	prefixed: List[str] = []
	for s in _filePrefixes:
		if len(s) != 0 and s[0] == '/':
			prefixed[:] = [s]
		else:
			prefixed.append(s)
	prefixed.append(name)
	return ''.join(prefixed)

def pushFilePrefix(prefix: str) -> None:
	_filePrefixes.append(prefix)

def popFilePrefix() -> None:
	if len(_filePrefixes) == 0:
		raise LogicError("popFilePrefix failed; the stack is empty")
	_filePrefixes.pop()


#----------------------------------------------------------
# Wrappers
#----------------------------------------------------------

def _wrap(C: Type["Vec[T]"], l: Union["Vec[T]", Iterable["T"]]) -> "Vec[T]":
	if isinstance(l, C):
		return l
	lcpp = C()
	lcpp.extend(l)
	return lcpp
def _unwrap(lcpp: Iterable["T"]) -> List["T"]:
	l: List[T] = []
	l.extend(lcpp)
	return l

def _funcWrap(F: Type["U"], f,
		resultWrap: Optional[Type["Vec[T]"]] = None,
		module: Any=libpymod) -> "U":
	if hasattr(f, "__call__"):
		class FuncWrapper(F):  # type: ignore
			def __init__(self, f) -> None:
				self.f = f
				F.__init__(self)
			def clone(self) -> "FuncWrapper":
				return module._sharedToStd(FuncWrapper(self.f))
			def __str__(self) -> str:
				lines, lnum = inspect.getsourcelines(self.f)
				source = ''.join(lines)
				filename = inspect.getfile(self.f)
				return "FuncWrapper(%s)\nCode from %s:%d >>>>>\n%s<<<<< Code from %s:%d" % (str(self.f), filename, lnum, source, filename, lnum)
			def __call__(self, *args: List[Any]) -> Union["T", "Vec[T]"]:
				try:
					if resultWrap is not None:
						return _wrap(resultWrap,
							cast(Iterable["T"], self.f(*args)))
					else:
						return cast("T", self.f(*args))
				except:
					print("Error in wrapped function when called:", str(self))
					print("Base type is '" + str(F) + "'")
					raise
										
		res = FuncWrapper(f)
	else:							# assume constant
		class Constant(F):  # type: ignore
			def __init__(self, c: "T") -> None:
				self.c = c
				F.__init__(self)
			def clone(self) -> "Constant":
				return module._sharedToStd(Constant(self.c))
			def __str__(self) -> str:
				return "Constant(" + str(self.c) + ")"
			def __call__(self, *args: List[Any]) -> "T":
				return self.c
		res = Constant(cast("T", f))
	return module._sharedToStd(res)


#----------------------------------------------------------
# Common stuff
#----------------------------------------------------------

_lsString = LabelSettings(LabelType.String, LabelRelation.Isomorphism)

def _reprById(self) -> str:
	return f"{self}({self.id})"

def _eqById(self, other) -> bool:
	return type(self) is type(other) and self.id == other.id

def _ltById(self, other) -> bool:
	if type(self) is type(other):
		return self.id < other.id
	else:
		raise TypeError(f"'<' not supported between instances of '{type(self)}' and '{type(other)}.")

def _setSpecialForIdClass(Class) -> None:
	Class.__repr__ = _reprById
	Class.__eq__ = _eqById
	Class.__lt__ = _ltById
	Class.__hash__ = lambda self: self.id


###########################################################
# Chem
###########################################################

BondType.__str__ = libpymod._bondTypeToString  # type: ignore


###########################################################
# Config
###########################################################

LabelType.__str__ = libpymod._LabelType__str__  # type: ignore
LabelRelation.__str__ = libpymod._LabelRelation__str__  # type: ignore
IsomorphismPolicy.__str__ = libpymod._IsomorphismPolicy__str__  # type: ignore
SmilesClassPolicy.__str__ = libpymod._SmilesClassPolicy__str__  # type: ignore
Action.__str__ = libpymod._Action__str__  # type: ignore

def getAvailableILPSolvers() -> List[str]:
	return _unwrap(libpymod._getAvailableILPSolvers())

config = getConfig()


###########################################################
# Derivation
###########################################################

Derivation.__repr__ = Derivation.__str__  # type: ignore

def _Derivation__setattr__(self: Derivation, name: str, value: Any) -> None:
	if name in ("left", "right"):
		object.__setattr__(self, name, _wrap(libpymod._VecGraph, value))
	else:
		_NoNew__setattr__(self, name, value)
Derivation.__setattr__ = _Derivation__setattr__  # type: ignore


Derivations.__repr__ = Derivations.__str__  # type: ignore

def _Derivations__setattr__(self: Derivation, name: str, value: Any) -> None:
	if name in ("left", "right"):
		object.__setattr__(self, name, _wrap(libpymod._VecGraph, value))
	elif name == "rules":
		object.__setattr__(self, name, _wrap(libpymod._VecRule, value))
	else:
		_NoNew__setattr__(self, name, value)
Derivations.__setattr__ = _Derivations__setattr__  # type: ignore


###########################################################
# DG
###########################################################

def dgDerivations(ders: Iterable[Derivation]) -> DG:
	_deprecation("dgDerivations is deprecated. Use the new build interface.")
	dg = DG()
	with dg.build() as b:
		for d in ders:
			b.addDerivation(d)
	return dg

def dgRuleComp(graphs: Iterable[Graph], strat: DGStrat,
		labelSettings: LabelSettings=_lsString,
		ignoreRuleLabelTypes: bool=False) -> DG:
	_deprecation("dgRuleComp is deprecated. Use the new build interface.")
	dg = DG(labelSettings=labelSettings, graphDatabase=graphs)
	object.__setattr__(dg, "_ruleCompData", {
		"ignoreRuleLabelTypes": ignoreRuleLabelTypes,
		"strat": strat
	})
	def _DG_calc(dg: DG, printInfo: bool=True) -> None:
		_deprecation("DG.calc() is deprecated. Use the new build interface.")
		d = dg._ruleCompData  # type: ignore
		dg.build().execute(d["strat"], ignoreRuleLabelTypes=d["ignoreRuleLabelTypes"])
		object.__setattr__(dg, "_ruleCompData", None)
		object.__setattr__(dg, "calc", None)
	import types
	object.__setattr__(dg, "calc", types.MethodType(_DG_calc, dg))
	return dg

_DG_load_orig = DG.load
def _DG_load(
		graphDatabase: List[Graph], ruleDatabase: List[Rule], f: str,
		graphPolicy: IsomorphismPolicy = IsomorphismPolicy.Check,
		verbosity: int = 2) -> DG:
	return _DG_load_orig(
		_wrap(libpymod._VecGraph, graphDatabase),
		_wrap(libpymod._VecRule, ruleDatabase),
		prefixFilename(f), graphPolicy, verbosity)
DG.load = _DG_load  # type: ignore

_DG__init__old = DG.__init__
def _DG__init__(self: DG, *,
		labelSettings: LabelSettings=_lsString,
		graphDatabase: List[Graph] = [],
		graphPolicy: IsomorphismPolicy = IsomorphismPolicy.Check) -> None:
	return _DG__init__old(self,  # type: ignore
	                      labelSettings,
	                      _wrap(libpymod._VecGraph, graphDatabase),
	                      graphPolicy)
DG.__init__ = _DG__init__  # type: ignore

_DG_print_orig = DG.print
def _DG_print(self: DG, printer: Optional[DGPrinter] = None, data: Optional[DGPrintData] = None) -> Tuple[str, str]:
	if printer is None:
		printer = DGPrinter()
	if data is None:
		data = DGPrintData(self)
	return _DG_print_orig(self, printer, data)
DG.print = _DG_print  # type: ignore

_DG_findEdge_orig = DG.findEdge
def _DG_findEdge(self: DG,
		srcsI: Union[Sequence[Graph], Sequence[DG.Vertex]],
		tarsI: Union[Sequence[Graph], Sequence[DG.Vertex]]) -> DG.HyperEdge:
	srcs = srcsI
	tars = tarsI

	s: Union[None, Type[libpymod._VecGraph], Type[libpymod._VecDGVertex]]
	t: Union[None, Type[libpymod._VecGraph], Type[libpymod._VecDGVertex]]

	if len(srcs) == 0:
		s = None
	elif isinstance(srcs[0], Graph):
		s = libpymod._VecGraph
	else:
		s = libpymod._VecDGVertex

	if len(tars) == 0:
		t = None
	elif isinstance(tars[0], Graph):
		t = libpymod._VecGraph
	else:
		t = libpymod._VecDGVertex

	if s is None and t is None:
		s = libpymod._VecDGVertex
		t = libpymod._VecDGVertex
	elif s is None:
		s = t
	elif t is None:
		t = s
	return _DG_findEdge_orig(self, _wrap(s, srcs), _wrap(t, tars))  # type: ignore
DG.findEdge = _DG_findEdge  # type: ignore

DG.__repr__ = DG.__str__  # type: ignore

def _DG__getattribute__(self: DG, name: str) -> Any:
	if name == "graphDatabase":
		return _unwrap(self._graphDatabase)  # type: ignore
	elif name == "createdGraphs":
		return _unwrap(self._createdGraphs)  # type: ignore
	else:
		return object.__getattribute__(self, name)
DG.__getattribute__ = _DG__getattribute__  # type: ignore

_setSpecialForIdClass(DG)
DG.__repr__ = DG.__str__  # type: ignore


class DGBuildContextManager:
	_builder: Optional[DG.Builder]

	def __init__(self, dg: DG, onNewVertex, onNewHyperEdge) -> None:
		assert dg is not None
		self._builder = _DG_build_orig(dg,
			None if onNewVertex is None else _funcWrap(libpymod._Func_VoidDGVertex, onNewVertex),
			None if onNewHyperEdge is None else _funcWrap(libpymod._Func_VoidDGHyperEdge, onNewHyperEdge)
		)

	def __enter__(self) -> "DGBuildContextManager":
		return self

	def __exit__(self, exc_type, exc_val, exc_tb) -> None:
		del self._builder
		self._builder = None

	@property
	def dg(self) -> DG:
		assert self._builder
		return self._builder.dg

	@property
	def isActive(self) -> bool:
		assert self._builder
		return self._builder.isActive

	def addDerivation(self, d: Derivations,
			graphPolicy: IsomorphismPolicy = IsomorphismPolicy.Check) -> DG.HyperEdge:
		assert self._builder
		return self._builder.addDerivation(d, graphPolicy)

	def addHyperEdge(self, e: DG.HyperEdge,
			graphPolicy: IsomorphismPolicy = IsomorphismPolicy.Check) -> DG.HyperEdge:
		assert self._builder
		return self._builder.addHyperEdge(e, graphPolicy)

	def execute(self, strategy: DGStrat, *, verbosity: int=2, ignoreRuleLabelTypes: bool=False) -> DG.Builder.ExecuteResult:
		assert self._builder
		return self._builder.execute(dgStrat(strategy), verbosity, ignoreRuleLabelTypes)  # type: ignore
	
	def apply(self, graphs: Iterable[Graph], rule: Rule, onlyProper: bool=True, verbosity: int=0,
			graphPolicy: IsomorphismPolicy=IsomorphismPolicy.Check) -> List[DG.HyperEdge]:
		assert self._builder
		return _unwrap(self._builder.apply(_wrap(libpymod._VecGraph, graphs), rule, onlyProper, verbosity, graphPolicy))

	def addAbstract(self, description: str) -> DG.Builder.AddAbstractResult:
		assert self._builder
		return self._builder.addAbstract(description)

	def load(self, ruleDatabase: List[Rule], f: str, verbosity: int = 2) -> None:
		assert self._builder
		return self._builder.load(
			_wrap(libpymod._VecRule, ruleDatabase),
			prefixFilename(f), verbosity)

_DG_build_orig = DG.build
DG.build = lambda self, *, onNewVertex=None, onNewHyperEdge=None: DGBuildContextManager(self, onNewVertex, onNewHyperEdge)  # type: ignore


#----------------------------------------------------------
# DG.Builder.ExecuteResult
#----------------------------------------------------------

def _DGExecuteResult__getattribute__(self: DG.Builder.ExecuteResult, name: str) -> Any:
	if name in ("subset", "universe"):
		return _unwrap(object.__getattribute__(self, name))
	return object.__getattribute__(self, name)

DG.Builder.ExecuteResult.__getattribute__ = _DGExecuteResult__getattribute__  # type: ignore

_DGExecuteResult_list_orig = DG.Builder.ExecuteResult.list
def _DGExecuteResult_list(self: DG.Builder.ExecuteResult, *, withUniverse: bool=False) -> None:
	return _DGExecuteResult_list_orig(self, withUniverse)  # type: ignore
DG.Builder.ExecuteResult.list = _DGExecuteResult_list  # type: ignore


#----------------------------------------------------------
# DG.HyperEdge
#----------------------------------------------------------

_DGHyperEdge_print_orig = DG.HyperEdge.print
DG.HyperEdge.print = lambda self, *args, **kwargs: _unwrap(_DGHyperEdge_print_orig(self, *args, **kwargs))  # type: ignore


#----------------------------------------------------------
# DGPrinter
#----------------------------------------------------------

def _makeGraphToVertexCallback(orig, name, func):
	def callback(self, f, *args, **kwargs):
		if hasattr(f, "__call__"):
			import inspect
			spec = inspect.getfullargspec(f)
			if len(spec.args) == 2:
				_deprecation("The callback for {} seems to take two arguments, a graph and a derivation graph. This is deprecated, the callback should take a single DG.Vertex argument.".format(name))
				fOrig = f
				f = lambda v, fOrig=fOrig: fOrig(v.graph, v.dg)  # noqa
		return orig(self, _funcWrap(func, f), *args, **kwargs)
	return callback

_DGPrinter_pushVertexVisible_orig = DGPrinter.pushVertexVisible
DGPrinter.pushVertexVisible = _makeGraphToVertexCallback(  # type: ignore
	_DGPrinter_pushVertexVisible_orig, "pushVertexVisible", libpymod._Func_BoolDGVertex)

_DGPrinter_pushEdgeVisible_orig = DGPrinter.pushEdgeVisible
DGPrinter.pushEdgeVisible = (  # type: ignore
	lambda self, f: _DGPrinter_pushEdgeVisible_orig(self, _funcWrap(libpymod._Func_BoolDGHyperEdge, f)))

_DGPrinter_pushVertexLabel_orig = DGPrinter.pushVertexLabel
DGPrinter.pushVertexLabel = _makeGraphToVertexCallback(  # type: ignore
	_DGPrinter_pushVertexLabel_orig , "pushVertexLabel", libpymod._Func_StringDGVertex)

_DGPrinter_pushEdgeLabel_orig = DGPrinter.pushEdgeLabel
DGPrinter.pushEdgeLabel = (  # type: ignore
	lambda self, f: _DGPrinter_pushEdgeLabel_orig(self, _funcWrap(libpymod._Func_StringDGHyperEdge, f)))

_DGPrinter_pushVertexColour_orig = DGPrinter.pushVertexColour
_DGPrinter_pushVertexColour_inner = _makeGraphToVertexCallback(
	_DGPrinter_pushVertexColour_orig, "pushVertexColour", libpymod._Func_StringDGVertex)
DGPrinter.pushVertexColour = (  # type: ignore
	lambda self, f, extendToEdges=True: _DGPrinter_pushVertexColour_inner(self, f, extendToEdges))

_DGPrinter_pushEdgeColour_orig = DGPrinter.pushEdgeColour
DGPrinter.pushEdgeColour = (  # type: ignore
	lambda self, f: _DGPrinter_pushEdgeColour_orig(self, _funcWrap(libpymod._Func_StringDGHyperEdge, f)))

_DGPrinter_setRotationOverwrite_orig = DGPrinter.setRotationOverwrite
DGPrinter.setRotationOverwrite = (  # type: ignore
	lambda self, f: _DGPrinter_setRotationOverwrite_orig(self, _funcWrap(libpymod._Func_IntGraph, f)))

_DGPrinter_setMirrorOverwrite_orig = DGPrinter.setMirrorOverwrite
DGPrinter.setMirrorOverwrite = (  # type: ignore
	lambda self, f: _DGPrinter_setMirrorOverwrite_orig(self, _funcWrap(libpymod._Func_BoolGraph, f)))

_DGPrinter_setImageOverwrite_orig = DGPrinter.setImageOverwrite
def _DGPrinter_setImageOverwrite(self, f):
	if f is None:
		wrapped = None
	else:
		wrapped = _funcWrap(
			libpymod._Func_PairStringStringDGVertexInt, f)
	return _DGPrinter_setImageOverwrite_orig(self, wrapped)
DGPrinter.setImageOverwrite = _DGPrinter_setImageOverwrite  # type: ignore


#----------------------------------------------------------
# Strategy
#----------------------------------------------------------

def _DGStratGraphState__getattribute__(self: DGStrat.GraphState, name: str) -> Any:
	if name == "subset":
		return _unwrap(self._subset)  # type: ignore
	elif name == "universe":
		return _unwrap(self._universe)  # type: ignore
	else:
		return object.__getattribute__(self, name)
DGStrat.GraphState.__getattribute__ = _DGStratGraphState__getattribute__  # type: ignore

_DGStrat_makeAddStatic_orig = DGStrat.makeAddStatic
def _DGStrat_makeAddStatic(onlyUniverse: bool, graphs: Iterable[Graph], graphPolicy: IsomorphismPolicy) -> DGStrat:
	return _DGStrat_makeAddStatic_orig(onlyUniverse, _wrap(libpymod._VecGraph, graphs), graphPolicy)
DGStrat.makeAddStatic = _DGStrat_makeAddStatic  # type: ignore

_DGStrat_makeAddDynamic_orig = DGStrat.makeAddDynamic
def _DGStrat_makeAddDynamic(onlyUniverse: bool, generator: Callable[[], List[Graph]], graphPolicy: IsomorphismPolicy) -> DGStrat:
	return _DGStrat_makeAddDynamic_orig(onlyUniverse, _funcWrap(libpymod._Func_VecGraph, generator, resultWrap=libpymod._VecGraph), graphPolicy)
DGStrat.makeAddDynamic = _DGStrat_makeAddDynamic  # type: ignore

_DGStrat_makeSequence_orig = DGStrat.makeSequence
def _DGStrat_makeSequence(l: Iterable[DGStrat]) -> DGStrat:
	return _DGStrat_makeSequence_orig(_wrap(libpymod._VecDGStrat, l))
DGStrat.makeSequence = _DGStrat_makeSequence  # type: ignore

_DGStrat_makeParallel_orig = DGStrat.makeParallel
def _DGStrat_makeParallel(l: Iterable[DGStrat]) -> DGStrat:
	return _DGStrat_makeParallel_orig(_wrap(libpymod._VecDGStrat, l))
DGStrat.makeParallel = _DGStrat_makeParallel  # type: ignore

_DGStrat_makeFilter_orig = DGStrat.makeFilter
def _DGStrat_makeFilter(alsoUniverse: bool, filterFunc: Callable[[Graph, DGStrat.GraphState, bool], bool]) -> DGStrat:
	return _DGStrat_makeFilter_orig(alsoUniverse, _funcWrap(libpymod._Func_BoolGraphDGStratGraphStateBool, filterFunc))
DGStrat.makeFilter = _DGStrat_makeFilter  # type: ignore

_DGStrat_makeExecute_orig = DGStrat.makeExecute
def _DGStrat_makeExecute(func: Callable[[DGStrat.GraphState], None]) -> DGStrat:
	return _DGStrat_makeExecute_orig(_funcWrap(libpymod._Func_VoidDGStratGraphState, func))
DGStrat.makeExecute = _DGStrat_makeExecute  # type: ignore

_DGStrat_makeLeftPredicate_orig = DGStrat.makeLeftPredicate
def _DGStrat_makeLeftPredicate(pred: Callable[[Derivation], bool], strat: DGStrat) -> DGStrat:
	return _DGStrat_makeLeftPredicate_orig(_funcWrap(libpymod._Func_BoolDerivation, pred), strat)
DGStrat.makeLeftPredicate = _DGStrat_makeLeftPredicate  # type: ignore

_DGStrat_makeRightPredicate_orig = DGStrat.makeRightPredicate
def _DGStrat_makeRightPredicate(pred: Callable[[Derivation], bool], strat: DGStrat) -> DGStrat:
	return _DGStrat_makeRightPredicate_orig(_funcWrap(libpymod._Func_BoolDerivation, pred), strat)
DGStrat.makeRightPredicate = _DGStrat_makeRightPredicate  # type: ignore


#----------------------------------------------------------
# DG Strategy Prettification
#----------------------------------------------------------

_DGStratType = Union[DGStrat, Rule, "_DGStrat_sequenceProxy", Iterable['_DGStratType']]

def dgStrat(s: _DGStratType) -> DGStrat:
	if isinstance(s, DGStrat):
		return s
	elif isinstance(s, Rule):
		return DGStrat.makeRule(s)
	elif isinstance(s, _DGStrat_sequenceProxy):
		return DGStrat.makeSequence(s.strats)
	elif isinstance(s, collections.abc.Iterable):
		# do deep dgStrat
		l = [dgStrat(a) for a in s]
		return DGStrat.makeParallel(l)	
	else:
		raise TypeError("Can not convert type '" + str(type(s)) + "' to DGStrat.")

# add
#----------------------------------------------------------

_DGStratAddStaticGraphType = Union[Graph, Iterable[Graph]]

def _DGStrat_add(doUniverse: bool, g: Union[_DGStratAddStaticGraphType, Callable[[], List[Graph]]],
		gs: Tuple[_DGStratAddStaticGraphType, ...], graphPolicy: IsomorphismPolicy) -> DGStrat:
	if hasattr(g, "__call__"): # assume the dynamic version is meant
		if len(gs) > 0:
			raise TypeError("The dynamic version of addSubset/addUniverse takes exactly 1 argument (" + str(len(gs) + 1) + " given).")
		return DGStrat.makeAddDynamic(doUniverse, cast(Callable[[], List[Graph]], g), graphPolicy)
	else: # assume the static version was meant
		def convertGraphs(graphs: List[Graph], g: Union[Graph, Iterable[Graph]]) -> None:
			if isinstance(g, Graph):
				graphs.append(g)
			else:
				graphs.extend(a for a in g)
		graphs = []  # type: List[Graph]
		convertGraphs(graphs, cast(Union[Graph, Iterable[Graph]], g))
		for a in gs:
			convertGraphs(graphs, a)
		return DGStrat.makeAddStatic(doUniverse, graphs, graphPolicy)
def addUniverse(g: _DGStratAddStaticGraphType , *gs: _DGStratAddStaticGraphType,
		graphPolicy: IsomorphismPolicy=IsomorphismPolicy.Check) -> DGStrat:
	return _DGStrat_add(True, g, gs, graphPolicy)
def addSubset(g: _DGStratAddStaticGraphType , *gs: _DGStratAddStaticGraphType,
		graphPolicy: IsomorphismPolicy=IsomorphismPolicy.Check) -> DGStrat:
	return _DGStrat_add(False, g, gs, graphPolicy)

# derivation predicates
#----------------------------------------------------------

class _DGStrat_DerivationPredicateProxyPredicateHolder:
	def __init__(self, isLeft: bool, predicate: Callable[[Derivation], bool]) -> None:
		self.isLeft = isLeft
		self.predicate = predicate
	def __call__(self, strat: DGStrat) -> DGStrat:
		if self.isLeft:
			return DGStrat.makeLeftPredicate(self.predicate, dgStrat(strat))
		else:
			return DGStrat.makeRightPredicate(self.predicate, dgStrat(strat))

class _DGStrat_DerivationPredicateProxy:
	def __init__(self, isLeft: bool):
		self.isLeft = isLeft
	def __getitem__(self, predicate: Callable[[Derivation], bool]) -> _DGStrat_DerivationPredicateProxyPredicateHolder:
		return _DGStrat_DerivationPredicateProxyPredicateHolder(self.isLeft, predicate)

leftPredicate = _DGStrat_DerivationPredicateProxy(True)
rightPredicate = _DGStrat_DerivationPredicateProxy(False)

# execute
#----------------------------------------------------------

def execute(func):
	return DGStrat.makeExecute(func)

# filter
#----------------------------------------------------------

class _DGStrat_FilterProxy:
	def __init__(self, alsoUniverse: bool) -> None:
		self.alsoUniverse = alsoUniverse
	def __call__(self, filterFunc: Callable[[Graph, DGStrat.GraphState, bool], bool]) -> DGStrat:
		return DGStrat.makeFilter(self.alsoUniverse, filterFunc)

filterUniverse = _DGStrat_FilterProxy(True)
filterSubset = _DGStrat_FilterProxy(False)

# repeat
#----------------------------------------------------------

class _DGStrat_RepeatProxyBoundHolder:
	def __init__(self, bound: int) -> None:
		self.bound = bound
		if bound < 0:
			raise ArgumentError("The number of repetitions in a repeat strategy must be non-negative. Got '" + str(bound))  # type: ignore
	def __call__(self, strat: DGStrat) -> DGStrat:
		return DGStrat.makeRepeat(self.bound, dgStrat(strat))

class _DGStrat_RepeatProxy:
	def __getitem__(self, bound: int) -> _DGStrat_RepeatProxyBoundHolder:
		return _DGStrat_RepeatProxyBoundHolder(bound)
	def __call__(self, strat: DGStrat) -> DGStrat:
		return self[2**31 - 1](strat)
		
repeat = _DGStrat_RepeatProxy()

# revive
#----------------------------------------------------------

def revive(s):
	return DGStrat.makeRevive(dgStrat(s))

# sequence
#----------------------------------------------------------

def _DGStrat_sequence__rshift__(a: "_DGStrat_sequenceProxy", b: DGStrat) -> "_DGStrat_sequenceProxy":
	strats = []  # type: List[DGStrat]
	if isinstance(a, _DGStrat_sequenceProxy):
		strats.extend(s for s in a.strats)
	else:
		strats.append(dgStrat(a))
	if isinstance(b, _DGStrat_sequenceProxy):
		strats.extend(s for s in b.strats)
	else:
		strats.append(dgStrat(b))
	return _DGStrat_sequenceProxy(strats)


class _DGStrat_sequenceProxy:
	def __init__(self, strats: List[DGStrat]) -> None:
		self.strats = strats

	def  __rshift__(self, other: DGStrat) -> "_DGStrat_sequenceProxy":
		return _DGStrat_sequence__rshift__(self, other)

DGStrat.__rshift__ = _DGStrat_sequence__rshift__  # type: ignore
DGStrat.__rrshift__ = lambda self, other: _DGStrat_sequence__rshift__(other, self)  # type: ignore
Rule.__rshift__ = _DGStrat_sequence__rshift__  # type: ignore


###########################################################
# Graph
###########################################################

inputGraphs = []

def _Graph__getattribute__(self, name):
	if name == "loadingWarnings":
		return _unwrap(object.__getattribute__(self, name))
	else:
		return object.__getattribute__(self, name)
Graph.__getattribute__ = _Graph__getattribute__  # type: ignore

_Graph_print_orig = Graph.print
def _Graph_print(self: Graph, first: Optional[GraphPrinter]=None, second: Optional[GraphPrinter]=None) -> Tuple[str, str]:
	if first is None:
		return _Graph_print_orig(self)
	if second is None:
		second = first
	return _Graph_print_orig(self, first, second)
Graph.print = _Graph_print  # type: ignore

_Graph_aut = Graph.aut
Graph.aut = lambda self, labelSettings=_lsString: _Graph_aut(self, labelSettings)  # type: ignore

_Graph_isomorphism = Graph.isomorphism
Graph.isomorphism = lambda self, g, maxNumMatches=1, labelSettings=_lsString: _Graph_isomorphism(self, g, maxNumMatches, labelSettings)  # type: ignore

_Graph_monomorphism = Graph.monomorphism
Graph.monomorphism = lambda self, g, maxNumMatches=1, labelSettings=_lsString: _Graph_monomorphism(self, g, maxNumMatches, labelSettings)  # type: ignore

_Graph_enumerateIsomorphisms = Graph.enumerateIsomorphisms  # type: ignore
Graph.enumerateIsomorphisms = lambda self, codomain, *, callback, labelSettings=_lsString: _Graph_enumerateIsomorphisms(  # type: ignore
	self, codomain, _funcWrap(libpymod._Func_BoolVertexMapGraphGraph, callback), labelSettings)  # type: ignore

_Graph_enumerateMonomorphisms = Graph.enumerateMonomorphisms  # type: ignore
Graph.enumerateMonomorphisms = lambda self, codomain, *, callback, labelSettings=_lsString: _Graph_enumerateMonomorphisms(  # type: ignore
	self, codomain, _funcWrap(libpymod._Func_BoolVertexMapGraphGraph, callback), labelSettings)  # type: ignore

_Graph_getGMLString = Graph.getGMLString
Graph.getGMLString = lambda self, withCoords=False: _Graph_getGMLString(self, withCoords)  # type: ignore
_Graph_printGML = Graph.printGML
Graph.printGML = lambda self, withCoords=False: _Graph_printGML(self, withCoords)  # type: ignore

# Loading
###########################################################

def _graphLoad(a: Graph, name: Optional[str], add: bool) -> Graph:
	if name is not None:
		a.name = name
	if add:
		inputGraphs.append(a)
	return a

def _graphsLoad(gs: List[Graph], add: bool) -> List[Graph]:
	us = _unwrap(gs)
	res = [_graphLoad(a, name=None, add=add) for a in us]
	return res

def _graphssLoad(gs: List[List[Graph]], add: bool) -> List[List[Graph]]:
	us = _unwrap(gs)
	res = [_graphsLoad(a, add=add) for a in us]
	return res

_Graph_fromGMLString_orig      = Graph.fromGMLString
_Graph_fromGMLFile_orig        = Graph.fromGMLFile
_Graph_fromGMLStringMulti_orig = Graph.fromGMLStringMulti
_Graph_fromGMLFileMulti_orig   = Graph.fromGMLFileMulti
_Graph_fromDFS_orig            = Graph.fromDFS
_Graph_fromDFSMulti_orig       = Graph.fromDFSMulti
_Graph_fromSMILES_orig         = Graph.fromSMILES
_Graph_fromSMILESMulti_orig    = Graph.fromSMILESMulti
_Graph_fromMOLString_orig      = Graph.fromMOLString
_Graph_fromMOLFile_orig        = Graph.fromMOLFile
_Graph_fromMOLStringMulti_orig = Graph.fromMOLStringMulti
_Graph_fromMOLFileMulti_orig   = Graph.fromMOLFileMulti
_Graph_fromSDString_orig       = Graph.fromSDString
_Graph_fromSDFile_orig         = Graph.fromSDFile
_Graph_fromSDStringMulti_orig  = Graph.fromSDStringMulti
_Graph_fromSDFileMulti_orig    = Graph.fromSDFileMulti

def _Graph_fromGMLString(     s: str, name: Optional[str] = None,                                     add: bool = True, printStereoWarnings: bool = True) -> Graph:
	return _graphLoad(_Graph_fromGMLString_orig(                   s         , printStereoWarnings), name, add)
def _Graph_fromGMLFile(       f: str, name: Optional[str] = None,                                     add: bool = True, printStereoWarnings: bool = True) -> Graph:
	return _graphLoad(_Graph_fromGMLFile_orig(      prefixFilename(f)        , printStereoWarnings), name, add)
def _Graph_fromGMLStringMulti(s: str,                                                                 add: bool = True, printStereoWarnings: bool = True) -> List[Graph]:
	return _graphsLoad(_Graph_fromGMLStringMulti_orig(             s         , printStereoWarnings),       add)
def _Graph_fromGMLFileMulti(  f: str,                                                                 add: bool = True, printStereoWarnings: bool = True) -> List[Graph]:
	return _graphsLoad(_Graph_fromGMLFileMulti_orig(prefixFilename(f)        , printStereoWarnings),       add)
def _Graph_fromDFS(           s: str, name: Optional[str] = None,                                     add: bool = True) -> Graph:
	return _graphLoad(_Graph_fromDFS_orig(                         s                              ), name, add)
def _Graph_fromDFSMulti(      s: str,                                                                 add: bool = True) -> List[Graph]:
	return _graphsLoad(_Graph_fromDFSMulti_orig(                   s                              ),       add)
def _Graph_fromSMILES(        s: str, name: Optional[str] = None, allowAbstract: bool = False, classPolicy: SmilesClassPolicy = SmilesClassPolicy.NoneOnDuplicate,
                                                                                                      add: bool = True, printStereoWarnings: bool = True) -> Graph:
	return _graphLoad(_Graph_fromSMILES_orig(                      s, allowAbstract, classPolicy, printStereoWarnings), name, add)
def _Graph_fromSMILESMulti(   s: str,                             allowAbstract: bool = False, classPolicy: SmilesClassPolicy = SmilesClassPolicy.NoneOnDuplicate,
                                                                                                      add: bool = True, printStereoWarnings: bool = True) -> List[Graph]:
	return _graphsLoad(_Graph_fromSMILESMulti_orig(                s, allowAbstract, classPolicy, printStereoWarnings),       add)
def _Graph_fromMOLString(     s: str, name: Optional[str] = None, options: MDLOptions = MDLOptions(), add: bool = True) -> Graph:
	return _graphLoad(_Graph_fromMOLString_orig(                   s,  options                    ), name, add)
def _Graph_fromMOLFile(       f: str, name: Optional[str] = None, options: MDLOptions = MDLOptions(), add: bool = True) -> Graph:
	return _graphLoad(_Graph_fromMOLFile_orig(      prefixFilename(f), options                    ), name, add)
def _Graph_fromMOLStringMulti(s: str,                             options: MDLOptions = MDLOptions(), add: bool = True) -> List[Graph]:
	return _graphsLoad(_Graph_fromMOLStringMulti_orig(             s,  options                    ),       add)
def _Graph_fromMOLFileMulti(  f: str,                             options: MDLOptions = MDLOptions(), add: bool = True) -> List[Graph]:
	return _graphsLoad(_Graph_fromMOLFileMulti_orig(prefixFilename(f), options                    ),       add)
def _Graph_fromSDString(      s: str,                             options: MDLOptions = MDLOptions(), add: bool = True) -> List[Graph]:
	return _graphsLoad(_Graph_fromSDString_orig(                   s,  options                    ),       add)
def _Graph_fromSDFile(        f: str,                             options: MDLOptions = MDLOptions(), add: bool = True) -> List[Graph]:
	return _graphsLoad(_Graph_fromSDFile_orig(      prefixFilename(f), options                    ),       add)
def _Graph_fromSDStringMulti( s: str,                             options: MDLOptions = MDLOptions(), add: bool = True) -> List[List[Graph]]:
	return _graphssLoad(_Graph_fromSDStringMulti_orig(             s,  options                    ),       add)
def _Graph_fromSDFileMulti(   f: str,                             options: MDLOptions = MDLOptions(), add: bool = True) -> List[List[Graph]]:
	return _graphssLoad(_Graph_fromSDFileMulti_orig(prefixFilename(f), options                    ),       add)

Graph.fromGMLString      = _Graph_fromGMLString  # type: ignore
Graph.fromGMLFile        = _Graph_fromGMLFile  # type: ignore
Graph.fromGMLStringMulti = _Graph_fromGMLStringMulti  # type: ignore
Graph.fromGMLFileMulti   = _Graph_fromGMLFileMulti  # type: ignore
Graph.fromDFS            = _Graph_fromDFS  # type: ignore
Graph.fromDFSMulti       = _Graph_fromDFSMulti  # type: ignore
Graph.fromSMILES         = _Graph_fromSMILES  # type: ignore
Graph.fromSMILESMulti    = _Graph_fromSMILESMulti  # type: ignore
Graph.fromMOLString      = _Graph_fromMOLString  # type: ignore
Graph.fromMOLFile        = _Graph_fromMOLFile  # type: ignore
Graph.fromMOLStringMulti = _Graph_fromMOLStringMulti  # type: ignore
Graph.fromMOLFileMulti   = _Graph_fromMOLFileMulti  # type: ignore
Graph.fromSDString       = _Graph_fromSDString  # type: ignore
Graph.fromSDFile         = _Graph_fromSDFile  # type: ignore
Graph.fromSDStringMulti  = _Graph_fromSDStringMulti  # type: ignore
Graph.fromSDFileMulti    = _Graph_fromSDFileMulti  # type: ignore

def makeDepGraphLoad(fNew, nOld, nNew):
	def f(*args, **kwargs):
		_deprecation(f"{nOld} is deprecated, use {nNew} instead.")
		return fNew(*args, **kwargs)
	return f

graphGMLString = makeDepGraphLoad(Graph.fromGMLString, "graphGMLString", "Graph.fromGMLString")
graphGML       = makeDepGraphLoad(Graph.fromGMLFile, "graphGML", "Graph.fromGMLFile")
graphDFS       = makeDepGraphLoad(Graph.fromDFS, "graphDFS", "Graph.fromDFS")
smiles         = makeDepGraphLoad(Graph.fromSMILES, "smiles", "Graph.fromSMILES")

###########################################################

_setSpecialForIdClass(Graph)

def _Graph__setattr__(self: Graph, name: str, value: Any) -> None:
	if name == "image":
		object.__setattr__(self, "image", _funcWrap(libpymod._Func_String, value))
	else:
		_NoNew__setattr__(self, name, value)
Graph.__setattr__ = _Graph__setattr__  # type: ignore


###########################################################
# Post
###########################################################

def _post_command(self, cmd: str) -> None:
	_deprecation("'post(cmd)' is deprecated. Use 'post.command(cmd)'.")
	return post.command(cmd)
post.__init__ = _post_command  # type: ignore

def postFlush() -> None:
	_deprecation("'postFlush()' is deprecated. Use 'post.flushCommands()'.")
	return post.flushCommands()
def postDisable() -> None:
	_deprecation("'postDisable()' is deprecated. Use 'post.disableCommands()'.")
	return post.disableCommands()
def postEnable() -> None:
	_deprecation("'postEnable()' is deprecated. Use 'post.enableCommands()'.")
	return post.enableCommands()
def postReset() -> None:
	_deprecation("'postReset()' is deprecated. Use 'post.reopenCommandFile()'.")
	return post.reopenCommandFile()

def postChapter(heading: str) -> None:
	_deprecation("'postChapter(heading)' is deprecated. Use 'post.summaryChapter(heading)'.")
	post.summaryChapter(heading)
def postSection(heading: str) -> None:
	_deprecation("'postSection(heading)' is deprecated. Use 'post.summarySection(heading)'.")
	post.summarySection(heading)


###########################################################
# Rule
###########################################################

inputRules = []

_Rule_print_orig = Rule.print
def _Rule_print(self: Rule, first: Optional[GraphPrinter]=None, second: Optional[GraphPrinter]=None,
		printCombined: bool=False) -> Tuple[str, str]:
	if first is None:
		return _Rule_print_orig(self, printCombined)
	if second is None:
		second = first
	return _Rule_print_orig(self, first, second, printCombined)
Rule.print = _Rule_print  # type: ignore

_Rule_isomorphism = Rule.isomorphism
Rule.isomorphism = lambda self, r, maxNumMatches=1, labelSettings=_lsString: _Rule_isomorphism(self, r, maxNumMatches, labelSettings)  # type: ignore
_Rule_monomorphism = Rule.monomorphism
Rule.monomorphism = lambda self, r, maxNumMatches=1, labelSettings=_lsString: _Rule_monomorphism(self, r, maxNumMatches, labelSettings)  # type: ignore

_Rule_getGMLString = Rule.getGMLString
Rule.getGMLString = lambda self, withCoords=False: _Rule_getGMLString(self, withCoords)  # type: ignore
_Rule_printGML = Rule.printGML
Rule.printGML = lambda self, withCoords=False: _Rule_printGML(self, withCoords)  # type: ignore

def _ruleLoad(a: Rule, name: Optional[str], add: bool) -> Rule:
	if name is not None:
		a.name = name
	if add:
		inputRules.append(a)
	return a

_Rule_fromGMLString_orig = Rule.fromGMLString
_Rule_fromGMLFile_orig   = Rule.fromGMLFile
_Rule_fromDFS_orig       = Rule.fromDFS

def _Rule_fromGMLString(s: str, name=None, *, invert: bool=False, add: bool=True, printStereoWarnings: bool=True) -> Rule:
	return _ruleLoad(_Rule_fromGMLString_orig(s, invert, printStereoWarnings), name, add)
def _Rule_fromGMLFile(f: str, name=None, *, invert: bool=False, add: bool=True, printStereoWarnings: bool=True) -> Rule:
	return _ruleLoad(_Rule_fromGMLFile_orig(prefixFilename(f), invert, printStereoWarnings), name, add)
def _Rule_fromDFS(s: str, name=None, *, invert: bool=False, add: bool=True) -> Rule:
	return _ruleLoad(_Rule_fromDFS_orig(s, invert), name, add)

Rule.fromGMLString = _Rule_fromGMLString  # type: ignore
Rule.fromGMLFile   = _Rule_fromGMLFile  # type: ignore
Rule.fromDFS       = _Rule_fromDFS  # type: ignore

def makeDepRuleLoad(fNew, nOld, nNew):
	def f(*args, **kwargs):
		_deprecation(f"{nOld} is deprecated, use {nNew} instead.")
		return fNew(*args, **kwargs)
	return f

ruleGMLString = makeDepRuleLoad(Rule.fromGMLString, "ruleGMLString", "Rule.fromGMLString")
ruleGML       = makeDepRuleLoad(Rule.fromGMLFile, "ruleGML", "Rule.fromGMLFile")

_setSpecialForIdClass(Rule)


#----------------------------------------------------------
# Composition
#----------------------------------------------------------

_RCEvaluator__init__old = RCEvaluator.__init__
def _RCEvaluator__init__(self: RCEvaluator, ruleDatabase: Iterable[Rule], labelSettings: LabelSettings=_lsString) -> None:
	return _RCEvaluator__init__old(self, _wrap(libpymod._VecRule, ruleDatabase), labelSettings)
RCEvaluator.__init__ = _RCEvaluator__init__  # type: ignore

def _RCEvaluator__getattribute__(self: RCEvaluator, name: str) -> Any:
	if name == "ruleDatabase":
		return _unwrap(self._ruleDatabase)  # type: ignore
	elif name == "createdRules":
		return _unwrap(self._createdRules)  # type: ignore
	else:
		return object.__getattribute__(self, name)
RCEvaluator.__getattribute__ = _RCEvaluator__getattribute__  # type: ignore

_RCEvaluator_eval = RCEvaluator.eval
RCEvaluator.eval = lambda self, exp, *, onlyUnique=True, verbosity=0: _unwrap(_RCEvaluator_eval(self, exp, onlyUnique, verbosity))  # type: ignore

def rcEvaluator(rules: Iterable[Rule], labelSettings: LabelSettings=_lsString) -> RCEvaluator:
	_deprecation("The rcEvaluator() function is deprecated, use RCEvaluator() instead.")
	return RCEvaluator(rules, labelSettings)


#----------------------------------------------------------
# RCMatch
#----------------------------------------------------------

_RCMatch_compose_orig = RCMatch.compose
def _RCMatch_compose(self, *, verbose=False):
	return _RCMatch_compose_orig(self, verbose)
RCMatch.compose = _RCMatch_compose  # type: ignore

_RCMatch_composeWithMaps_orig = RCMatch.composeWithMaps
def _RCMatch_composeWithMaps(self, *, verbose=False):
	return _RCMatch_composeWithMaps_orig(self, verbose)
RCMatch.composeWithMaps = _RCMatch_composeWithMaps  # type: ignore


_RCMatch_composeAll_orig = RCMatch.composeAll
def _RCMatch_composeAll(self, *, maximum=False, verbose=False):
	return _unwrap(_RCMatch_composeAll_orig(self, maximum, verbose))
RCMatch.composeAll = _RCMatch_composeAll  # type: ignore


_RCMatch_composeAllWithMaps_orig = RCMatch.composeAllWithMaps
def _RCMatch_composeAllWithMaps(self, *, maximum=False, verbose=False):
	return _unwrap(_RCMatch_composeAllWithMaps_orig(self, maximum, verbose))
RCMatch.composeAllWithMaps = _RCMatch_composeAllWithMaps  # type: ignore


#----------------------------------------------------------
# RCExp prettification
#----------------------------------------------------------

_rcExpType = Union[RCExpExp, RCExpBind, RCExpComposeCommon, RCExpComposeParallel, RCExpComposeSub, RCExpComposeSuper, Iterable["_rcExpType"]]

def rcExp(e: _rcExpType) -> RCExpExp:
	if isinstance(e, RCExpExp) or isinstance(e, Rule) or isinstance(e, RCExpUnion):
		return e
	elif isinstance(e, RCExpBind) or isinstance(e, RCExpId) or isinstance(e, RCExpUnbind):
		return e
	elif isinstance(e, RCExpComposeCommon) or isinstance(e, RCExpComposeParallel) or isinstance(e, RCExpComposeSub) or isinstance(e, RCExpComposeSuper):
		return e
	elif isinstance(e, collections.abc.Iterable):
		return RCExpUnion(_wrap(libpymod._VecRCExpExp, [rcExp(a) for a in e]))
	else:
		raise TypeError("Can not convert type '" + str(type(e)) + "' to RCExpExp")

_GraphOrGraphs = Union[Graph, Iterable[Graph]]

def _rcConvertGraph(g: _GraphOrGraphs, cls: Type[Union[RCExpBind, RCExpId, RCExpUnbind]], f: Callable[[Graph], RCExpExp]) -> RCExpExp:
	if isinstance(g, Graph):
		return cls(g)
	elif isinstance(g, collections.abc.Iterable):
		l = [f(a) for a in g]
		return rcExp(l)
	else:
		raise TypeError("Can not convert type '" + str(type(g)) + "' to " + str(cls))


def rcBind(g: _GraphOrGraphs) -> RCExpExp:
	return _rcConvertGraph(g, RCExpBind, rcBind)


def rcId(g: _GraphOrGraphs) -> RCExpExp:
	return _rcConvertGraph(g, RCExpId, rcId)


def rcUnbind(g: _GraphOrGraphs) -> RCExpExp:
	return _rcConvertGraph(g, RCExpUnbind, rcUnbind)


class _RCCommonOpFirstBound:
	def __init__(self, maximum: bool, connected: bool, includeEmpty: bool, first: RCExpExp) -> None:
		self.maximum = maximum
		self.connected = connected
		self.includeEmpty = includeEmpty
		self.first = first

	def __mul__(self, second: RCExpExp) -> RCExpExp:
		return RCExpComposeCommon(rcExp(self.first), rcExp(second), self.maximum, self.connected, self.includeEmpty)


class _RCCommonOpArgsBound:
	def __init__(self, maximum: bool, connected: bool, includeEmpty: bool) -> None:
		self.maximum = maximum
		self.connected = connected
		self.includeEmpty = includeEmpty

	def __rmul__(self, first: RCExpExp) -> _RCCommonOpFirstBound:
		return _RCCommonOpFirstBound(self.maximum, self.connected, self.includeEmpty, first)


class _RCCommonOp:
	def __call__(self, maximum: bool=False, connected: bool=True, includeEmpty: bool=False) -> _RCCommonOpArgsBound:
		return _RCCommonOpArgsBound(maximum, connected, includeEmpty)

	def __rmul__(self, first: RCExpExp) -> _RCCommonOpFirstBound:
		return first * self()


rcCommon = _RCCommonOp()


class _RCParallelOpFirstBound:
	def __init__(self, first: RCExpExp) -> None:
		self.first = first

	def __mul__(self, second: RCExpExp) -> RCExpExp:
		return RCExpComposeParallel(rcExp(self.first), rcExp(second))


class _RCParallelOp:
	def __rmul__(self, first: RCExpExp) -> _RCParallelOpFirstBound:
		return _RCParallelOpFirstBound(first)


rcParallel = _RCParallelOp()


class _RCSubOpFirstBound:
	def __init__(self, allowPartial: bool, first: RCExpExp) -> None:
		self.allowPartial = allowPartial
		self.first = first

	def __mul__(self, second: RCExpExp) -> RCExpExp:
		return RCExpComposeSub(rcExp(self.first), rcExp(second), self.allowPartial)


class _RCSubOpArgsBound:
	def __init__(self, allowPartial: bool) -> None:
		self.allowPartial = allowPartial

	def __rmul__(self, first: RCExpExp) -> _RCSubOpFirstBound:
		return _RCSubOpFirstBound(self.allowPartial, first)


class _RCSubOp:
	def __call__(self, allowPartial: bool=True) -> _RCSubOpArgsBound:
		return _RCSubOpArgsBound(allowPartial)

	def __rmul__(self, first: RCExpExp) -> _RCSubOpFirstBound:
		return first * self()


rcSub = _RCSubOp()


class _RCSuperOpFirstBound:
	def __init__(self, allowPartial: bool, enforceConstraints: bool, first: RCExpExp) -> None:
		self.allowPartial = allowPartial
		self.enforceConstraints = enforceConstraints
		self.first = first

	def __mul__(self, second: RCExpExp) -> RCExpExp:
		return RCExpComposeSuper(rcExp(self.first), rcExp(second), self.allowPartial, self.enforceConstraints)


class _RCSuperOpArgsBound:
	def __init__(self, allowPartial: bool, enforceConstraints: bool) -> None:
		self.allowPartial = allowPartial
		self.enforceConstraints = enforceConstraints

	def __rmul__(self, first: RCExpExp) -> _RCSuperOpFirstBound:
		return _RCSuperOpFirstBound(self.allowPartial, self.enforceConstraints, first)


class _RCSuperOp:
	def __call__(self, allowPartial: bool=True, enforceConstraints: bool=False) -> _RCSuperOpArgsBound:
		return _RCSuperOpArgsBound(allowPartial, enforceConstraints)

	def __rmul__(self, first: RCExpExp) -> _RCSuperOpFirstBound:
		return first * self()


rcSuper = _RCSuperOp()


# ----------------------------------------------------------
# Util
# ----------------------------------------------------------

def showDump(f: str) -> None:
	return libpymod.showDump(prefixFilename(f))  # type: ignore


###########################################################
# Causality
###########################################################

# EventTrace
# ----------------------------------------------------------

_EventTrace_load_orig = causality.EventTrace.load
def _EventTrace_load(dgOrNet, f):
	return _EventTrace_load_orig(dgOrNet, prefixFilename(f))
causality.EventTrace.load = _EventTrace_load  # type: ignore

_EventTrace_print_orig = causality.EventTrace.print
def _EventTrace_print(self: causality.EventTrace, printer: Optional[causality.EventTracePrinter] = None) -> str:
	if printer is None:
		printer = causality.EventTracePrinter()
	return _EventTrace_print_orig(self, printer)
causality.EventTrace.print = _EventTrace_print  # type: ignore

_EventTracePrinter_pushOptions_orig = causality.EventTracePrinter.pushOptions
causality.EventTracePrinter.pushOptions = (  # type: ignore
	lambda self, f: _EventTracePrinter_pushOptions_orig(self, _funcWrap(libpymod._Func_StringDG, f)))

_EventTracePrinter_pushVertexVisible_orig = causality.EventTracePrinter.pushVertexVisible
causality.EventTracePrinter.pushVertexVisible = (  # type: ignore
	lambda self, f: _EventTracePrinter_pushVertexVisible_orig(self, _funcWrap(libpymod._Func_BoolDGVertex, f)))

_EventTracePrinter_pushVertexOptions_orig = causality.EventTracePrinter.pushVertexOptions
causality.EventTracePrinter.pushVertexOptions = (  # type: ignore
	lambda self, f: _EventTracePrinter_pushVertexOptions_orig(self, _funcWrap(libpymod._Func_StringDGVertex, f)))

_EventTracePrinter_setPreContent_orig = causality.EventTracePrinter.setPreContent
causality.EventTracePrinter.setPreContent = (  # type: ignore
	lambda self, f: _EventTracePrinter_setPreContent_orig(self, _funcWrap(libpymod._Func_StringDG, f)))

_EventTracePrinter_setPostContent_orig = causality.EventTracePrinter.setPostContent
causality.EventTracePrinter.setPostContent = (  # type: ignore
	lambda self, f: _EventTracePrinter_setPostContent_orig(self, _funcWrap(libpymod._Func_StringDG, f)))


# Stochsim
# ----------------------------------------------------------

class _Simulator:
	class DrawTimeExponential:
		def __call__(self, activitySum: float) -> float:
			return -math.log(rngUniformReal()) / activitySum


	class ExpandByStrategy:
		def __init__(self, strat: DGStrat):
			self.strat = dgStrat(strat)

		def __call__(self, b: DG.Builder,
				s: List[Graph], u: List[Graph]) -> bool:
			b.execute(addSubset(s) >> addUniverse(u) >> self.strat, verbosity=0)
			return True


	class DrawMassAction:
		def __init__(self, *,
				inputRate:    Union[None, Callable[[DG.Vertex],
					Tuple[float, bool]], Tuple[float, bool]] = None,
				reactionRate: Union[None, Callable[[DG.HyperEdge],
					Tuple[float, bool]], Tuple[float, bool]] = None,
				outputRate:   Union[None, Callable[[DG.Vertex],
					Tuple[float, bool]], Tuple[float, bool]] = None) -> None:
			self.inputRate = inputRate
			self.reactionRate = reactionRate
			self.outputRate = outputRate

		def __call__(self, dg: DG) -> "causality.Simulator.DrawMassAction.Function":  # type: ignore
			return causality.Simulator.DrawMassAction.Function(dg,  # type: ignore
				self.inputRate, self.reactionRate, self.outputRate)


	def __init__(self, *,
			labelSettings: LabelSettings = LabelSettings(
				LabelType.String, LabelRelation.Isomorphism),
			graphDatabase: List[Graph],  # noqa
			expandNetwork: Callable[
				[DG.Builder, List[Graph], List[Graph]], bool],
			initialState: Dict[Graph, int],
			draw: Callable[[DG], "causality.DrawFunction"] = DrawMassAction(),  # type: ignore
			drawTime: Callable[[float], float] = DrawTimeExponential(),
			withSetCompare: bool = True) -> None:
		self._impl = causality._SimulatorImpl()  # type: ignore
		self._dg = DG(graphDatabase=graphDatabase,
							labelSettings=labelSettings)
		self._builder = self._dg.build()
		self._doExpansion = True
		self._expandNetwork = expandNetwork
		# make sure the initial state is represented in the DG
		self._builder.execute(addSubset(initialState), verbosity=0)

		self._withSetCompare = withSetCompare
		if withSetCompare:
			self._markingSupportSet: Optional[causality.MarkingSet] \
				= causality.MarkingSet()
		else:
			self._markingSupportSet = None
		self._petriNet = causality.Net(self._dg)
		self._marking = causality.Marking(self._petriNet)
		for g, c in initialState.items():
			v = self._dg.findVertex(g)
			assert v
			self._marking.add(v, c)
		self._trace = causality.EventTrace(self._marking)

		self._draw = draw(self._dg)
		self._drawTime = drawTime

		self.onIterationBegin = lambda self: None
		self.onIterationEnd = lambda self, action, timeInc: True
		self.onDeadlock = lambda self: None
		self.onExpand = lambda self: None
		self.onExpandAvoided = lambda self: None

	@property
	def dg(self) -> DG:
		return self._dg

	@property
	def iteration(self) -> int:
		return self._impl.iteration

	@property
	def time(self) -> float:
		return self._impl.time

	@property
	def trace(self) -> causality.EventTrace:
		return self._trace

	def __setattr__(self, name, value) -> None:
		if name == "onIterationBegin":
			import inspect
			spec = inspect.getfullargspec(value)
			if len(spec.args) != 1:
				_deprecation("causality.Simulator.onIterationBegin has been changed to take just the simulator object as argument. Use the .time and .iteration properties to get the old information.")
				origValue = value
				def fWrapped(sim, f=origValue) -> None:
					return f(sim.time, sim.iteration)
				value = fWrapped
		elif name == "onNewState":
			import inspect
			spec = inspect.getfullargspec(value)
			if len(spec.args) != 3:
				_deprecation("causality.Simulator.onNewState has been renamed to causality.Simulator.onIterationEnd, and changed to take just the simulator object as argument. Use the .time, .iteration, and .trace properties to get the old information.")
				origValue = value
				def fWrapped(sim, action, timeInc, f=origValue) -> None:  # type: ignore
					return f(sim.time, sim.iteration, self.trace, action, timeInc)
				name = "onIterationEnd"
				value = fWrapped
		elif name == "onDeadlock":
			import inspect
			spec = inspect.getfullargspec(value)
			if len(spec.args) != 1:
				_deprecation("causality.Simulator.onDeadlock has been changed to take just the simulator object as argument. Use the .time, .iteration, and .trace properties to get the old information.")
				origValue = value
				def fWrapped(sim, f=origValue) -> None:
					return f(sim.time, sim.iteration, self.trace)
				value = fWrapped
		elif name in ("onRecompute", "onRecomputeAvoided"):
			newName = "onExpand" + name[11:]
			_deprecation("causality.Simulator.{} has been renamed to causality.Simulator.{}, and changed to take just the simulator object as argument. Use the .iteration to get the old information.".format(name, newName))
			name = newName
		super().__setattr__(name, value)

	def simulate(self, *,
			time: Optional[float] = None,
			advanceToEndTime: bool = False,
			iterations: Optional[int] = None,
			keepNetworkOpen: bool = False) -> causality.EventTrace:
		stopTime = None if time is None else self._impl.time + time
		stopIter = None if iterations is None else self._impl.iteration + iterations
		marking = self._marking

		subset = marking.getNonZeroPlaces()
		if self._doExpansion:
			# do an initial network expansion, e.g., to support a dynamically added
			# network, where no further expansions are doing anything
			self.onExpand(self)
			self._expandNeighbourhood(subset)

		while stopIter is None or self._impl.iteration < stopIter:
			self._impl.doIteration()
			self.onIterationBegin(self)

			# do we need to expand the neighbourhood?
			if len(subset) > 0 and self._doExpansion:
				if (self._markingSupportSet is None
						or self._markingSupportSet.addIfNotSubset(marking)):
					self.onExpand(self)
					self._expandNeighbourhood(subset)
				else:
					self.onExpandAvoided(self)

			# Pick reaction and time
			action, rateSum = self._draw.draw(marking)
			if rateSum == 0:
				self.onDeadlock(self)
				break

			timeInc = self._drawTime(rateSum)
			if stopTime is not None and self._impl.time + timeInc > stopTime:
				if advanceToEndTime:
					self._impl.time = stopTime
				break
			self._impl.time += timeInc

			# Update state
			if isinstance(action, causality.EdgeAction):
				subset = marking.getEmptyPostPlaces(action.edge)
			elif isinstance(action, causality.OutputAction):
				subset = []
			elif isinstance(action, causality.InputAction):
				subset = [action.vertex]
			else:
				assert False, "Unknown action for subset computation: {}".format(action)
			action.applyTo(marking)
			self._trace.add(self._impl.time, action)
			continue_ = self.onIterationEnd(self, action, timeInc)
			if not continue_:
				break

		if not keepNetworkOpen and hasattr(self, "_builder"):
			del self._builder
		return self._trace

	def _expandNeighbourhood(self, subset: List[DG.Vertex]) -> None:
		if not hasattr(self, "_builder"):
			raise LogicError("Can not expand neighbourhood, the network is closed. An earlier call to simulate() had keepNetworkOpen=False (the default).")
		subsetGraphs = list(v.graph for v in subset)
		universeGraphs = list(v.graph for v in self._marking.getNonZeroPlaces())
		self._doExpansion = self._expandNetwork(self._builder, subsetGraphs, universeGraphs)
		if self._doExpansion is None:
			_deprecation("causality.Simulator: the expandNetwork callback should now return a boolean, indicating whether to call it again later. Assuming True.")
			self._doExpansion = True
		self._petriNet.syncSize()
		self._marking.syncSize()
		self._draw.syncSize()

causality.Simulator = _Simulator  # type: ignore

causality.Simulator.DrawMassAction.Function = causality._DrawMassActionFunction  # type: ignore

_DrawMassActionFunction__init__orig = causality.Simulator.DrawMassAction.Function.__init__  # type: ignore
def _DrawMassActionFunction__init__(self: causality.Simulator.DrawMassAction.Function,  # type: ignore
			dg: DG,
			inputRate:    Union[None, Callable[[DG.Vertex],
				Tuple[float, bool]], Tuple[float, bool]],
			reactionRate: Union[None, Callable[[DG.HyperEdge],
				Tuple[float, bool]], Tuple[float, bool]],
			outputRate:   Union[None, Callable[[DG.Vertex],
				Tuple[float, bool]], Tuple[float, bool]]) -> None:
	inputRate = None if inputRate is None else  _funcWrap(libpymod._Func_PairDoubleBoolDGVertex, inputRate)
	reactionRate = None if reactionRate is None else _funcWrap(libpymod._Func_PairDoubleBoolDGHyperEdge, reactionRate)
	outputRate = None if outputRate is None else _funcWrap(libpymod._Func_PairDoubleBoolDGVertex, outputRate)
	return _DrawMassActionFunction__init__orig(self, dg, inputRate, reactionRate, outputRate)
causality.Simulator.DrawMassAction.Function.__init__ = _DrawMassActionFunction__init__  # type: ignore


###########################################################
# Hyperflow
###########################################################

_setSpecialForIdClass(hyperflow.Model)
hyperflow.Model.__repr__ = hyperflow.Model.__str__  # type: ignore

class Flow:
	def __new__(cls, dg: DG, ilpSolver="default") -> hyperflow.Model:  # type: ignore
		_deprecation("Flow is deprecated, construct a hyperflow.Model object directly.")
		return hyperflow.Model(dg, ilpSolver=ilpSolver)

	@staticmethod
	def load(*args, **kwargs) -> hyperflow.Model:
		_deprecation("Flow.load() is deprecated, use hyperflow.Model.load().")
		return hyperflow.Model.load(*args, **kwargs)

def dgFlow(dg: DG) -> hyperflow.Model:
	_deprecation("dgFlow is deprecated, construct a hyperflow.Model object directly.")
	return hyperflow.Model(dg)

def dgFlowDump(dg: DG, s: str) -> hyperflow.Model:
	_deprecation("dgFlowDump() is deprecated, use hyperflow.Model.load().")
	return hyperflow.Model.load(dg, s)


def dgFlowDumpString(dg: DG, s: str) -> hyperflow.Model:
	_deprecation("dgFlowDumpString() is deprecated, use hyperflow.Model.loadString().")
	return hyperflow.Model.loadString(dg, s)

def _Flow__getattribute__(self: hyperflow.Model, name: str) -> Any:
	if name in ("sources", "sinks", "excludedVertices",
				"customBoolVariables", "customIntVariables", "customFloatVariables",
				"enumerationVars", "transitEnumeration"):
		return _unwrap(object.__getattribute__(self, name))
	else:
		return object.__getattribute__(self, name)
hyperflow.Model.__getattribute__ = _Flow__getattribute__  # type: ignore

def _Flow__setattr__(self: hyperflow.Model, name: str, value: Any) -> None:
	if name == 'objectiveFunction':
		object.__setattr__(self, name, value)
	elif name.startswith("_solverHax"):
		object.__setattr__(self, name, value)
	else:
		_NoNew__setattr__(self, name, value)
hyperflow.Model.__setattr__ = _Flow__setattr__  # type: ignore


_Flow_findSolutions_orig = hyperflow.Model.findSolutions
def _Flow_findSolutions(self: hyperflow.Model, *, maxNumSolutions: int=1,
						verbosity: int=1, ilpVerbosity: int=1) -> hyperflow.SolutionRange:
	return _Flow_findSolutions_orig(self, maxNumSolutions,  # type: ignore
	                                verbosity, ilpVerbosity)
hyperflow.Model.findSolutions = _Flow_findSolutions  # type: ignore

def _Flow_setSolverEnumerateBy(self: hyperflow.Model, absGap: Optional[int]=None, maxNumSolutions: int=2**30,
	                           enumerationVarSpecifier: Optional[hyperflow.LinExp]=None,
	                           transitEnumeration: List[Union[Graph, DG.Vertex]]=[]) -> None:
	_deprecation("setSolverEnumerateBy() on a flow model is partially deprecated and removed. Use addEnumerationVar(), addTransitEnumeration(), and arguments to findSolutions() instead.")
	if enumerationVarSpecifier is not None:
		raise Exception("setSolverEnumerateBy no longer accepts enumerationVarSpecifier, use addEnumerationVar() on the flow model instead.")
	self._solverHax = True  # type: ignore
	self._solverHax_maxNumSolutions = maxNumSolutions  # type: ignore
	self._solverHax_transitEnumeration = transitEnumeration  # type: ignore
	self.absGap = absGap
hyperflow.Model.setSolverEnumerateBy = _Flow_setSolverEnumerateBy  # type: ignore

def _Flow_calc(self: hyperflow.Model, *, maxNumSolutions: int=1) -> None:
	_deprecation("calc() on a flow model is deprecated. Use findSolutions() instead.")
	if hasattr(self, "_solverHax"):
		if maxNumSolutions != 1:
			assert False
		print("Calc: using settings from deprecated setSolverEnumerateBy")
		maxNumSolutions = self._solverHax_maxNumSolutions  # type: ignore
		print("\tmaxNumSolutions = %d" % maxNumSolutions)
		print("\tadded transitEnumeration =", end="")
		for g in self._solverHax_transitEnumeration:  # type: ignore
			print("", g, end="")
			self.addTransitEnumeration(g)  # type: ignore
		print()
		del self._solverHax
	self.findSolutions(maxNumSolutions=maxNumSolutions)
hyperflow.Model.calc = _Flow_calc  # type: ignore

_Flow_load_orig = hyperflow.Model.load
def _Flow_load(dg: DG, f: str, ilpSolver: str="default", verbosity: int=1) -> hyperflow.Model:
	return _Flow_load_orig(dg, prefixFilename(f), ilpSolver, verbosity)
hyperflow.Model.load = _Flow_load  # type: ignore


# Variable Specifiers
#----------------------------------------------------------

def FlowLinExp(*args, **kwargs) -> hyperflow.LinExp:
	_deprecation("FlowLinExp is deprecated, construct a hyperflow.LinExp object directly.")
	return hyperflow.LinExp(*args, **kwargs)

_FlowVarSum__repr__ = lambda self: "%s(%s)" % (self.__class__.__name__, self.id)  # noqa
hyperflow.VarSumVertex.__repr__ = _FlowVarSum__repr__  # type: ignore
hyperflow.VarVertex.__repr__ = lambda self: "VarVertex(%s, %s)" % (self.id, self.vertex)  # type: ignore
hyperflow.VarVertexGraph.__repr__ = lambda self: "VarVertexGraph(%s, %s)" % (self.id, self.graph)  # type: ignore
hyperflow.VarSumEdge.__repr__ = _FlowVarSum__repr__  # type: ignore
hyperflow.VarEdge.__repr__ = lambda self: "VarEdge(%s, %s)" % (self.id, self.edge)  # type: ignore
hyperflow.VarSumCustom.__repr__ = lambda self: "VarSumCustom(%s)" % self.id  # type: ignore
hyperflow.VarCustom.__repr__ = lambda self: "VarCustom(%s, %s)" % (self.id, self.name)  # type: ignore


# Operators
#----------------------------------------------------------

_Flow__pos__ = lambda self: +hyperflow.LinExp(self)  # noqa
_Flow__neg__ = lambda self: -hyperflow.LinExp(self)  # noqa
_Flow__add__ = lambda self, other: hyperflow.LinExp(self) + hyperflow.LinExp(other)  # noqa
_Flow__sub__ = lambda self, other: hyperflow.LinExp(self) - hyperflow.LinExp(other)  # noqa
_Flow__mul__ = lambda self, other: hyperflow.LinExp(self) * other  # noqa
_Flow__le__ = lambda self, other: hyperflow.LinExp(self) <= other  # noqa
_Flow__eq__ = lambda self, other: hyperflow.LinExp(self) == other  # noqa
_Flow__ge__ = lambda self, other: hyperflow.LinExp(self) >= other  # noqa
		
for t in [hyperflow.VarSumVertex, hyperflow.VarVertex, hyperflow.VarVertexGraph,
		hyperflow.VarSumEdge, hyperflow.VarEdge,
		hyperflow.VarSumCustom, hyperflow.VarCustom
	]:
	if t is not hyperflow.LinExp:
		t.__pos__ = _Flow__pos__  # type: ignore
		t.__neg__ = _Flow__neg__  # type: ignore
		t.__le__ = _Flow__le__  # type: ignore
		t.__eq__ = _Flow__eq__  # type: ignore
		t.__ge__ = _Flow__ge__  # type: ignore
	t.__add__ = _Flow__add__  # type: ignore
	t.__sub__ = _Flow__sub__  # type: ignore
	t.__mul__ = _Flow__mul__  # type: ignore
	t.__rmul__ = _Flow__mul__  # type: ignore

def _FlowVar_indexing(self, x):
	_deprecation("Operator (.) on flow variable specifiers is deprecated. Use operator [.] instead.")
	return self[x]
hyperflow.VarSumVertex.__call__ = _FlowVar_indexing  # type: ignore
hyperflow.VarSumEdge.__call__ = _FlowVar_indexing  # type: ignore

#----------------------------------------------------------
# SolutionRange
#----------------------------------------------------------

_FlowSolutionRange_print_orig = hyperflow.SolutionRange.print
def _FlowSolutionRange_print(self: hyperflow.SolutionRange, printer: Optional[hyperflow.Printer]=None, data: Optional[DGPrintData]=None) -> None:
	if printer is None:
		printer = hyperflow.Printer()
	if data is None:
		data = DGPrintData(self.model.dg)
	_FlowSolutionRange_print_orig(self, printer, data)
hyperflow.SolutionRange.print = _FlowSolutionRange_print  # type: ignore

#----------------------------------------------------------
# Solution
#----------------------------------------------------------

hyperflow.Solution.__hash__ = lambda self: hash((self.flow, self.id))  # type: ignore

_FlowSolution_print_orig = hyperflow.Solution.print
def _FlowSolution_print(self: hyperflow.Solution, printer: Optional[hyperflow.Printer]=None, data: Optional[DGPrintData]=None) -> Tuple[str, str]:
	if printer is None:
		printer = hyperflow.Printer()
	if data is None:
		data = DGPrintData(self.model.dg)
	return _FlowSolution_print_orig(self, printer, data)
hyperflow.Solution.print = _FlowSolution_print  # type: ignore

#----------------------------------------------------------
# Printer
#----------------------------------------------------------

def FlowPrinter(*args, **kwargs) -> hyperflow.Printer:
	_deprecation("FlowPrinter is deprecated, use hyperflow.Printer instead.")
	return hyperflow.Printer(*args, **kwargs)

_FlowPrinter_pushInEdgeLabel_orig = hyperflow.Printer.pushInEdgeLabel
def _FlowPrinter_pushInEdgeLabel(self: hyperflow.Printer, f: Union[str, Callable[[DG.Vertex], str]]) -> None:
	_FlowPrinter_pushInEdgeLabel_orig(self, _funcWrap(libpymod._Func_StringDGVertex, f))
hyperflow.Printer.pushInEdgeLabel = _FlowPrinter_pushInEdgeLabel  # type: ignore

_FlowPrinter_pushOutEdgeLabel_orig = hyperflow.Printer.pushOutEdgeLabel
def _FlowPrinter_pushOutEdgeLabel(self: hyperflow.Printer, f: Union[str, Callable[[DG.Vertex], str]]) -> None:
	_FlowPrinter_pushOutEdgeLabel_orig(self, _funcWrap(libpymod._Func_StringDGVertex, f))
hyperflow.Printer.pushOutEdgeLabel = _FlowPrinter_pushOutEdgeLabel  # type: ignore
