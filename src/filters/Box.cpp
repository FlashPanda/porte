#include <filters/Box.h>

namespace porte
{
	float BoxFilter::Evaluate(const Vector2f& p) const
	{
		return 1.f;
	}

	Filter* CreateBoxFilter()
	{
		return new BoxFilter(Vector2f(0.5f, 0.5f));
	}
}