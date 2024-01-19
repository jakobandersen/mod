#include <mod/dg/VertexMapper.hpp>

#include <boost/concept_check.hpp>

int main() {
	BOOST_CONCEPT_ASSERT((boost::RandomAccessIterator<mod::dg::VertexMapper::iterator>));
}