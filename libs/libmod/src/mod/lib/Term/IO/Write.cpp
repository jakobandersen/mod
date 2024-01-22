#include "Write.hpp"

#include <mod/lib/StringStore.hpp>

#include <array>
#include <iomanip>
#include <unordered_set>

#include <iostream>

namespace mod::lib::Term::Write {
namespace {

std::ostream &rawVarFromCell(std::ostream &s, Cell cell) {
	assert(cell.tag == Cell::Tag::REF);
	switch(cell.REF.addr.type) {
	case AddressType::Heap:
		return s << "_H" << cell.REF.addr.addr;
	case AddressType::Temp:
		return s << "_T" << cell.REF.addr.addr;
	}
	MOD_ABORT;
}

} // namespace

std::ostream &rawTerm(const RawTerm &term, const StringStore &strings, std::ostream &s) {
	struct Printer {
		void operator()(RawVariable v) const {
			s << "_" << strings.getString(v.name);
		}

		void operator()(const RawStructure &str) const {
			s << strings.getString(str.name);
			if(!str.args.empty()) {
				s << '(';
				std::visit(*this, str.args.front());
				for(int i = 1; i != str.args.size(); i++) {
					s << ", ";
					std::visit(*this, str.args[i]);
				}
				s << ')';
			}
		}
	public:
		std::ostream &s;
		const StringStore &strings;
	};
	std::visit(Printer{s, strings}, term);
	return s;
}

std::ostream &element(Cell cell, const StringStore &strings, std::ostream &s) {
	switch(cell.tag) {
	case Cell::Tag::STR:
		return s << "STR " << cell.STR.addr;
	case Cell::Tag::Structure:
		s << strings.getString(cell.Structure.name);
		if(cell.Structure.arity > 0)
			s << "/" << cell.Structure.arity;
		return s;
	case Cell::Tag::REF:
		return s << "REF " << cell.REF.addr;
	}
	__builtin_unreachable();
}

void wam(const Wam &machine, const StringStore &strings, IO::Logger logger) {
	wam(machine, strings, logger, [](Address, std::ostream &) {});
}

void wam(const Wam &machine, const StringStore &strings, IO::Logger logger,
         std::function<void(Address, std::ostream &s)> addressCallback) {
	logger.indent() << "Heap:\n";
	for(std::size_t i = 0; i < machine.getHeap().size(); i++) {
		Cell cell = machine.getHeap()[i];
		logger.indent() << std::setw(5) << std::left << i;
		element(cell, strings, logger.s);
		addressCallback({AddressType::Heap, i}, logger.s);
		logger.s << '\n';
	}
	logger.indent() << "Temp:\n";
	for(std::size_t i = 0; i < machine.getTemp().size(); i++) {
		Cell cell = machine.getTemp()[i];
		logger.indent() << std::setw(5) << std::left << i;
		element(cell, strings, logger.s);
		addressCallback({AddressType::Temp, i}, logger.s);
		logger.s << '\n';
	}
}

std::ostream &term(const Wam &machine, Address addr, const StringStore &strings, std::ostream &s) {
	struct Printer {
		Printer(const Wam &machine, const StringStore &strings, std::ostream &s)
				: machine(machine), strings(strings), s(s) {
			occurred[0].resize(machine.getHeap().size(), 0);
			occurred[1].resize(machine.getTemp().size(), 0);
		}

		void operator()(Address addr) {
			Cell cell = machine.getCell(addr);
			switch(cell.tag) {
			case Cell::Tag::REF:
				if(cell.REF.addr == addr
				   || occurred[static_cast<int>(cell.REF.addr.type)][cell.REF.addr.addr] != 0
						) {
					rawVarFromCell(s, cell);
				} else (*this)(cell.REF.addr);
				break;
			case Cell::Tag::STR:
				(*this)(cell.STR.addr);
				break;
			case Cell::Tag::Structure:
				if(occurred[static_cast<int>(addr.type)][addr.addr] != 0) {
					wam(machine, strings, IO::Logger(s));
					s << "addr.addr = " << addr.addr << std::endl;
					s << "occurred:" << std::endl;
					for(int aType: {0, 1}) {
						for(const auto o: occurred[aType]) {
							if(o == 0) continue;
							s << "   [" << aType << "]: " << o << std::endl;
						}
					}
				}
				assert(occurred[static_cast<int>(addr.type)][addr.addr] == 0);
				s << strings.getString(cell.Structure.name);
				if(cell.Structure.arity > 0) {
					++occurred[static_cast<int>(addr.type)][addr.addr];
					s << "(";
					(*this)(addr + 1);
					for(std::size_t i = 2; i <= cell.Structure.arity; i++) {
						s << ", ";
						(*this)(addr + i);
					}
					s << ")";
					--occurred[static_cast<int>(addr.type)][addr.addr];
				}
				break;
			}
		}
	private:
		const Wam &machine;
		const StringStore &strings;
		std::ostream &s;
		std::array<std::vector<int>, 2> occurred;
	};
	Printer(machine, strings, s)(addr);
	return s;
}

std::ostream &mgu(const Wam &machine, const MGU &mgu, const StringStore &strings, std::ostream &s) {
	switch(mgu.status) {
	case MGU::Status::Exists:
		s << "Exists: ";
		break;
	case MGU::Status::Fail:
		term(machine, mgu.errorLeft, strings, s << "Fail(") << " != ";
		term(machine, mgu.errorRight, strings, s) << ")";
		return s;
	}
	bool first = true;
	for(auto binding: mgu.bindings) {
		if(binding.type == AddressType::Heap && binding.addr >= mgu.preHeapSize) continue;
		if(!first) s << ", ";
		first = false;
		Cell cell;
		cell.tag = Cell::Tag::REF;
		cell.REF.addr = binding;
		rawVarFromCell(s, cell) << " = ";
		term(machine, binding, strings, s);
	}
	return s;
}

} // namespace mod::lib::Term::Write
namespace mod::lib::Term {

std::ostream &operator<<(std::ostream &s, Address addr) {
	switch(addr.type) {
	case AddressType::Heap:
		s << "H";
		break;
	case AddressType::Temp:
		s << "T";
		break;
	}
	return s << "[" << addr.addr << "]";
}

} // namespace mod::lib::Term