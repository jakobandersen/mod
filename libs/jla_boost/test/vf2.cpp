#include <jla_boost/graph/PairToRangeAdaptor.hpp>
#include <jla_boost/graph/morphism/finders/vf2.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/random.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_map/property_map.hpp>

#include <boost/graph/mcgregor_common_subgraphs.hpp>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <random>

namespace jla_boost {
namespace test {
using namespace boost;
namespace GM = jla_boost::GraphMorphism;
using namespace GM;

namespace {

template<typename Graph1, typename Graph2>
void randomly_permute_graph(Graph1 &g1, const Graph2 &g2) {
	BOOST_REQUIRE(num_vertices(g1) <= num_vertices(g2));
	BOOST_REQUIRE(num_edges(g1) == 0);

	typedef typename graph_traits<Graph1>::vertex_descriptor vertex1;
	typedef typename graph_traits<Graph2>::vertex_descriptor vertex2;
	typedef typename graph_traits<Graph1>::vertex_iterator vertex_iterator;
	typedef typename graph_traits<Graph2>::edge_iterator edge_iterator;

	std::mt19937 gen;

	// Decide new order
	std::vector<vertex2> orig_vertices;
	std::copy(vertices(g2).first, vertices(g2).second, std::back_inserter(orig_vertices));
	std::shuffle(orig_vertices.begin(), orig_vertices.end(), gen);
	std::map<vertex2, vertex1> vertex_map;

	std::size_t i = 0;
	for(vertex_iterator vi = vertices(g1).first;
	    vi != vertices(g1).second; ++i, ++vi) {
		vertex_map[orig_vertices[i]] = *vi;
		put(vertex_name_t(), g1, *vi, get(vertex_name_t(), g2, orig_vertices[i]));
	}

	for(edge_iterator ei = edges(g2).first; ei != edges(g2).second; ++ei) {
		typename std::map<vertex2, vertex1>::iterator si = vertex_map.find(source(*ei, g2)),
				ti = vertex_map.find(target(*ei, g2));
		if((si != vertex_map.end()) && (ti != vertex_map.end()))
			add_edge(si->second, ti->second, get(edge_name_t(), g2, *ei), g1);
	}
}

template<typename Graph>
void generate_random_digraph(Graph &g, double edge_probability,
                             int max_parallel_edges, double parallel_edge_probability,
                             int max_edge_name, int max_vertex_name) {

	BOOST_REQUIRE((0 <= edge_probability) && (edge_probability <= 1));
	BOOST_REQUIRE((0 <= parallel_edge_probability) && (parallel_edge_probability <= 1));
	BOOST_REQUIRE(0 <= max_parallel_edges);
	BOOST_REQUIRE(0 <= max_edge_name);
	BOOST_REQUIRE(0 <= max_vertex_name);

	typedef typename graph_traits<Graph>::vertex_iterator vertex_iterator;
	std::mt19937 gen;
	std::uniform_real_distribution<double> dist_real(0.0, 1.0);

	for(vertex_iterator u = vertices(g).first; u != vertices(g).second; ++u) {
		for(vertex_iterator v = vertices(g).first; v != vertices(g).second; ++v) {
			if(dist_real(gen) <= edge_probability) {
				add_edge(*u, *v, g);
				for(int i = 0; i < max_parallel_edges; ++i) {
					if(dist_real(gen) <= parallel_edge_probability)
						add_edge(*u, *v, g);
				}
			}
		}
	}

	{
		std::uniform_int_distribution<int> dist_int(0, max_edge_name);
		const auto random_int_dist = [&dist_int, &gen]() {
			return dist_int(gen);
		};
		randomize_property<vertex_name_t>(g, random_int_dist);
	}

	{
		std::uniform_int_distribution<int> dist_int(0, max_vertex_name);
		const auto random_int_dist = [&dist_int, &gen]() {
			return dist_int(gen);
		};
		randomize_property<edge_name_t>(g, random_int_dist);
	}

}

template<typename EdgeEquivalencePredicate, typename VertexEquivalencePredicate>
struct test_callback {

	test_callback(EdgeEquivalencePredicate edge_comp, VertexEquivalencePredicate vertex_comp, bool output)
			: edge_comp_(edge_comp), vertex_comp_(vertex_comp), output_(output) {}

	template<typename VertexMap, typename GraphDom, typename GraphCodom>
	bool operator()(VertexMap &&m, const GraphDom &gDom, const GraphCodom &gCodom) {

		bool verified;
		BOOST_CHECK(verified = verify_vf2_subgraph_iso(gDom, gCodom, m, edge_comp_, vertex_comp_));

		// Output (sub)graph isomorphism map
		if(!verified || output_) {
			std::cout << "Verfied: " << std::boolalpha << verified << std::endl;
			std::cout << "Num vertices: " << num_vertices(gDom) << ' ' << num_vertices(gCodom) << std::endl;
			for(auto v : asRange(vertices(gDom)))
				std::cout << '(' << get(vertex_index_t(), gDom, v) << ", "
				          << get(vertex_index_t(), gCodom, get(m, gDom, gCodom, v)) << ") ";

			std::cout << std::endl;
		}

		return true;
	}

private:
	EdgeEquivalencePredicate edge_comp_;
	VertexEquivalencePredicate vertex_comp_;
	bool output_;
};

} // namespace

void test_vf2_sub_graph_iso(int n1, int n2, double edge_probability,
                            int max_parallel_edges, double parallel_edge_probability,
                            int max_edge_name, int max_vertex_name, bool output) {

	typedef property<edge_name_t, int> edge_property;
	typedef property<vertex_name_t, int, property<vertex_index_t, int> > vertex_property;

	typedef adjacency_list<listS, listS, bidirectionalS, vertex_property, edge_property> graph1;
	typedef adjacency_list<vecS, vecS, bidirectionalS, vertex_property, edge_property> graph2;

	graph1 g1(n1);
	graph2 g2(n2);
	generate_random_digraph(g2, edge_probability, max_parallel_edges, parallel_edge_probability,
	                        max_edge_name, max_vertex_name);
	randomly_permute_graph(g1, g2);

	int v_idx = 0;
	for(graph_traits<graph1>::vertex_iterator vi = vertices(g1).first;
	    vi != vertices(g1).second; ++vi) {
		put(vertex_index_t(), g1, *vi, v_idx++);
	}


	// Create vertex and edge predicates
	typedef property_map<graph1, vertex_name_t>::type vertex_name_map1;
	typedef property_map<graph2, vertex_name_t>::type vertex_name_map2;

	typedef property_map_equivalent<vertex_name_map1, vertex_name_map2> vertex_predicate;
	vertex_predicate vertex_comp =
			make_property_map_equivalent(get(vertex_name, g1), get(vertex_name, g2));

	typedef property_map<graph1, edge_name_t>::type edge_name_map1;
	typedef property_map<graph2, edge_name_t>::type edge_name_map2;

	typedef property_map_equivalent<edge_name_map1, edge_name_map2> edge_predicate;
	edge_predicate edge_comp =
			make_property_map_equivalent(get(edge_name, g1), get(edge_name, g2));


	std::clock_t start = std::clock();

	// Create callback
	test_callback<edge_predicate, vertex_predicate> callback(edge_comp, vertex_comp, output);

	std::cout << std::endl;
	BOOST_CHECK(vf2_subgraph_iso(g1, g2, callback, vertex_order_by_mult(g1),
	                             edges_equivalent(edge_comp).vertices_equivalent(vertex_comp)));

	std::clock_t end1 = std::clock();
	std::cout << "vf2_subgraph_iso: elapsed time (clock cycles): " << (end1 - start) << std::endl;

	if(num_vertices(g1) == num_vertices(g2)) {
		std::cout << std::endl;
		BOOST_CHECK(vf2_graph_iso(g1, g2, callback, vertex_order_by_mult(g1),
		                          edges_equivalent(edge_comp).vertices_equivalent(vertex_comp)));

		std::clock_t end2 = std::clock();
		std::cout << "vf2_graph_iso: elapsed time (clock cycles): " << (end2 - end1) << std::endl;
	}

	if(output) {
		std::fstream file_graph1("graph1.dot", std::fstream::out);
		boost::write_graphviz(file_graph1, g1,
		                      boost::make_label_writer(get(boost::vertex_name, g1)),
		                      boost::make_label_writer(get(boost::edge_name, g1)));

		std::fstream file_graph2("graph2.dot", std::fstream::out);
		boost::write_graphviz(file_graph2, g2,
		                      boost::make_label_writer(get(boost::vertex_name, g2)),
		                      boost::make_label_writer(get(boost::edge_name, g2)));
	}
}

void test_vf2(int argc, char *argv[]) {

	int num_vertices_g1 = 10;
	int num_vertices_g2 = 20;
	double edge_probability = 0.4;
	int max_parallel_edges = 2;
	double parallel_edge_probability = 0.4;
	int max_edge_name = 5;
	int max_vertex_name = 5;
	bool output = false;

	if(argc > 1) {
		num_vertices_g1 = boost::lexical_cast<int>(argv[1]);
	}

	if(argc > 2) {
		num_vertices_g2 = boost::lexical_cast<int>(argv[2]);
	}

	if(argc > 3) {
		edge_probability = boost::lexical_cast<double>(argv[3]);
	}

	if(argc > 4) {
		max_parallel_edges = boost::lexical_cast<int>(argv[4]);
	}

	if(argc > 5) {
		parallel_edge_probability = boost::lexical_cast<double>(argv[5]);
	}

	if(argc > 6) {
		max_edge_name = boost::lexical_cast<int>(argv[6]);
	}

	if(argc > 7) {
		max_vertex_name = boost::lexical_cast<int>(argv[7]);
	}

	if(argc > 8) {
		output = boost::lexical_cast<bool>(argv[8]);
	}

	test_vf2_sub_graph_iso(num_vertices_g1, num_vertices_g2, edge_probability,
	                       max_parallel_edges, parallel_edge_probability,
	                       max_edge_name, max_vertex_name, output);
}

struct test_callback_2 {

	test_callback_2(bool &got_hit, bool stop) : got_hit(got_hit), stop(stop) {}

	template<typename VertexMap, typename GraphDom, typename GraphCodom>
	bool operator()(VertexMap &&, const GraphDom &, const GraphCodom &) {
		got_hit = true;
		return stop;
	}

	bool &got_hit;
	bool stop;
};

struct false_predicate {

	template<typename VertexOrEdge1, typename VertexOrEdge2>
	bool operator()(VertexOrEdge1 ve1, VertexOrEdge2 ve2) const {
		return false;
	}
};

void test_empty_graph_cases() {
	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS> Graph;
	Graph gEmpty, gLarge;
	add_vertex(gLarge);

	{ // isomorphism
		bool got_hit = false;
		test_callback_2 callback(got_hit, true);
		bool exists = vf2_graph_iso(gEmpty, gEmpty, callback);
		BOOST_CHECK(exists);
		BOOST_CHECK(got_hit); // even empty matches are reported
	}
	{ // subgraph isomorphism (induced)
		{ // empty vs. empty
			bool got_hit = false;
			test_callback_2 callback(got_hit, true);
			bool exists = vf2_subgraph_iso(gEmpty, gEmpty, callback);
			BOOST_CHECK(exists);
			BOOST_CHECK(got_hit); // even empty matches are reported
		}
		{ // empty vs. non-empty
			bool got_hit = false;
			test_callback_2 callback(got_hit, true);
			bool exists = vf2_subgraph_iso(gEmpty, gLarge, callback);
			BOOST_CHECK(exists);
			BOOST_CHECK(got_hit); // even empty matches are reported
		}
	}
	{ // subgraph monomorphism (non-induced subgraph isomorphism)
		{ // empty vs. empty
			bool got_hit = false;
			test_callback_2 callback(got_hit, true);
			bool exists = vf2_subgraph_mono(gEmpty, gEmpty, callback);
			BOOST_CHECK(exists);
			BOOST_CHECK(got_hit); // even empty matches are reported
		}
		{ // empty vs. non-empty
			bool got_hit = false;
			test_callback_2 callback(got_hit, true);
			bool exists = vf2_subgraph_mono(gEmpty, gLarge, callback);
			BOOST_CHECK(exists);
			BOOST_CHECK(got_hit); // even empty matches are reported
		}
	}
}

void test_return_value() {
	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS> Graph;
	typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
	Graph gSmall, gLarge;
	add_vertex(gSmall);
	Vertex v1 = add_vertex(gLarge);
	Vertex v2 = add_vertex(gLarge);
	add_edge(v1, v2, gLarge);

	{ // isomorphism
		{ // no morphism due to sizes
			bool got_hit = false;
			test_callback_2 callback(got_hit, true);
			bool exists = vf2_graph_iso(gSmall, gLarge, callback);
			BOOST_CHECK(!exists);
			BOOST_CHECK(!got_hit);
		}
		{ // no morphism due to vertex mismatches
			bool got_hit = false;
			test_callback_2 callback(got_hit, true);
			false_predicate pred;
			bool exists = vf2_graph_iso(gLarge, gLarge, callback, vertex_order_by_mult(gLarge),
			                            boost::edges_equivalent(pred).vertices_equivalent(pred));
			BOOST_CHECK(!exists);
			BOOST_CHECK(!got_hit);
		}
		{ // morphism, find all
			bool got_hit = false;
			test_callback_2 callback(got_hit, false);
			bool exists = vf2_graph_iso(gLarge, gLarge, callback);
			BOOST_CHECK(exists);
			BOOST_CHECK(got_hit);
		}
		{ // morphism, stop after first hit
			bool got_hit = false;
			test_callback_2 callback(got_hit, true);
			bool exists = vf2_graph_iso(gLarge, gLarge, callback);
			BOOST_CHECK(exists);
			BOOST_CHECK(got_hit);
		}
	}
	{ // subgraph isomorphism (induced)
		{ // no morphism due to sizes
			bool got_hit = false;
			test_callback_2 callback(got_hit, true);
			bool exists = vf2_subgraph_iso(gLarge, gSmall, callback);
			BOOST_CHECK(!exists);
			BOOST_CHECK(!got_hit);
		}
		{ // no morphism due to vertex mismatches
			bool got_hit = false;
			test_callback_2 callback(got_hit, true);
			false_predicate pred;
			bool exists = vf2_subgraph_iso(gLarge, gLarge, callback, vertex_order_by_mult(gLarge),
			                               boost::edges_equivalent(pred).vertices_equivalent(pred));
			BOOST_CHECK(!exists);
			BOOST_CHECK(!got_hit);
		}
		{ // morphism, find all
			bool got_hit = false;
			test_callback_2 callback(got_hit, false);
			bool exists = vf2_subgraph_iso(gLarge, gLarge, callback);
			BOOST_CHECK(exists);
			BOOST_CHECK(got_hit);
		}
		{ // morphism, stop after first hit
			bool got_hit = false;
			test_callback_2 callback(got_hit, true);
			bool exists = vf2_subgraph_iso(gLarge, gLarge, callback);
			BOOST_CHECK(exists);
			BOOST_CHECK(got_hit);
		}
	}
	{ // subgraph monomorphism (non-induced subgraph isomorphism)
		{ // no morphism due to sizes
			bool got_hit = false;
			test_callback_2 callback(got_hit, true);
			bool exists = vf2_subgraph_mono(gLarge, gSmall, callback);
			BOOST_CHECK(!exists);
			BOOST_CHECK(!got_hit);
		}
		{ // no morphism due to vertex mismatches
			bool got_hit = false;
			test_callback_2 callback(got_hit, true);
			false_predicate pred;
			bool exists = vf2_subgraph_mono(gLarge, gLarge, callback, vertex_order_by_mult(gLarge),
			                                boost::edges_equivalent(pred).vertices_equivalent(pred));
			BOOST_CHECK(!exists);
			BOOST_CHECK(!got_hit);
		}
		{ // morphism, find all
			bool got_hit = false;
			test_callback_2 callback(got_hit, false);
			bool exists = vf2_subgraph_mono(gLarge, gLarge, callback);
			BOOST_CHECK(exists);
			BOOST_CHECK(got_hit);
		}
		{ // morphism, stop after first hit
			bool got_hit = false;
			test_callback_2 callback(got_hit, true);
			bool exists = vf2_subgraph_mono(gLarge, gLarge, callback);
			BOOST_CHECK(exists);
			BOOST_CHECK(got_hit);
		}
	}
}

} // namespace test
} // namespace jla_boost

BOOST_AUTO_TEST_CASE(test_main) {
	using namespace jla_boost::test;
	test_vf2(0, nullptr);
	test_empty_graph_cases();
	test_return_value();
}