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

using namespace Gecode;

class SailMatchOptions : public Options {
public:
	int boats;
	int skippers;
	SailMatchOptions(const char * s, int boats0, int skippers0) : Options(s), boats(boats0), skippers(skippers0){}
	void parse(int& argc, char * argv[]) {
		Options::parse(argc, argv);
		for(int i = 0;i < argc;i++) {
			if (argv[i] )
		}
	}
}


class SailMatch : public Script {
proctected:
	IntVarArray ;
public:
	SailMatch() {
	}
}

int main(int argc, char * argv[]) {
	return 0;
}
