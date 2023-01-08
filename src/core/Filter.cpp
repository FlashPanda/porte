#include "Filter.h"

namespace panda
{
	float BoxFilter::Evaluate(const Vector2Df& p) const
	{
		return 1.f;
	}

	Filter* CreateBoxFilter()
	{
		return new BoxFilter(Vector2Df({ 0.5f, 0.5f }));
	}
}