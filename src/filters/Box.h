#pragma once
#include <core/Filter.h>

namespace porte
{
	// Box Filter
	class BoxFilter : public Filter
	{
	public:
		virtual ~BoxFilter() {}
		BoxFilter(const Vector2f& radius) : Filter(radius) {}
		float Evaluate(const Vector2f& p) const;
	};

	Filter* CreateBoxFilter();
}