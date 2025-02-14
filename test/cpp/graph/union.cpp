
#include <mod/GraphConcepts.hpp>
#include <mod/graph/Union.hpp>

#include <mod/lib/LabelledGraph.hpp>
#include <mod/lib/LabelledUnionGraph.hpp>
#include <mod/lib/Graph/LabelledGraph.hpp>
#include <mod/lib/Graph/Properties/Stereo.hpp>
#include <mod/lib/Graph/Properties/String.hpp>
#include <mod/lib/Graph/Properties/Term.hpp>

#include <boost/concept_check.hpp>

int main() {
	BOOST_CONCEPT_ASSERT((mod::concepts::LabelledGraph<mod::graph::Union>));
	BOOST_CONCEPT_ASSERT((mod::lib::LabelledGraphConcept<mod::lib::LabelledUnionGraph<mod::lib::graph::LabelledGraph>>));
}