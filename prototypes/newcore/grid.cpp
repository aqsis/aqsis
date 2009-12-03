#include "grid.h"
#include "renderer.h"

void QuadGrid::render(Renderer& renderer)
{
	renderer.render(*this);
}
