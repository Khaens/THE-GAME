#include "Game.h"

int main() {
	// Testing
	Game g(5);
	size_t first = g.WhoStartsFirst();
	std::cout << first;
	return 0;
}