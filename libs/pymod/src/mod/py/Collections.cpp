#include <mod/py/Common.hpp>

#include <mod/Derivation.hpp>
#include <mod/dg/DG.hpp>
#include <mod/dg/GraphInterface.hpp>
#include <mod/dg/Strategies.hpp>
#include <mod/dg/VertexMapper.hpp>
#include <mod/graph/Graph.hpp>
#include <mod/rule/CompositionExpr.hpp>
#include <mod/rule/Rule.hpp>

#include <boost/python/extract.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <vector>

namespace mod {
// This is just hax for the vector_indexing_suite, which requires == and !=.
// Currently, the Boost sandbox has a version where the operators can be disabled.

bool operator==(const Derivation &, const Derivation &) {
	std::cerr << "Derivation does not support ==" << std::endl;
	std::exit(1);
}

namespace rule::RCExp {

bool operator==(const Expression &, const Expression &) {
	std::cerr << "RCExpExp does not support ==" << std::endl;
	std::exit(1);
}

} // namespace rule::RCExp
namespace Py {

template<typename T>
bool listCompare(const std::vector<T> &l, py::object r) {
	auto er = py::extract<const std::vector<T> &>(r);
	if(er.check()) return l == er();
	else {
		std::vector<T> er;
		er.reserve(py::len(r));
		for(int i = 0; i != py::len(r); ++i)
			er.push_back(py::extract<T>(r[i]));
		return l == er;
	}
}

#define makeVector(Name, Type)                        \
   using Name = std::vector<Type>;                    \
   py::class_<Name>("_" #Name)                        \
      .def(py::vector_indexing_suite<Name, true>())   \
      .def("__eq__", &listCompare<Type>)

template<typename T, typename U>
void makePair() {
	py::to_python_converter<std::pair<T, U>, PairToTupleConverter<T, U>>();
	TupleToPairConverter<T, U>();
}

void Collections_doExport() {
	// Vector
	makeVector(VecDerivation, mod::Derivation);
	makeVector(VecDGVertex, dg::DG::Vertex);
	makeVector(VecDGHyperEdge, dg::DG::HyperEdge);
	makeVector(VecDGStrat, std::shared_ptr<dg::Strategy>);
	makeVector(VecGraph, std::shared_ptr<graph::Graph>);
	makeVector(VecVecGraph, std::vector<std::shared_ptr<graph::Graph>>);
	makeVector(VecRule, std::shared_ptr<rule::Rule>);
	using PairString = std::pair<std::string, std::string>;
	makeVector(VecPairString, PairString);
	using PairStringBool = std::pair<std::string, bool>;
	makeVector(VecPairStringBool, PairStringBool);
	makeVector(VecRCExpExp, rule::RCExp::Expression);
	makeVector(VecString, std::string);

	// Pair
	makePair<std::string, std::string>();
	makePair<std::string, bool>();
	makePair<int, int>();
	makePair<double, double>();
	makePair<std::shared_ptr<rule::Rule>, dg::VertexMapper::Map>();

	// Optional
	py::to_python_converter<std::optional<int>, ToPythonOptionalValue<int>>();
}

} // namespace Py
} // namespace mod