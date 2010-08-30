// Aqsis
// Copyright (C) 1997 - 2007, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

/** \file
 *
 * \brief Unit tests for CqMatrix
 * \author Tobias Sauerwein
 */

#include <aqsis/math/noise.h>
#include <aqsis/math/vector3d.h>
#include <aqsis/math/color.h>

#define BOOST_TEST_DYN_LINK

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

BOOST_AUTO_TEST_SUITE(noise_tests)

const TqFloat epsilon = 0.1f; 


static bool vecEquals(const Aqsis::CqVector3D& a, const Aqsis::CqVector3D& b)
{
	return Aqsis::isClose(a,b);
}

static bool colEquals(const Aqsis::CqColor& a, const Aqsis::CqColor& b)
{
	return Aqsis::isClose(a,b);
}


BOOST_AUTO_TEST_CASE(CqNoise_1D_float_Perlin_noise_test)
{
	Aqsis::CqNoise noise;
	
	BOOST_CHECK_CLOSE(noise.FGNoise1(1.5f), 0.570500016f, epsilon);
}

BOOST_AUTO_TEST_CASE(CqNoise_1D_float_Perlin_periodic_noise_test)
{
	Aqsis::CqNoise noise;
	
	BOOST_CHECK_CLOSE(noise.FGPNoise1(2.1f, 3.4f), 0.475567549f, epsilon);
}

BOOST_AUTO_TEST_CASE(CqNoise_2D_float_Perlin_noise_test)
{
	Aqsis::CqNoise noise;
	
	BOOST_CHECK_CLOSE(noise.FGNoise2(2.1f, 3.4f), 0.505296886f, epsilon);
}

BOOST_AUTO_TEST_CASE(CqNoise_2D_float_Perlin_periodic_noise_test)
{
	Aqsis::CqNoise noise;
	
	BOOST_CHECK_CLOSE(noise.FGPNoise2(2.3f, 4.5f, 3.0f, 2.0f), 0.510765612f, epsilon);
}

BOOST_AUTO_TEST_CASE(CqNoise_3D_float_Perlin_noise_test)
{
	Aqsis::CqNoise noise;
	
	BOOST_CHECK_CLOSE(noise.FGNoise3(Aqsis::CqVector3D(6.3f, 3.0f, 2.0f)), 0.553425074f, epsilon);
}

BOOST_AUTO_TEST_CASE(CqNoise_3D_float_Perlin_periodic_noise_test)
{
	Aqsis::CqNoise noise;
	
	BOOST_CHECK_CLOSE(noise.FGPNoise3(Aqsis::CqVector3D(4.0f, 2.2f, 3.0f), Aqsis::CqVector3D(1.0f, 2.0f, 3.0f)), 0.478314757f, epsilon);
}

BOOST_AUTO_TEST_CASE(CqNoise_4D_float_Perlin_noise_test)
{
	Aqsis::CqNoise noise;
	
	BOOST_CHECK_CLOSE(noise.FGNoise4(Aqsis::CqVector3D(4.0f, 3.1f, 2.0f), 3.0f), 0.546478808f, epsilon);
}

BOOST_AUTO_TEST_CASE(CqNoise_4D_float_Perlin_periodic_noise_test)
{
	Aqsis::CqNoise noise;
	
	BOOST_CHECK_CLOSE(noise.FGPNoise4(Aqsis::CqVector3D(5.0f, 2.0f, 3.0f), 2.2f, Aqsis::CqVector3D(1.0f, 2.0f, 3.0f), 2.0f), 0.520156145f, epsilon);
}

BOOST_AUTO_TEST_CASE(CqNoise_1D_vector_Perlin_noise_test)
{
	Aqsis::CqNoise noise;
	
	BOOST_CHECK_PREDICATE(vecEquals, (noise.PGNoise1(1.0f))(Aqsis::CqVector3D(0.5f, 0.745078f, 0.730963f)));
}

BOOST_AUTO_TEST_CASE(CqNoise_1D_vector_Perlin_periodic_noise_test)
{
	Aqsis::CqNoise noise;
	
	BOOST_CHECK_PREDICATE(vecEquals, (noise.PGPNoise1(1.0f, 2.0f))(Aqsis::CqVector3D(0.5f, 0.685829f, 0.674439f)));
}

BOOST_AUTO_TEST_CASE(CqNoise_2D_vector_Perlin_noise_test)
{
	Aqsis::CqNoise noise;
	
	BOOST_CHECK_PREDICATE(vecEquals, (noise.PGNoise2(1.0f, 3.0f))(Aqsis::CqVector3D(0.5f, 0.75016f, 0.290464f)));
}

BOOST_AUTO_TEST_CASE(CqNoise_2D_vector_Perlin_periodic_noise_test)
{
	Aqsis::CqNoise noise;
	
	BOOST_CHECK_PREDICATE(vecEquals, (noise.PGPNoise2(1.0f, 2.0f, 3.0f, 2.0f))(Aqsis::CqVector3D(0.5f, 0.736767f, 0.490799f)));
}

BOOST_AUTO_TEST_CASE(CqNoise_3D_vector_Perlin_noise_test)
{
	Aqsis::CqNoise noise;
	
	BOOST_CHECK_PREDICATE(vecEquals, (noise.PGNoise3(Aqsis::CqVector3D(1.0f, 3.0f, 2.0f)))(Aqsis::CqVector3D(0.5f, 0.515687f, 0.393374f)));
}

BOOST_AUTO_TEST_CASE(CqNoise_3D_vector_Perlin_periodic_noise_test)
{
	Aqsis::CqNoise noise;
	
	BOOST_CHECK_PREDICATE(vecEquals, (noise.PGPNoise3(Aqsis::CqVector3D(1.0f, 2.0f, 3.0f), Aqsis::CqVector3D(1.0f, 2.0f, 3.0f)))(Aqsis::CqVector3D(0.5f, 0.724586f, 0.545074f)));
}

BOOST_AUTO_TEST_CASE(CqNoise_4D_vector_Perlin_noise_test)
{
	Aqsis::CqNoise noise;
	
	BOOST_CHECK_PREDICATE(vecEquals, (noise.PGNoise4(Aqsis::CqVector3D(1.0f, 3.0f, 2.0f), 3.0f))(Aqsis::CqVector3D(0.5f, 0.575012f, 0.320111f)));
}

BOOST_AUTO_TEST_CASE(CqNoise_4D_vector_Perlin_periodic_noise_test)
{
	Aqsis::CqNoise noise;
	
	BOOST_CHECK_PREDICATE(vecEquals, (noise.PGPNoise4(Aqsis::CqVector3D(1.0f, 2.0f, 3.0f), 2.0f, Aqsis::CqVector3D(1.0f, 2.0f, 3.0f), 2.0f))(Aqsis::CqVector3D(0.5f, 0.62703f, 0.349767f)));
}

BOOST_AUTO_TEST_CASE(CqNoise_1D_color_Perlin_noise_test)
{
	Aqsis::CqNoise noise;
	
	BOOST_CHECK_PREDICATE(colEquals, (noise.CGNoise1(1.0f))(Aqsis::CqColor(0.5f, 0.745078f, 0.730963f)));
}

BOOST_AUTO_TEST_CASE(CqNoise_1D_color_Perlin_periodic_noise_test)
{
	Aqsis::CqNoise noise;
	
	BOOST_CHECK_PREDICATE(colEquals, (noise.CGPNoise1(1.0f, 2.0f))(Aqsis::CqColor(0.5f, 0.685829f, 0.674439f)));
}

BOOST_AUTO_TEST_CASE(CqNoise_2D_color_Perlin_noise_test)
{
	Aqsis::CqNoise noise;
	
	BOOST_CHECK_PREDICATE(colEquals, (noise.CGNoise2(1.0f, 3.0f))(Aqsis::CqColor(0.5f, 0.75016f, 0.290464f)));
}

BOOST_AUTO_TEST_CASE(CqNoise_2D_color_Perlin_periodic_noise_test)
{
	Aqsis::CqNoise noise;
	
	BOOST_CHECK_PREDICATE(colEquals, (noise.CGPNoise2(1.0f, 2.0f, 3.0f, 2.0f))(Aqsis::CqColor(0.5f, 0.736767f, 0.490799f)));
}

BOOST_AUTO_TEST_CASE(CqNoise_3D_color_Perlin_noise_test)
{
	Aqsis::CqNoise noise;
	
	BOOST_CHECK_PREDICATE(colEquals, (noise.CGNoise3(Aqsis::CqVector3D(1.0f, 3.0f, 2.0f)))(Aqsis::CqColor(0.5f, 0.515687f, 0.393374f)));
}

BOOST_AUTO_TEST_CASE(CqNoise_3D_color_Perlin_periodic_noise_test)
{
	Aqsis::CqNoise noise;
	
	BOOST_CHECK_PREDICATE(colEquals, (noise.CGPNoise3(Aqsis::CqVector3D(1.0f, 2.0f, 3.0f), Aqsis::CqVector3D(1.0f, 2.0f, 3.0f)))(Aqsis::CqColor(0.5f, 0.724586f, 0.545074f)));
}

BOOST_AUTO_TEST_CASE(CqNoise_4D_color_Perlin_noise_test)
{
	Aqsis::CqNoise noise;
	
	BOOST_CHECK_PREDICATE(colEquals, (noise.CGNoise4(Aqsis::CqVector3D(1.0f, 3.0f, 2.0f), 3.0f))(Aqsis::CqColor(0.5f, 0.575012f, 0.320111f)));
}

BOOST_AUTO_TEST_CASE(CqNoise_4D_color_Perlin_periodic_noise_test)
{
	Aqsis::CqNoise noise;
	
	BOOST_CHECK_PREDICATE(colEquals, (noise.CGPNoise4(Aqsis::CqVector3D(1.0f, 2.0f, 3.0f), 2.0f, Aqsis::CqVector3D(1.0f, 2.0f, 3.0f), 2.0f))(Aqsis::CqColor(0.5f, 0.62703f, 0.349767f)));
}

BOOST_AUTO_TEST_SUITE_END()
