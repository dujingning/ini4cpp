#include "../../inicpp.hpp"

int main()
{
	inicpp::ValueProxy proxy(42);
	return static_cast<int>(proxy);
}
