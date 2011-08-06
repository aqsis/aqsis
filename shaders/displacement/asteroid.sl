/*
Aqsis
Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
* Neither the name of the software's owners nor the names of its
  contributors may be used to endorse or promote products derived from this
  software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

(This is the New BSD license)
*/


/*
-----------------------------------------------------------------------
Author: Chris Foster  chris42f (at) gmail (dot) com
See www.aqsis.org
*/

/*
Turbulence - fractal brownian noise function.

Turbulence is the addition of an increasing sequence of noise frequencies,
with decreasing amplitude for the higher frequencies.  The noise is obtained
by sampling some underling noise function - here we use the standard builtin
Perlin noise function.

pos     - seed position for noise function
octaves - how many octaves of noise to use
lambda  - frequency multiplier for successive octaves.  The frequency of the
          N'th octave of noise is scaled by lambda^N.
omega   - amplitude for the N^th octave of noise is omega^N
*/
float turbulence(point pos; float octaves; float lambda; float omega)
{
    float value = 0;
    float l = 1;
    float o = 1;
    float i = 0;
    for(i=0; i < octaves; i+=1)
    {
        value += o*(2*noise(pos*l)-1);
        l *= lambda;
        o *= omega;
    }
    return value;
}


/*
Randomly distributed crater pattern.

This uses a spherical "crater" amplitude function in each unit cell.  The
centre and radius of the craters are jittered to avoid regularities.

p - shading point
jitter - how much to jitter centres of craters from regular grid
overlap - How large craters are compared to the regular grid.
          An overlap of 1 means that craters are 1/2 the size of
          the distance between craters on average.
sharpness - steepness of the crater walls (<=1).  1 == infinitely steep.
*/
float crater(point p; float jitter; float overlap; float sharpness)
{
    point centre = (floor(xcomp(p) + 0.5), floor(ycomp(p) + 0.5),
                    floor(zcomp(p) + 0.5));
    float amp = 0;
    float i,j,k;
    for(i = -1; i <= 1; i += 1)
    for(j = -1; j <= 1; j += 1)
    for(k = -1; k <= 1; k += 1)
    {
        point cellCentreIjk = centre + vector(i,j,k);
        cellCentreIjk += jitter * vector cellnoise(cellCentreIjk) - 0.5;
        float rad = 0.5*overlap*float cellnoise(cellCentreIjk);
        float d = length(p - cellCentreIjk);
        amp = min(amp, smoothstep(sharpness*rad, rad, d) - 1);
    }
    return amp;
}


/*
Displacement shader for creating an asteroid-like surface structure.

This uses a combination of various size craters and a turbulence function, it's
designed to be used with geometry the size of a radius-1 sphere, with the
output resembling an asteroid.

amplitude - scaling for displacement amount.
patternDisplace - solid texture offset.  Change this to create unique patterns.
*/
displacement asteroid(float amplitude = 0.1; vector patternDisplace=(0,0,0))
{
    point Pobj = transform("object", P) + patternDisplace;
    float amp = amplitude*(
          0.3*crater(2*Pobj + 0.1*vector noise(4*Pobj), 0.5, 1, 0.5)
        + 0.1*crater(5*Pobj + 0.2*vector noise(7*Pobj), 1, 1, 0.6)
        + 0.015*crater(20*Pobj, 1, 1, 0.8)
        + 2*turbulence(0.5*Pobj, 8, 2, 0.4)
    );
    P += normalize(N)*amp;
    N = calculatenormal(P);
}

