#include "Term.hpp"

#include <mod/lib/IO/IO.hpp>
#include <mod/lib/IO/ParsingError.hpp>
#include <mod/lib/Term/RawTerm.hpp>
#include <mod/lib/Term/WAM.hpp>
#include <mod/lib/Term/IO/Read.hpp>
#include <mod/lib/Term/IO/Write.hpp>

#include <iostream>

namespace mod::Term {

void mgu(const std::string &left, const std::string &right) {
	lib::Term::RawTerm tRawLeft, tRawRight;
	try {
		tRawLeft = lib::Term::Read::rawTerm(left, lib::Term::getStrings());
		tRawRight = lib::Term::Read::rawTerm(right, lib::Term::getStrings());
	} catch(const lib::IO::ParsingError &e) {
		std::cout << e.msg << '\n';
		return;
	}

	lib::Term::Wam machine, machineTemp;
	lib::Term::RawAppendStore storeMachine, storeMachineTemp;
	auto addrLeft = lib::Term::append(machine, tRawLeft, storeMachine);
	auto addrRight = lib::Term::append(machineTemp, tRawRight, storeMachineTemp);
	machine.setTemp(machineTemp);
	addrRight.type = lib::Term::AddressType::Temp;
	std::cout << "=================================================" << std::endl;
	lib::Term::Write::wam(machine, lib::Term::getStrings(), lib::IO::Logger(std::cout));
	std::cout << "Left = " << addrLeft << ", Right = " << addrRight << std::endl;
	std::cout << "Most general unifier of" << std::endl
			<< "\t";
	lib::Term::Write::term(machine, addrLeft, lib::Term::getStrings(), std::cout);
	std::cout << " =? ";
	lib::Term::Write::term(machine, addrRight, lib::Term::getStrings(), std::cout);
	std::cout << std::endl;
	std::cout << "\t";
	lib::Term::Write::rawTerm(tRawLeft, lib::Term::getStrings(), std::cout);
	std::cout << " =? ";
	lib::Term::Write::rawTerm(tRawRight, lib::Term::getStrings(), std::cout);
	std::cout << std::endl;
	
	auto mgu = machine.unifyHeapTemp(addrLeft.addr, addrRight.addr);
	std::cout << "is\n";
	lib::Term::Write::wam(machine, lib::Term::getStrings(), lib::IO::Logger(std::cout));
	std::cout << "Left = " << machine.deref(addrLeft) << ", Right = " << machine.deref(addrRight) << std::endl;

	lib::Term::Write::mgu(machine, mgu, lib::Term::getStrings(), std::cout);
	std::cout << std::endl;
}

} // namespace mod::Term