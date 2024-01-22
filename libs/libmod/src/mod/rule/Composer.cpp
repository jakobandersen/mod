#include "Composer.hpp"

#include <mod/rule/Rule.hpp>
#include <mod/lib/RC/Evaluator.hpp>

namespace mod::rule {

struct Composer::Pimpl {
	Pimpl(const std::unordered_set<std::shared_ptr<rule::Rule>> &database, LabelSettings labelSettings)
			: evaluator(database, labelSettings) {}
public:
	lib::RC::Evaluator evaluator;
};

Composer::Composer(const std::unordered_set<std::shared_ptr<rule::Rule>> &database, LabelSettings labelSettings)
		: p(new Pimpl(database, labelSettings)) {}

Composer::~Composer() = default;

const std::unordered_set<std::shared_ptr<rule::Rule>> &Composer::getRuleDatabase() const {
	return p->evaluator.getRuleDatabase();
}

const std::unordered_set<std::shared_ptr<rule::Rule>> &Composer::getProducts() const {
	return p->evaluator.getProducts();
}

std::vector<std::shared_ptr<rule::Rule>> Composer::eval(const RCExp::Expression &exp, bool onlyUnique, int verbosity) {
	return p->evaluator.eval(exp, onlyUnique, verbosity);
}

void Composer::print() const {
	p->evaluator.print();
}

//------------------------------------------------------------------------------
// Static
//------------------------------------------------------------------------------

std::shared_ptr<Composer> Composer::create(const std::unordered_set<std::shared_ptr<rule::Rule> > &database,
                                           LabelSettings labelSettings) {
	return std::shared_ptr<Composer>(new Composer(database, labelSettings));
}

} // namespace mod::rule