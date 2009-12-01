#ifndef SURFACE_H_INCLUDED
#define SURFACE_H_INCLUDED

#include "renderer.h"

class Patch : Surface
{
	private:
		// uv coordinates for corners of the base patch.
		float m_u[2];
		float m_v[2];

		Vec3 m_P[4];
		PrimvarList m_vars;
		const Options& m_opts;
	public:

        Patch(const Options& opts, Vec3 a, Vec3 b, Vec3 c, Vec3 d)
            : m_opts(opts)
        {
            m_P[0] = a; m_P[1] = b;
            m_P[2] = c; m_P[3] = d;
        }

		void splitdice(const Mat4& proj, RenderQueue& queue) const
		{
			// 1. Project points into pixel coordinates
			// 2. If area is less than threshold, dice
			// 3. Else measure side lengths, & split along the longer one.
			Vec3 a = m_P[0] * splitTrans;
			Vec3 b = m_P[1] * splitTrans;
			Vec3 c = m_P[2] * splitTrans;
			Vec3 d = m_P[3] * splitTrans;

			// Diceable test: Estimate area as the sum of the areas of two triangles
			// which make up the patch.
			float area = 0.5 * (((b-a)%(c-a)).length() + ((b-d)%(c-d)).length());

			const float maxArea = m_opts.gridSize*m_opts.gridSize / m_opts.shadingRate;
			if(area <= maxArea)
			{
				// When the area (in number of micropolys) is small enough,
				// dice the surface.
				boost::shared_ptr<Grid>& grid(new Grid(uRes, vRes));
			}
			else
			{
				// Otherwise, split the surface.  First determine splitting
				// direction -
				//
				// estimate length in a-b, c-d direction
				float l1 = 0.5*((b-a).length() + (d-c).length());
				// estimate length in a-c, b-d direction
				float l2 = 0.5*((c-a).length() + (d-b).length());

				// Split
				if(l1 > l2)
				{
					// split in the middle of the a-b and c-d sides.
					// a---b
					// | | |
					// c---d
					Vec3 ab = 0.5f*(m_P[0] + m_P[1]);
					Vec3 cd = 0.5f*(m_P[2] + m_P[3]);
					queue.push(boost::shared_ptr<Patch>(new Patch(m_P[0], ab,
									                              m_P[2], cd)));
					queue.push(boost::shared_ptr<Patch>(new Patch(ab, m_P[1],
									                              cd, m_P[3])));
				}
				else
				{
					// split in the middle of the a-c and b-d sides.
					// a---b
					// |---|
					// c---d
					Vec3 ac = 0.5f*(m_P[0] + m_P[2]);
					Vec3 bd = 0.5f*(m_P[1] + m_P[3]);
					queue.push(boost::shared_ptr<Patch>(new Patch(m_P[0], m_P[1],
									                              ac, bd)));
					queue.push(boost::shared_ptr<Patch>(new Patch(ac, bd,
									                              m_P[2], m_P[3])));
				}
			}
		}

        Box bound()
        {
            Box b(m_P[0]);
            b.extendBy(m_P[1]);
            b.extendBy(m_P[2]);
            b.extendBy(m_P[3]);
        }
};


#endif // SURFACE_H_INCLUDED
