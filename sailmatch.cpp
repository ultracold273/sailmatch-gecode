/**
 * Author: 
 * SID:
 *
 * CSCI 5240 Project
 *
 * Sailing Match Race Schedules: Round-Robin Pairing Lists
 * http://www.dcs.gla.ac.uk/~ciaran/papers/cp2015-boats-paper.pdf
 * by Craig Macdonald, Ciaran McCreesh, ALice Miller, Patrick Prosser
 * on CP2015
 */

#include <gecode/int.hh>
#include <gecode/search.hh>
#include <gecode/driver.hh>
#include <string>
#include <sstream>

using namespace Gecode;

class SailMatchOptions : public Options {
public:
	int boats;
	int skippers;
	SailMatchOptions(const char * s, int boats0, int skippers0) : Options(s), boats(boats0), skippers(skippers0){}
	void parse(int& argc, char * argv[]) {
		Options::parse(argc, argv);
		std::istringstream strInt;
		for(int i = 0;i < argc;i++) {
			std::string strArg(argv[i]);
			if (strArg == "-boats" && i + 1 < argc) {
				strInt.str(argv[i+1]);
				if (!(strInt >> boats)) {
					std::cerr << "Invalid Argument for boats" << argv[i + 1] << std::endl;
				}
				i++;
			}
			if (strArg == "-skippers" && i + 1 < argc) {
				strInt.str(argv[i+1]);
				if (!(strInt >> skippers)) {
					std::cerr << "Invalid Argument for skippers" << argv[i + 1] << std::endl;
				}
				i++;
			}
		}
	}
}


class SailMatch : public Script {
proctected:
	IntVarArray timeSlots;
	const SailMatchOptions& opt;
public:
	SailMatch(const SailMatchOptions& opt) {
	}
}

int main(int argc, char * argv[]) {
	SailMatchOptions opt("SailMatchScheduling", 8, 8);
	opt.solutions(0);
	opt.parse(argc, argv);

	Script::run<SailMatch, BAB, SailMatchOptions>(opt);
	return 0;
}
