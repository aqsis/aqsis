% Matlab script to test image filtering for mipmap creation in Aqsis
% Copyright (C) 2007, Christopher J. Foster
%
% Redistribution and use in source and binary forms, with or without
% modification, are permitted provided that the following conditions are met:
%
% * Redistributions of source code must retain the above copyright notice,
%   this list of conditions and the following disclaimer.
% * Redistributions in binary form must reproduce the above copyright notice,
%   this list of conditions and the following disclaimer in the documentation
%   and/or other materials provided with the distribution.
% * Neither the name of the software's owners nor the names of its
%   contributors may be used to endorse or promote products derived from this
%   software without specific prior written permission.
%
% THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
% AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
% IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
% ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
% LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
% CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
% SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
% INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
% CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
% ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
% POSSIBILITY OF SUCH DAMAGE.
%
% (This is the New BSD license)

%----------------------------------------------------------------------
function img_filter()

%im = double(imread('lena_std.tif'))/255;
%im = double(imread('kitten_tst.png'))/255;
im = getGridImg(128,21);

%investigateWindowTypes(im, @sincKer);
%plotWindowFuncs(10);
compareOddEven(im);

%windowFunc = makeWinWithoutZeros(@lanczos);
%windowFunc = @(N) lanczos(N,1.6);
%kerFunc = @sincKer;
%
% Investigate factor of 2 vs factor of 4 downsampling
%mipmap = genMipmapIter(im, 7, 1, windowFunc, kerFunc);
%mipmap = genMipmapIterDownsamp4(im, 7, 1, windowFunc, kerFunc);
%
%imwrite(cellarrayToImage(mipmap(2:end),2), 'mipmap.png','png');

%image(cellarrayToImage(mipmap,1));
%axis image;
%axis off;


%----------------------------------------------------------------------
% High level driver functions.
%----------------------------------------------------------------------
function compareOddEven(im)
% Compare the difference between sampling an image with an odd or even-sized
% filter.

kerFunc = @sincKer;
windowFunc = makeWinWithoutZeros(@lanczos);

% Use an odd-width filter
mipmap = genMipmapIter(im, 7, 0, windowFunc, kerFunc);
imwrite(cellarrayToImage(mipmap,2), 'mipmap_odd_width.png','png');

%pause(4);
% Use an even-width filter
mipmap = genMipmapIter(im, 8, 0, windowFunc, kerFunc);
imwrite(cellarrayToImage(mipmap,2), 'mipmap_even_width.png', 'png');


%----------------------------------------------------------------------
function investigateWindowTypes(im, kerFunc)
% Grab the level-three mipmaps arising from using a bunch of windows with
% a fixed filter kernel.

% Generate a version using the boxcar kernel [1,1] for reference.
mipmap = genMipmapIter(im, 1, 1, @rectwin, @boxKer);
miplevel3{1} = mipmap{3};

winFuncs = {@lanczos, @hamming, @hanning, @blackman, @rectwin, @kaiser};
% Generate mipmaps using the other window functions
for ii=1:length(winFuncs)
  mipmap = genMipmapIter(im, 7, 1, winFuncs{ii}, kerFunc);
  miplevel3{ii+1} = mipmap{3};
end

imwrite(cellarrayToImage(miplevel3,2), 'mipmap_lvl3_all_wins.png','png');


%----------------------------------------------------------------------
function plotWindowFuncs(N)
% Plot various window functions.
winFuncs = {@lanczos, @hamming, @hanning, @blackman, @rectwin, @kaiser};
winFuncNames = {'lanczos,' 'hamming,' 'hanning,' 'blackman', 'boxcar', 'kaiser'};
clf();
hold on;
for ii=1:length(winFuncs)
  plot(winFuncs{ii}(N), rnd_ln_style(ii));
end
legend(winFuncNames);
axis([1,N,0,1.05]);
set(gca(),'box','on');


%----------------------------------------------------------------------
% Functions for creating/manipulating mipmaps
%----------------------------------------------------------------------
function mipmap = genMipmapIter(im, width, selectWidth, windowFunc, kerFunc)
% Generate a mipmap by downsampling each image in turn to get the next level.
%
% im - input image
% width - desired "width" of filter kernel.  the kernel will have width+1 points.
% selectWidth - whether the function should try to adjust the filter width for
%               even/odd sized images.  To avoid loosing information, even
%               filters should go with even sized images and vice versa
% windowFunc  - windowing function to use on the filter.
%

mipmap = {};

scale = 2;

ker = kerFunc(width, scale);
ker = applyWindow(ker, windowFunc);

ii = 1;
while(size(im,1) > 1)
  mipmap{ii} = im;
  ii = ii + 1;

  if(selectWidth)
    if(abs(mod(size(im,1) + width + 1, 2)) < eps(10))
      ker = kerFunc(width,scale);
    else
      ker = kerFunc(width+1,scale);
    end
    ker = applyWindow(ker, windowFunc);
  end
  disp(sprintf('side length = %d,  number of points in filter = %d', ...
               size(im,1), length(ker)));

  im = imDownsamp(im, ker, scale);
end


%----------------------------------------------------------------------
function mipmap = genMipmapIterDownsamp4(im, width, selectWidth, windowFunc, kerFunc)
% Same as genMipmapIter, except tries to downsample by a factor of 4 rather
% than two when possible.
%
% (Beware: ugly hack)

mipmap = {};

scale = 4;

scales = scale*ones(1,20);
scales(1) = 2;
widths = 2*width*ones(1,20);
widths(1) = width;

ker = kerFunc(width, scale);
ker = applyWindow(ker, windowFunc);
prevIm = im;

ii = 1;
while(size(im,1) > 1)
  scale = scales(ii);
  width = widths(ii);

  mipmap{ii} = im;
  ii = ii + 1;

  if(selectWidth)
    if(abs(mod(size(im,1) + width + 1, 2)) < eps(10))
      ker = kerFunc(width,scale);
    else
      ker = kerFunc(width+1,scale);
    end
    ker = applyWindow(ker, windowFunc);
  end
  disp(sprintf('side length = %d,  number of points in filter = %d', ...
               size(im,1), length(ker)));

  im = imDownsamp(mipmap{max(ii-2,1)}, ker, scale);
end


%----------------------------------------------------------------------
function mipmap = genmipmap(im)
% Generate a mipmap by applying succesively larger filter kernels to the
% original image (Note: impractically slow!)
%
% Currently broken.

mipmap = {};

imSmall = im;

scale = 2;
currScale = 2;
xpos = 1;
for ii = 1:3
  mipmap(ii) = imSmall;
  xpos = xpos + size(imSmall,1);
  ker = sincKer(2*currScale,currScale);
%  window = sin(linspace(0,pi,length(ker)));
%  window = window(:) * window(:).';
%  ker = ker .* window;
  disp(ii);

  imSmall = imDownsamp(im, ker, currScale);

  currScale = currScale*scale;
end


%----------------------------------------------------------------------
function mipmapImg = cellarrayToImage(mipmap, dim)
% "Flatten" mipmap levels into a single image.
%
% mipmap - cell array of scaled images
% dim - dimension number to lay the images out along

len = 0;
for ii=1:length(mipmap)
  len = len + size(mipmap{ii},dim);
  %disp(size(mipmap{ii},dim));
end

mipSize  = size(mipmap{1});
mipSize(dim) = len;

mipmapImg = ones(mipSize)*0.7;
pos = 1;
for ii = 1:length(mipmap)
  if dim == 1
    mipmapImg(pos:pos+size(mipmap{ii},1)-1, 1:size(mipmap{ii},2), :) = mipmap{ii};
  else
    mipmapImg(1:size(mipmap{ii},1), pos:pos+size(mipmap{ii},2)-1, :) = mipmap{ii};
  end
  pos = pos + size(mipmap{ii},1);
end


%----------------------------------------------------------------------
%----------------------------------------------------------------------
% Windowing functions
%----------------------------------------------------------------------
function win = lanczos(N, tau)
% return N-point Lanczos window with width tau
if nargin == 1
  tau = 1;
end
win = sinc(linspace(-1,1,N)/tau);


%----------------------------------------------------------------------
function win = makeWinWithoutZeros(winFunc)
% Return a window function constructed from the given window function, but
% without the two edge points (see natWin).
win = @(N, varargin) natWin(N, winFunc, varargin{:});


function win = natWin(N, winFunc, varargin)
% Windowing functions often include zeros at the edges.  This is a waste of
% computational effort, since convolution with zeros doesn't achieve anything.
%
% This function removes the two end points in any window.
win = winFunc(N+2, varargin{:});
win = win(2:end-1);

%----------------------------------------------------------------------
function kerWin = applyWindow(ker, windowFunc)
% Window a filter kernel with the given windowing function.

window = windowFunc(length(ker));
window = window(:) * window(:).';
kerWin = ker .* window;
%kerWin = ker


%----------------------------------------------------------------------
%----------------------------------------------------------------------
% Filter Kernels
%----------------------------------------------------------------------
function k = sincKer(width, scale)
% Return a sinc filter kernel with specified width and scale
%
% width - total number of points in the filter (not radius!)
% scale - zeros will be centered around 0 with spacing "scale"

x = (0:width)-width*0.5;
k = sinc(x/scale)' * sinc(x/scale);


%----------------------------------------------------------------------
function k = boxKer(width, scale)
% Trivial box filter kernel

k = ones(width+1);


%----------------------------------------------------------------------
%----------------------------------------------------------------------
% Image manipuation
%----------------------------------------------------------------------
function res = imConv(im, ker)
% Convolve an image with the given filter kernel

res = zeros(size(im));
normalisation = conv2(ones(size(im,1), size(im,2)), ker, 'full');
% compute how much of the full convolution to trim from top left and bottom right
trim_tl = ceil((size(ker)-1)/2);
trim_br = floor((size(ker)-1)/2);
for ii=1:size(im,3)
  fullconv = conv2(im(:,:,ii), ker, 'full')./normalisation;
  res(:,:,ii) = fullconv(1+trim_tl(1):end-trim_br(1), 1+trim_tl(2):end-trim_br(2));
end


%----------------------------------------------------------------------
function res = imDownsamp(im, ker, skip)
% Downsample an image by
% (1) Applying the filter kernel ker at each pixel
% (2) Decimate by taking elements 1:skip:end from the result
% (3) Clamp the resulting intensities between 0 and 1 

res = imConv(im,ker);
res = res(1+floor((skip-1)/2):skip:end,1+floor((skip-1)/2):skip:end,:);
res = clamp(res);


%----------------------------------------------------------------------
function trilinear()
% Trilinear interpolation (not implemented...)
blah();


%----------------------------------------------------------------------
function res = clamp(im)
% Clamp an image between zero and one.
res = min(max(im,0),1);


%----------------------------------------------------------------------
function im = getGridImg(N, gridSize)
% Get a test image containig a one pixel wide grid 
%
% N - side length
% gridSize - size of the one-pixel wide black grid in the image.

im = zeros(N, N, 3);

x = linspace(0,1,N);
[xx,yy] = ndgrid(x,x);
im(:,:,1) = xx;
im(:,:,2) = 0.5;
im(:,:,3) = yy;

im(1:gridSize:end-gridSize+1,:,:) = 0;
im(:,1:gridSize:end-gridSize+1,:) = 0;

im(:,end,:) = 0;
im(end,:,:) = 0;


%----------------------------------------------------------------------
% Misc
%----------------------------------------------------------------------
function lineStyle = rnd_ln_style(idx, colOverride)
% lsty = rnd_ln_style(idx, colOverride)
%
% Return a 'random' line style which may be used to usefully distinguish
% between a large number of curves.

lineDashes = {'-','--','-.',':'};
numDashed = length(lineDashes);
lineColors = {'b','k','r'};
if nargin >= 2
  lineColors = {colOverride};
end
numCols = length(lineColors);

idx = idx - 1;
lineStyle = [lineColors{mod(idx, numCols)+1}, lineDashes{mod(idx, numDashed)+1}];
