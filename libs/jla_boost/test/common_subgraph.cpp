// Adapted from Boost.Graph mcgregor_common_subgraphs_test

#define MORPHISM_INJECTIVE_ENUMERATION_DEBUG

#include <jla_boost/graph/morphism/AsPropertyMap.hpp>
#include <jla_boost/graph/morphism/Predicates.hpp>
#include <jla_boost/graph/morphism/callbacks/Limit.hpp>
#include <jla_boost/graph/morphism/finders/CommonSubgraph.hpp>
#include <jla_boost/graph/morphism/models/Inverted.hpp>

#include <boost/lexical_cast.hpp>

#define BOOST_ALLOW_DEPRECATED_HEADERS

#include <boost/random.hpp>

#undef BOOST_ALLOW_DEPRECATED_HEADERS

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/isomorphism.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/random.hpp>
#include <boost/property_map/shared_array_property_map.hpp>

#define _DO_ASSERT(exp)                                                               \
   do {                                                                               \
      const bool res = exp;                                                           \
      if(!res) throw std::runtime_error(std::string(__func__) + " in "                \
         + std::string(__FILE__) + ":" + boost::lexical_cast<std::string>(__LINE__)); \
   } while(false)

#define BOOST_CHECK(exp) _DO_ASSERT(exp)
#define BOOST_REQUIRE(exp) _DO_ASSERT(exp)

#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <random>


namespace jla_boost {
namespace test {
using namespace boost;
namespace GM = jla_boost::GraphMorphism;
using namespace GM;
namespace {

// Fills a membership map (vertex -> bool) using the information
// present in th vertex map.
// Every vertex in a membership map will have a true value only if it is not
// associated with a null vertex in the correspondence map.

template<typename GraphRight, typename GraphLeft, typename VertexMap, typename MembershipMap>
void fillMembershipMap(const GraphLeft &gLeft, const VertexMap &m, MembershipMap &pMemberhip) {
	for(auto vLeft : asRange(vertices(gLeft)))
		put(pMemberhip, vLeft, get(m, vLeft) != boost::graph_traits<GraphRight>::null_vertex());
}

// Returns a filtered sub-graph of graph whose edge and vertex inclusion is dictated by membership_map.

template<typename Graph, typename MembershipMap>
boost::filtered_graph<Graph, boost::keep_all, boost::property_map_filter<MembershipMap> >
make_membership_filtered_graph(const Graph &graph, MembershipMap &pMembership) {
	using VertexFilter = boost::property_map_filter<MembershipMap>;
	VertexFilter v_filter(pMembership);
	return boost::filtered_graph<Graph, boost::keep_all, VertexFilter>(graph, boost::keep_all(), v_filter);
}

// Callback that compares incoming graphs to the supplied common subgraph.
template<typename Graph>
struct CheckSubgraphCallback {
	CheckSubgraphCallback(Graph &gCommon, bool outputGraphs)
			: gCommon(gCommon), outputGraphs(outputGraphs) {}

	template<typename VertexMap>
	bool operator()(VertexMap &&m, const Graph &gLeft, const Graph &gRight) {
		auto subgraphSize = get_prop(PreImageSizeT(), m);
		using namespace jla_boost;
		if(subgraphSize != num_vertices(gCommon)) return true;

		// Fill membership maps for both graphs
		// Generate filtered graphs using membership maps.
		auto pMemberLeft = boost::make_shared_array_property_map(num_vertices(gLeft), false,
		                                                         get(boost::vertex_index, gLeft));
		auto pMemberRight = boost::make_shared_array_property_map(num_vertices(gRight), false,
		                                                          get(boost::vertex_index, gRight));
		fillMembershipMap<Graph>(gLeft, makeAsPropertyMap(std::ref(m), gLeft, gRight), pMemberLeft);
		fillMembershipMap<Graph>(gRight,
		                         makeAsPropertyMap(makeInvertedVertexMap(std::ref(m)), gRight, gLeft),
		                         pMemberRight);
		auto gLeftFilt = make_membership_filtered_graph(gLeft, pMemberLeft);
		auto gRightFilt = make_membership_filtered_graph(gRight, pMemberRight);

		auto idxLeft = get(boost::vertex_index, gLeftFilt);
		auto idxRight = get(boost::vertex_index, gRightFilt);

		auto pvNameCommon = get(boost::vertex_name, gCommon);
		auto pvNameLeft = get(boost::vertex_name, gLeftFilt);
		auto pvNameRight = get(boost::vertex_name, gRightFilt);

		auto peNameCommon = get(boost::edge_name, gCommon);
		auto peNameLeft = get(boost::edge_name, gLeftFilt);
		auto peNameRight = get(boost::edge_name, gRightFilt);

		// Verify that subgraph1 matches the supplied common subgraph
		for(auto vLeft : asRange(vertices(gLeftFilt))) {
			auto vCommon = vertex(get(idxLeft, vLeft), gCommon);
			// Match vertex names
			if(get(pvNameCommon, vCommon) != get(pvNameLeft, vLeft)) return true;
			for(auto vertex1_2 : asRange(vertices(gLeftFilt))) {
				auto vertex_common2 = vertex(get(idxLeft, vertex1_2), gCommon);
				auto edge_common = edge(vCommon, vertex_common2, gCommon);
				auto edge1 = edge(vLeft, vertex1_2, gLeftFilt);
				if(edge_common.second != edge1.second) return true;
				if((edge_common.second && edge1.second) &&
				   (get(peNameCommon, edge_common.first) != get(peNameLeft, edge1.first))) {
					return true;
				}
			}
		} // BGL_FORALL_VERTICES_T (subgraph1)

		// Verify that subgraph2 matches the supplied common subgraph

		for(auto vRight : asRange(vertices(gRightFilt))) {
			auto vCommon = vertex(get(idxRight, vRight), gCommon);
			// Match vertex names
			if(get(pvNameCommon, vCommon) != get(pvNameRight, vRight)) {
				return true;
			}
			for(auto vertex2_2 : asRange(vertices(gRightFilt))) {
				auto vertex_common2 = vertex(get(idxRight, vertex2_2), gCommon);
				auto edge_common = edge(vCommon, vertex_common2, gCommon);
				auto edge2 = edge(vRight, vertex2_2, gRightFilt);
				if(edge_common.second != edge2.second) return true;
				if((edge_common.second && edge2.second) &&
				   // TODO: should it really be && and not == inside the paren?
				   (get(peNameCommon, edge_common.first) != get(peNameRight, edge2.first))) {
					return true;
				}
			}
		} // BGL_FORALL_VERTICES_T (subgraph2)

		// Check isomorphism just to be thorough
		if(verify_isomorphism(gLeftFilt, gRightFilt, makeAsPropertyMap(std::ref(m), gLeft, gRight))) {
			found = true;
			if(outputGraphs) {
				std::fstream file_subgraph("found_common_subgraph.dot", std::fstream::out);
				write_graphviz(file_subgraph, gLeftFilt,
				               make_label_writer(get(boost::vertex_name, gLeft)),
				               make_label_writer(get(boost::edge_name, gLeft)));
			}
			return false;
		}
		return true;
	}

private:
	Graph &gCommon;
	const bool outputGraphs;
public:
	bool found = false;
};

struct SimpleRecordCallback {
	template<typename VertexMap, typename GraphLeft, typename GraphRight>
	bool operator()(VertexMap &&m, const GraphLeft &gLeft, const GraphRight &gRight) {
		std::stringstream ssSubgraph;
		for(auto vLeft : asRange(vertices(gLeft))) {
			auto vRight = get(m, gLeft, gRight, vLeft);
			if(vRight != boost::graph_traits<GraphRight>::null_vertex()) {
				ssSubgraph << vLeft << "," << vRight << " ";
			}
		}
		subgraphStrings.push_back(ssSubgraph.str());
		return true;
	}
public:
	std::vector<std::string> subgraphStrings;
};

template<typename Graph, typename RandomNumberGenerator, typename VertexNameMap, typename EdgeNameMap>
void addRandomVertices(Graph &graph, RandomNumberGenerator &generator,
                       int vertices_to_create, int max_edges_per_vertex, VertexNameMap vname_map,
                       EdgeNameMap ename_map) {

	typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
	typedef std::vector<Vertex> VertexList;

	VertexList new_vertices;
	for(int i = 0; i < vertices_to_create; ++i) {
		Vertex new_vertex = add_vertex(graph);
		put(vname_map, new_vertex, generator());
		new_vertices.push_back(new_vertex);
	}

	// Add edges for every new vertex. Care is taken to avoid parallel edges.
	for(const auto source_vertex : new_vertices) {
		int edges_for_vertex = std::min((int) (generator() % max_edges_per_vertex) + 1,
		                                (int) num_vertices(graph));
		while(edges_for_vertex > 0) {
			Vertex target_vertex = random_vertex(graph, generator);
			if(source_vertex == target_vertex)
				continue;

			for(const auto edge : asRange(out_edges(source_vertex, graph))) {
				if(target(edge, graph) == target_vertex)
					continue;
			}

			put(ename_map, add_edge(source_vertex, target_vertex, graph).first,
			    generator());

			edges_for_vertex--;
		}
	}
}

bool hasString(const std::string &v, const std::vector<std::string> &strings) {
	return std::find(strings.begin(), strings.end(), v) != strings.end();
}

using DiGraph = boost::adjacency_list<boost::listS, boost::vecS, boost::directedS,
		boost::property<boost::vertex_name_t, int>, boost::property<boost::edge_name_t, int>>;
using UndiGraph = boost::adjacency_list<boost::listS, boost::vecS, boost::directedS,
		boost::property<boost::vertex_name_t, int>, boost::property<boost::edge_name_t, int>>;

#define TEST_PREAMBLE_BASE(Graph)                                                 \
    std::cout << __func__ << "<" << type_name<typename boost::graph_traits<Graph>::directed_category>() << ">:" << std::endl;      \
    std::cout << std::string(80, '=') << std::endl;                               \
    Graph gLeft, gRight;                                                          \
    auto pvNameLeft = get(boost::vertex_name, gLeft);                             \
    auto pvNameRight = get(boost::vertex_name, gRight);                           \
    auto peNameLeft = get(boost::edge_name, gLeft);                               \
    auto peNameRight = get(boost::edge_name, gRight);                             \
    (void)pvNameLeft; (void)pvNameRight; (void)peNameLeft; (void)peNameRight;
#define TEST_PREAMBLE() TEST_PREAMBLE_BASE(DiGraph)
#define TEST_PREAMBLE_UNDIRECTED() TEST_PREAMBLE_BASE(UndiGraph)

void assertMatches(const SimpleRecordCallback &callback, std::vector<std::string> expected) {
	auto actualSorted = callback.subgraphStrings;
	std::sort(actualSorted.begin(), actualSorted.end());
	std::sort(expected.begin(), expected.end());
	auto res = std::mismatch(actualSorted.begin(), actualSorted.end(), expected.begin(), expected.end());
	if(res.first == actualSorted.end() && res.second == expected.end()) return;
	std::string msg = "Mismatch in matches.\nCommon prefix:\n";
	for(auto iter = actualSorted.begin(); iter != res.first; ++iter)
		msg += *iter + "\n";
	msg += "Remaining actual:\n";
	for(auto iter = res.first; iter != actualSorted.end(); ++iter)
		msg += *iter + "\n";
	msg += "Remaining expected:\n";
	for(auto iter = res.second; iter != expected.end(); ++iter)
		msg += *iter + "\n";
	msg += "\nOrignal unsorted actual matches:\n";
	for(const auto &s : callback.subgraphStrings)
		msg += s + "\n";
	throw std::runtime_error(std::move(msg));
}

#define ASSERT_MATCH(MatchString)                                                 \
    if(!hasString(MatchString, callback.subgraphStrings))                         \
        throw std::runtime_error("Subgraph '" MatchString "' not found.");
#define ASSERT_NO_MATCH(MatchString)                                              \
    if(hasString(MatchString, callback.subgraphStrings))                          \
        throw std::runtime_error("Subgraph '" MatchString "' found.");

void writeGraphs(bool outputGraphs, const SimpleRecordCallback &c) {
	if(outputGraphs) {
		std::copy(c.subgraphStrings.begin(), c.subgraphStrings.end(),
		          std::ostream_iterator<std::string>(std::cout, "\n"));
		std::cout << std::endl;
	}
}

template<typename Graph>
void testMaximumAndUnique(bool outputGraphs) {
	TEST_PREAMBLE_BASE(Graph);
	// left: 0--1
	//       | /
	//       2
	// right: 0--1--3
	//        | /
	//        2
	put(pvNameLeft, add_vertex(gLeft), 0);
	put(pvNameLeft, add_vertex(gLeft), 1);
	put(pvNameLeft, add_vertex(gLeft), 2);

	add_edge(0, 1, gLeft);
	add_edge(0, 2, gLeft);
	add_edge(1, 2, gLeft);

	put(pvNameRight, add_vertex(gRight), 0);
	put(pvNameRight, add_vertex(gRight), 1);
	put(pvNameRight, add_vertex(gRight), 2);
	put(pvNameRight, add_vertex(gRight), 3);

	add_edge(0, 1, gRight);
	add_edge(0, 2, gRight);
	add_edge(1, 2, gRight);
	add_edge(1, 3, gRight);

	{ // Unique subgraphs
		std::cout << "  Searching for unique subgraphs" << std::endl;
		SimpleRecordCallback callback;
		commonSubgraphs_unique<true>(gLeft, gRight,
		                             std::ref(callback),
		                             boost::vertices_equivalent(GM::makePropertyPredicateEq(pvNameLeft, pvNameRight)));
		assertMatches(callback, {"0,0 ", "1,1 ", "2,2 ",
		                         "0,0 1,1 ", "0,0 2,2 ", "1,1 2,2 ",
		                         "0,0 1,1 2,2 "});
		writeGraphs(outputGraphs, callback);
	}
	{ // Maximum subgraphs
		std::cout << "  Searching for maximum subgraphs" << std::endl;
		SimpleRecordCallback callback;
		commonSubgraphs_maximum<true>(gLeft, gRight,
		                              std::ref(callback),
		                              boost::vertices_equivalent(GM::makePropertyPredicateEq(pvNameLeft, pvNameRight)));

		assertMatches(callback, std::vector<std::string>(6, "0,0 1,1 2,2 "));
		writeGraphs(outputGraphs, callback);
	}
	{ // Maximum, unique subgraphs
		std::cout << "  Searching for maximum unique subgraphs" << std::endl;
		SimpleRecordCallback callback;
		commonSubgraphs_maximum_unique<true>(gLeft, gRight,
		                                     std::ref(callback),
		                                     boost::vertices_equivalent(
				                                     GM::makePropertyPredicateEq(pvNameLeft, pvNameRight)));

		assertMatches(callback, {"0,0 1,1 2,2 "});
		writeGraphs(outputGraphs, callback);
	}
}

template<typename Graph>
void testEdgeInduction(bool outputGraphs) {
	// Test on subgraphs where induction is not required in both graphs
	TEST_PREAMBLE_BASE(Graph);
	// g1: 0   1-->2   3-->4
	// g2: 0   1   2-->3-->4
	for(int i = 0; i <= 4; ++i) {
		put(pvNameLeft, add_vertex(gLeft), i);
		put(pvNameRight, add_vertex(gRight), i);
	}
	add_edge(1, 2, gLeft);
	add_edge(3, 4, gLeft);

	add_edge(2, 3, gRight);
	add_edge(3, 4, gRight);

	// Unique subgraphs
	std::cout << "  Searching for general subgraphs" << std::endl;
	SimpleRecordCallback callback;
	commonSubgraphs_maximum<false>(gLeft, gRight,
	                               std::ref(callback),
	                               boost::vertices_equivalent(
			                               GM::makePropertyPredicateEq(pvNameLeft, pvNameRight)));
	assertMatches(callback, {"0,0 1,1 2,2 3,3 4,4 "});
	writeGraphs(outputGraphs, callback);
}

template<typename Graph>
void testEdgeLabelMatching(bool outputGraphs) {
	// Test on subgraphs where edges don't have the same label.
	TEST_PREAMBLE_BASE(Graph);
	// g1: 0-x>1
	// g2: 0-y>1
	for(int i = 0; i <= 1; ++i) {
		put(pvNameLeft, add_vertex(gLeft), i);
		put(pvNameRight, add_vertex(gRight), i);
	}
	add_edge(0, 1, 5, gLeft);
	add_edge(0, 1, 42, gRight);

	SimpleRecordCallback callback;
	commonSubgraphs_maximum<false>(gLeft, gRight,
	                               std::ref(callback),
	                               edges_equivalent(GM::makePropertyPredicateEq(peNameLeft, peNameRight)).
			                               vertices_equivalent(GM::makePropertyPredicateEq(pvNameLeft, pvNameRight)));

	assertMatches(callback, {"0,0 ", "1,1 "});
	writeGraphs(outputGraphs, callback);
}

template<typename Graph>
void testAborting(bool outputGraphs) {
	TEST_PREAMBLE_BASE(Graph);
	// g1: 0   1---2   3---4
	// g2: 0   1   2---3---4
	for(int i = 0; i <= 4; ++i) {
		put(pvNameLeft, add_vertex(gLeft), i);
		put(pvNameRight, add_vertex(gRight), i);
	}
	add_edge(1, 2, gLeft);
	add_edge(3, 4, gLeft);

	add_edge(2, 3, gRight);
	add_edge(3, 4, gRight);

	std::cout << "  Searching for 2 subgraphs, and then abort." << std::endl;
	SimpleRecordCallback callback;
	commonSubgraphs<false>(gLeft, gRight,
	                       GM::makeLimit(2, std::ref(callback)),
	                       boost::vertices_equivalent(GM::makePropertyPredicateEq(pvNameLeft, pvNameRight)));

	if(callback.subgraphStrings.size() != 2)
		throw std::runtime_error("Did not find exactly 2 subgraphs. Found " +
		                         boost::lexical_cast<std::string>(callback.subgraphStrings.size()) +
		                         " instead.");
	writeGraphs(outputGraphs, callback);
}

template<typename Graph>
void testPremapping_prePushing(bool outputGraphs) {
	TEST_PREAMBLE_BASE(Graph);
	// g1: 1-x-0-x-2-x-3
	// g2: 1-x-0-x-2-y-3
	// premap 0 and 2, try 3
	for(int i = 0; i <= 3; ++i) {
		put(pvNameLeft, add_vertex(gLeft), i);
		put(pvNameRight, add_vertex(gRight), i);
	}
	add_edge(0, 1, 5, gLeft);
	add_edge(0, 2, 5, gLeft);
	add_edge(2, 3, 5, gLeft);

	add_edge(0, 1, 5, gRight);
	add_edge(0, 2, 5, gRight);
	add_edge(2, 3, 42, gRight);

	std::cout << "  Searching for subgraphs with preTryPush, and preForcePush" << std::endl;
	auto csg = makeCommonSubgraphEnumerator<true>(gLeft, gRight,
	                                              GM::makePropertyPredicateEq(peNameLeft, peNameRight),
	                                              GM::makePropertyPredicateEq(pvNameLeft, pvNameRight));
	{
		std::cout << "    No preTryPush" << std::endl;
		SimpleRecordCallback callback;
		csg(std::ref(callback));
		ASSERT_MATCH("1,1 ");
		writeGraphs(outputGraphs, callback);
	}
	{
		std::cout << "    preTryPush 0,0" << std::endl;
		bool success = csg.preTryPush(0, 0);
		if(!success) throw std::runtime_error("preTryPush 0,0 failed");
		SimpleRecordCallback callback;
		csg(std::ref(callback));
		ASSERT_NO_MATCH("1,1 ");
		ASSERT_MATCH("0,0 1,1 ");
		writeGraphs(outputGraphs, callback);
		csg.prePop();
	}
	{
		std::cout << "    preTryPush 0,0 2,2" << std::endl;
		bool success = csg.preTryPush(0, 0);
		if(!success) throw std::runtime_error("preTryPush 0,0 failed");
		success = csg.preTryPush(2, 2);
		if(!success) throw std::runtime_error("preTryPush 2,2 failed");
		SimpleRecordCallback callback;
		csg(std::ref(callback));
		ASSERT_NO_MATCH("0,0 1,1 ");
		writeGraphs(outputGraphs, callback);
		csg.prePop();
		csg.prePop();
	}
	{
		std::cout << "    preTryPush fail 0,0 2,2 3,3" << std::endl;
		bool success = csg.preTryPush(0, 0);
		if(!success) throw std::runtime_error("preTryPush 0,0 failed");
		success = csg.preTryPush(2, 2);
		if(!success) throw std::runtime_error("preTryPush 2,2 failed");
		success = csg.preTryPush(3, 3);
		if(success) throw std::runtime_error("preTryPush 3,3 not failed");
		csg.prePop();
		csg.prePop();
	}
	{
		std::cout << "    preTryPush fail disconnected 1,1 2,2" << std::endl;
		bool success = csg.preTryPush(1, 1);
		if(!success) throw std::runtime_error("preTryPush 1,1 failed");
		success = csg.preTryPush(2, 2);
		if(success) throw std::runtime_error("preTryPush 2,2 not failed");
		csg.prePop();
	}
	{
		std::cout << "    preForcePush 2,2 3,3" << std::endl;
		csg.preForcePush(2, 2);
		csg.preForcePush(3, 3);
		SimpleRecordCallback callback;
		csg(std::ref(callback));
		ASSERT_MATCH("0,0 2,2 3,3 ");
		ASSERT_MATCH("0,0 1,1 2,2 3,3 ");
		writeGraphs(outputGraphs, callback);
		csg.preForcePop();
		csg.preForcePop();
	}
}

} // namespace

void commonSubgraphRandom(std::size_t n, std::size_t maxEdgesPerVertex, bool outputGraphs, std::size_t seed) {
	std::cout << "testCommonSubgraph: Seed = " << seed << std::endl;
	std::cout << std::string(80, '=') << std::endl;
	boost::minstd_rand generator(seed);

	// Generate a random common subgraph and then add random vertices
	// and edges to the two parent graphs.
	TEST_PREAMBLE();
	DiGraph gCommon;

	for(std::size_t i = 0; i < n; ++i) {
		put(boost::vertex_name_t(), gCommon, add_vertex(gCommon), generator());
	}

	for(auto vSrc : asRange(vertices(gCommon))) {
		for(auto vTar : asRange(vertices(gCommon))) {
			if(vSrc == vTar) continue;
			auto e = add_edge(vSrc, vTar, gCommon).first;
			put(boost::edge_name, gCommon, e, generator());
		}
	}

	boost::randomize_property<boost::vertex_name_t>(gCommon, generator);
	boost::randomize_property<boost::edge_name_t>(gCommon, generator);

	boost::copy_graph(gCommon, gLeft);
	boost::copy_graph(gCommon, gRight);

	// Randomly add vertices and edges to graph1 and graph2.
	addRandomVertices(gLeft, generator, n, maxEdgesPerVertex, pvNameLeft, peNameLeft);
	addRandomVertices(gRight, generator, n, maxEdgesPerVertex, pvNameRight, peNameRight);

	if(outputGraphs) {
		std::ofstream fLeft("gLeft.dot"), fRight("gRight.dot"), fCommon("gCommonExpected.dot");

		write_graphviz(fLeft, gLeft, make_label_writer(pvNameLeft), make_label_writer(peNameLeft));
		write_graphviz(fRight, gRight, make_label_writer(pvNameRight), make_label_writer(peNameRight));
		write_graphviz(fCommon, gCommon,
		               make_label_writer(get(boost::vertex_name, gCommon)),
		               make_label_writer(get(boost::edge_name, gCommon)));
	}

	std::cout << "  Searching for common subgraph of size " << num_vertices(gCommon) << std::endl;

	CheckSubgraphCallback<DiGraph> callback(gCommon, outputGraphs);
	GM::commonSubgraphs<true>(gLeft, gRight, std::ref(callback),
	                          edges_equivalent(GM::makePropertyPredicateEq(peNameLeft, peNameRight)).
			                          vertices_equivalent(GM::makePropertyPredicateEq(pvNameLeft, pvNameRight)));

	if(!callback.found) throw std::runtime_error("Common subgraph not found!");
}

template<typename Graph>
void commonSubgraph(bool outputGraphs) {
	testMaximumAndUnique<Graph>(outputGraphs);
	testEdgeInduction<Graph>(outputGraphs);
	testEdgeLabelMatching<Graph>(outputGraphs);
	testAborting<Graph>(outputGraphs);
	testPremapping_prePushing<Graph>(outputGraphs);
}

} // namespace test
} // namespace jla_boost

int main() {
	using namespace jla_boost::test;
	commonSubgraph<DiGraph>(false);
	commonSubgraph<UndiGraph>(false);

	constexpr std::size_t n = 30;
	constexpr std::size_t maxEdgesPerVertex = 10;
	for(int i = 0; i != 10; ++i) {
		const std::size_t seed = std::random_device()();
		commonSubgraphRandom(n, maxEdgesPerVertex, false, seed);
	}
}