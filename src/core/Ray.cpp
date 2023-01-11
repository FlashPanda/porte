#include "Ray.h"

namespace porte
{
	std::ostream& operator<<(std::ostream& os, const Ray& r)
	{
		os << "[o=" << r.o << ", d=" << r.d << ", tMax = " << r.tMax
			<< "]" << std::endl;
		return os;
	}
}