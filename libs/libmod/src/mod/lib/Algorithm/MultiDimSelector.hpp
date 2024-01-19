#ifndef MOD_LIB_ALGORITHM_MULTIDIMSELECTOR_HPP
#define MOD_LIB_ALGORITHM_MULTIDIMSELECTOR_HPP

#include <mod/Error.hpp>

namespace mod::lib {

template<bool AllowPartial, typename InnerRangeProvider>
struct MultiDimSelector {
	using Self = MultiDimSelector<AllowPartial, InnerRangeProvider>;
	using InnerRange = decltype(std::declval<InnerRangeProvider>()(0, 0));
	using InnerIterator = decltype(std::declval<const InnerRange>().begin());

	struct const_iterator;
	friend struct const_iterator;
public:
	MultiDimSelector(std::size_t numPatterns, std::size_t numHosts, InnerRangeProvider morphismProvider)
			: preDisabled(numPatterns, false) {
		assert(numPatterns > 0);
		assert(numHosts > 0);
		morphisms.reserve(numPatterns);
		for(std::size_t iPattern = 0; iPattern < numPatterns; iPattern++) {
			assert(morphisms.size() == iPattern);
			std::vector<InnerRange> morphismsInner;
			morphismsInner.reserve(numHosts);
			for(std::size_t iHost = 0; iHost < numHosts; iHost++) {
				assert(morphismsInner.size() == iHost);
				morphismsInner.push_back(morphismProvider(iPattern, iHost));
			}
			morphisms.push_back(std::move(morphismsInner));
		}
	}

	const_iterator begin() const {
		return const_iterator(this);
	}

	const_iterator end() const {
		return const_iterator();
	}
public:
	// pattern -> host -> morphisms
	std::vector<std::vector<InnerRange> > morphisms;
	std::vector<bool> preDisabled;
};

template<bool AllowPartial, typename InnerRangeProvider>
MultiDimSelector<AllowPartial, InnerRangeProvider> makeMultiDimSelector(std::size_t numPatterns, std::size_t numHosts, InnerRangeProvider morphismProvider) {
	return MultiDimSelector<AllowPartial, InnerRangeProvider>(numPatterns, numHosts, std::forward<InnerRangeProvider>(morphismProvider));
}

// Implementation details
//------------------------------------------------------------------------------

template<bool AllowPartial, typename InnerRangeProvider>
struct MultiDimSelector<AllowPartial, InnerRangeProvider>::const_iterator {

	struct Position {
		std::size_t host;
		InnerIterator iterMorphism, iterMorphismEnd;
		bool disabled;
	public:

		Position() : disabled(false) { }

		bool operator==(const Position &other) const {
			return std::tie(host, iterMorphism) == std::tie(other.host, other.iterMorphism);
		}
	};
	using Positions = std::vector<Position>;
	using reference = const Positions&;
public:

	const_iterator() : owner(nullptr) { }

	const_iterator(const Self *owner) : owner(owner) {
		assert(owner);
		if(owner->morphisms.empty()) {
			owner = nullptr;
			return;
		}
		position.resize(owner->morphisms.size());
		maxHosts = owner->morphisms.front().size();
		std::size_t numUnmatched = 0;
		// find the first match
		for(std::size_t pattern = 0; pattern < position.size(); ++pattern) {
			auto &pos = position[pattern];
			pos.disabled = owner->preDisabled[pattern];
			if(!setFirst(pattern, 0)) {
				if(AllowPartial) {
					pos.host = maxHosts;
					pos.disabled = true;
					++numUnmatched;
				} else {
					this->owner = nullptr;
					return;
				}
			}
		}
		if(numUnmatched == position.size())
			this->owner = nullptr;
	}

	reference operator*() const {
		return position;
	}

	const_iterator &operator++() {
		inc(0);
		return *this;
	}

	bool operator==(const const_iterator &other) const {
		if(owner != other.owner) return false;
		if(!owner) return true;
		return position == other.position;
	}

	bool operator!=(const const_iterator &other) const {
		return !(*this == other);
	}
private:

	bool setFirst(std::size_t pattern, std::size_t firstHost) {
		auto &pos = position[pattern];
		assert(!pos.disabled);
		const auto &mPerHost = owner->morphisms[pattern];
		auto hostIter = std::find_if(mPerHost.begin() + firstHost, mPerHost.end(), [&](const InnerRange & ms) {
			return ms.begin() != ms.end();
		});
		if(hostIter == mPerHost.end()) {
			return false;
		} else {
			pos.host = hostIter - mPerHost.begin();
			pos.iterMorphism = hostIter->begin();
			pos.iterMorphismEnd = hostIter->end();
			return true;
		}
	}

	void inc(std::size_t pattern) {
		assert(owner);
		//			std::cout << ">>> morphism++, " << pattern << "\n";
		if(pattern == position.size()) {
			//				std::cout << ">>>	nullptr\n";
			owner = nullptr;
			return;
		}
		if(position[pattern].disabled) {
			//				std::cout << ">>>	disabled\n";
			inc(pattern + 1);
			return;
		}
		auto &pos = position[pattern];
		if(pos.host == maxHosts) {
			if(!AllowPartial) MOD_ABORT;
			//				std::cout << ">>>	maxHosts\n";
			if(!setFirst(pattern, 0))
				MOD_ABORT;
			inc(pattern + 1);
			return;
		}
		++pos.iterMorphism;
		if(pos.iterMorphism != pos.iterMorphismEnd) {
			//				std::cout << ">>>	morphism\n";
			return;
		}
		//			std::cout << ">>>	setFirst(" << pattern << ", " << (pos.host + 1) << ")\n";
		if(setFirst(pattern, pos.host + 1)) {
			//				std::cout << ">>>	got next\n";
			return;
		}
		if(AllowPartial) {
			pos.host = maxHosts;
			//				std::cout << ">>>	!setFirst, " << pos.host << "/" << (pos.iterMorphismEnd - pos.iterMorphismEnd) << std::endl;
			bool atEnd = std::all_of(position.begin(), position.end(), [this](const Position & p) {
				return p.disabled || p.host == maxHosts;
			});
			//				std::cout << ">>>	atEnd=" << std::boolalpha << atEnd << std::endl;
			if(atEnd) owner = nullptr;
			return;
		} else {
			//				std::cout << ">>>	setFirst(" << pattern << ", 0)\n";
			if(!setFirst(pattern, 0)) MOD_ABORT;
			inc(pattern + 1);
			return;
		}
	}
public:
	const Self *owner;
	std::size_t maxHosts;
	Positions position;
};

} // namespace mod::lib

#endif // MOD_LIB_ALGORITHM_MULTIDIMSELECTOR_HPP