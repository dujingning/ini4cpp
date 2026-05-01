#define INICPP_DEBUG
#include "../inicpp.hpp"

#ifdef INI_DEBUG
#error "inicpp.hpp should not export the INI_DEBUG macro"
#endif

class TimeFormatter
{
public:
	int value;
};

const int CODE_INFO = 7;

int main()
{
	TimeFormatter formatter;
	formatter.value = CODE_INFO;
	return formatter.value == 7 ? 0 : 1;
}
