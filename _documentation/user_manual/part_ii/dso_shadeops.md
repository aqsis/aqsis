---
layout: documentation
title: DSO Shadeops
category: part_ii 
order: 40
subcategory: dso_shadeops
---

Like most RenderMan-compatible renderers, Aqsis has a facility for extending the
RenderMan shading language (RSL or SL) with new functions, or *shadeops*.
These new functions are compiled separately into shared libraries which are
loaded and linked dynamically by the SL compiler and the renderer. The official
name for this extension mechanism is "Dynamic Shadeops", or **DSO**, and they
are written in C or C++.

This chapter describes the classic style DSO interface.  Beginning with PRMan
13.0 released in 2006, Pixar defined a more modern plug-in interface which
exposes a more efficient, explicit SIMD-style execution of external shadeops
written in C++. Support for this new plug-in interface has not made it into
Aqsis but may do so at some time in the future.((In fact, the internal shader
execution engine in Aqsis works in a SIMD fashion. Several calls to each shadeop
are placed in a queue and issued at once. While the DSO API does not reflect
this, the SIMD-like execution gives better cache efficiency and requires less
context switching.))


### Advantages and disadvantages

The RenderMan shading language has support for functions, but like many other
implementations of the RenderMan Interface, the Aqsis shader compiler does not
actually compile a shader to machine level code.  Have a look at the `.slx`
output from the shader compiler if you like - it is actually a text file
containing a simple and regular program for a virtual machine.  That code is
parsed during the loading of a shader and executed by an interpreter during
rendering. The interpreter is fairly efficient and does a good job at executing
moderately complex shaders, but its execution speed may be slow compared to
native machine code. DSO shadeops are a solution to this problem - in SL they
look like a regular function call, but the actual code for the function is in an
external file with compiled machine code.

Briefly, the advantages of DSO shadeops include:

  * Speed!  This may be very significant for complex functions, or functions for
    which the execution model of the shaderVM is not very efficient.
  * Generality - you can do things in a DSO shadeop that would be difficult or
    even impossible to do in SL.  Anything you can do in C or C++ is allowed in
    the body of a DSO shadeop function.

DSO shadeops have several disadvantages over functions written in the shading
language:

  * There is a certain amount of overhead involved when calling a DSO shadeop.
    Therefore, simple functions that are easily implemented as a few lines of SL
    code should not be written as DSO shadeops.
  * A DSO shadeop does not have access to any of the global variables of the SL
    execution environment, and the function is evaluated on a strict
    point-by-point basis.  This means that you cannot compute derivatives or
    call any SL internal functions.  It is still possible to pass such
    information into the function from SL as explicit arguments when you need
    them.
  * Difficulty of development - writing and debugging shaders in C or C++ can be
    more time consuming than making use of a special purpose language like SL.
  * By moving the functionality of a shader into native code, you are
    circumventing the restricted, safe compilation and execution environment of
    SL and linking Aqsis with potentially insecure code. DSO shadeops are
    externally linked machine level code, and ugly things can happen if that
    code is bad or incorrect, or if it was written with malicious intent.
  * Finally, a DSO shadeop will have to be recompiled for every platform where
    you want to use it.

Despite the disadvantages, there are many situations where a DSO shadeop is an
elegant and efficient way of extending the functionality of SL. The DSO
interface to Aqsis is identical at the source code level to that of most other
RenderMan compatible renderers, including PRMan, so DSO shadeops written for
other renderers will compile for Aqsis as well, and vice versa. The compiled DSO
binaries for Aqsis should also work straight off for most other
RenderMan-compatible renderers, including PRMan.


Example
-------

To write a DSO shadeop, you need to do two things (1) write the function in C or
C++ and (2) wire the new function into SL so that the interpreter and renderer
both recognise it.

Shadeops can be written in C or C++, but if C++ is used, C style linkage should
be used, i.e. `extern "C"` should be specified in the source.

Everything can be specified in a single source code file and compiled with your
compiler of choice. There is a header file `shadeop.h` that provides the
necessary type definitions and some convenient macros. You need to define a
special data structure that maps the SL syntax to your function names and
resolves the polymorphism of SL into plain C function names for each of the
overloaded versions of a function, and you need to pass input and output
parameters through a special data structure. This is probably best explained by
a simple example.

Assume we have a new and interesting 2D and 3D noise function we want to use in
a shader, and that we have two versions of it already written in C with the
following declarations:

    float f_newnoise2f(float x, float y);
    float f_newnoise3f(float x, float y, float z);

For the purpose of this example, we will not clutter the presentation with
actually useful implementations of `f_newnoise2f` and `f_newnoise3f`. If you
want to see some useful examples of DSO noise functions, please refer to
http://www.itn.liu.se/~stegu/aqsis/DSOs/. If you only want to compile this
example to see that it works, you can use these dummy implementations and place
them in a file named `newnoise.c`:

    float f_newnoise2f(float x, float y) {
      return 0.3f;
    }
    float f_newnoise3f(float x, float y, float z) {
      return 0.7f;
    }

In SL, we want the function to be named `newnoise()`, and we want the 3D
version to take a `point` argument instead of three `float` arguments,
similar to the SL built-in `noise()` function. The full source code to connect
the two existing C functions above to a DSO shadeop would be:

    #include <shadeop.h>

    extern float f_newnoise2f(float x, float y);
    extern float f_newnoise3f(float x, float y, float z);

    SHADEOP_TABLE (newnoise) = {
        { "float f_newnoiseP (point)", "", ""},
        { "float f_newnoiseFF (float, float)", "", ""},
        { "", "", "" }
    };

    SHADEOP (f_newnoiseFF) {
        float *result = (float *)argv[0];
        float *x = (float *) argv[1];
        float *y = (float *) argv[2];

        *result = f_newnoise2f(*x, *y);

        return 0;
    }

    SHADEOP (f_newnoiseP) {
        float *result = (float *)argv[0];
        float *P = (float *) argv[1];

        *result = f_newnoise3f(P[0], P[1], P[2]);

        return 0;
    }

The `SHADEOP_TABLE` is what connects the polymorphic variants of the function
in SL to their non-polymorphic, uniquely named counterparts in C. You might
expect that using C++ instead could take advantage of its native polymorphism to
remove some of the hassle with naming the functions, but the actual C or C++
function declarations for each of the overloaded functions are in fact
identical, they differ *only* by name, as we shall see below.

The `SHADEOP_TABLE` structure is an array of structs of strings, where each
entry of three strings specifies one overloaded version of the SL function with
the name `newnoise`. The first string is the function signature, using SL
syntax for the return type and argument list, but with the C function name. The
second and third strings are initialisation and cleanup functions, which we will
cover shortly. Our `newnoise()` function does not require any initialisation
or cleanup, so we set these entries to empty strings. A line with an empty
string as its first entry marks the end of the list. This explicit end-of-list
marker is required.

In the rest of the source file, all the functions named in the `SHADEOP_TABLE`
are defined, with the aid of the macro `SHADEOP()`. It actually expands to the
following function signature:

    int f_newnoiseFF(void *initdata, int argc, void **argv)

Thus, the arguments to any DSO shadeop function are always the same: a pointer
to initialisation data (which is not used by this example but will be covered
shortly), and a `main()`-style argument list with an integer `argc`
specifying the number of arguments and an array of pointers `%%**argv%%` to
those arguments. `argv[0]` points to the return value and should be written to
if the SL function has a return value. If not, its value is either `NULL` or
undefined, and it should not be used. `argv[1]` and up are the SL arguments,
in the order they were declared in the `SHADEOP_TABLE` structure. A `float`
in SL is sent as a `*float`. A `point`, `vector`, `normal` or `color`
in SL is stored as a `float[3]`, i.e. the pointer actually points to an array
of three `float` values. Strings in SL are sent as a special data structure,
which will be covered shortly.

The return value of the C function is an integer, where 0 means that the
function executed normally, and 1 signals an error.

### Compilation 

To compile your DSO shadeop, you need to compile its C or C++ source into a
shared object library for your platform (a `.so` file in Linux, a `.dll`
file in Windows.) In Linux, you would do it like this using `gcc` (assuming
`shadeop.h` is in the current directory, if not you probably need to point to
it with the `-I` option to the compiler):

    gcc -shared newnoise.c -o newnoise.so

You can also split it up into separate compilation and linking if you want more
control:

    gcc -c newnoise.c
    gcc -shared newnoise.o -o newnoise.so

In Windows you can do it like this using the free Windows compiler from
Microsoft:

    cl.exe /DWIN32 /LD newnoise.c

or you could do it exactly like in Linux by using the Mingw32 compiler, a
Windows port of GNU gcc:

    gcc -shared newnoise.c -o newnoise.dll

In MacOS X, dynamic libraries work a little differently, so you need to use the
option `-dynamiclib` for gcc. Also, for Aqsis to find the shadeop you need the
file name to end with `.so` rather than the default `.dylib`:

    gcc -dynamiclib newnoise.c -o newnoise.so

Of course, if the source file does not include the actual implementation of the
functions, you need to compile that as well and link it with the DSO shadeop
file when you create the shared library. The name of the library file is not
really important, but it is wise to name it after the shadeop it implements, to
make it clear what it contains.

### Using the example shadeop 

When you compile a shader containing your new shadeop using `aqsl`, it doesn't
know about your DSO shadeop, so you will receive a warning that your function
will be treated as a DSO external function during rendering. This is OK, you
just need to make your DSO file available to the renderer.

When you render the scene , you need to have the DSO file in a directory where
the renderer looks for external shadeops.  For `aqsis`, the DSO search path is
the same as the [[doc:options#options|shader search path]] (this behaviour is
similar to other RenderMan-compliant renderers).  At runtime, `aqsis` looks
for all files in the DSO search path with extensions indicating a shared library
(`.so` on Unix, `.dll` on windows).  The first file containing a shadeop
table symbol with the correct name is dynamically loaded and the methods
described in the table are executed on request from the compiled SL shader.

In terms of the example, this means that something like the following command
should appear somewhere in the RIB file:

    Option "searchpath" "shader" ["/path/to/dso/shadeop:&"]

where `/path/to/dso/shadeop` is a directory containing the compiled shadeop.
The shadeop can then be used directly in a SL file, just as one would use a
built-in function, for example,

    surface newNoiseColor()
    {
            /* Calls f_newnoise2f internally */
            float f = newnoise(xcomp(P), ycomp(P));
            /* Calls f_newnoise3f internally */
            float f2 = newnoise(P);
            Ci = f + f2;
    }

DSO shadeop API reference
-------------------------

FIXME.  Make this section into a formal API reference.

Some details were not covered in the simple example above, so here's the full
story on what you can do in a DSO shadeop, and how to do it.

The `SHADEOP_TABLE` structure specifies three functions for each overloaded SL
function. The first is the actual function call, referred to as the `method`,
the second is an `init` function that is called once when the shadeop is
loaded, before it is first used by the renderer, and the last one is a
`shutdown` function that does any necessary cleanup work after the shadeop is
no longer needed, like deallocating any memory that might have been allocated in
the `init` function.

The actual function signatures, without the macros, are these:

    void *init(int ctx, void *texturectx);
    int   method(void *initdata, int argc, void **argv);
    void  shutdown(void *initdata);

The `ctx` parameter is an integer that identifies the shader execution thread.
In a multi-threaded renderer, there might be several instances of the same
shader running in parallel, and it is necessary to distinguish between them.
The parameter `*texturectx` is an opaque handle to a structure that is not
actually used in current versions of RenderMan compatible renderers. Its
intended use is for a future extension to make it possible for a DSO shadeop to
do filtered texture lookups. A typical `init` function will not make any use
of either of these parameters, nor save them anywhere. The `init` function
should return a pointer to a data structure containing all the data it
allocates. Some DSO examples that can be found floating around use static data
for local variables, but in a multi-threaded renderer where several instances of
the same shader are running, this will cause serious problems unless all the
data is constant and read-only.

The `method` function takes not only the `argc` and `%%**argv%%`
parameters, but also gets passed the `*initdata` pointer that was returned
from the `init` function. If you need to allocate local data structures in a
shadeop and these need to be written to during shadeop execution, use this
pointer to access your data to avoid problems. Modern renderers might well be
multi-threaded, so writing to a static local variable is not a safe and proper
way to do it. As mentioned previously, the return value from the `method`
function should be 0 for success, 1 for failure.

The `shutdown` function takes as its only argument the pointer `*initdata`
to the data structure that was returned from the `init` function. It is the
responsibility of the `shutdown` function to deallocate any memory that was
allocated, close files that might have been opened and do other cleanup tasks.
The `shutdown` function does not return any value.

Arguments to the SL function are passed as `void` pointers in the
`%%**argv%%` array. To access them, you need to know the original types and
cast them back. Below is an example of an SL function with all the currently
allowed types. ((General arrays are not yet supported as arguments to Aqsis DSO
shadeops. This might be included in a future release.))

    float example(float t; point P; vector V; normal N; color C; matrix M; string str)

In the `method` functions, the arguments would be accessed like this:

    float t = (float*) argv[1];
    float *P = (float*) argv[2];
    float Px = P[0];
    float Py = P[1];
    float Pz = P[2];
    // vector, normal and color types are handled like the point type
    float *V = (float *) argv[3];
    float Vx = V[0]; float Vy = V[1]; float Vz = V[2];
    float *N = (float*) argv[4];
    float Nx = N[0]; float Ny = N[1]; float Nz = N[2];
    float *C = (float*) argv[5];
    float Cr = C[0]; float Cg = C[1]; float Cb = C[2];
    // Matrices are most easily cast to a 1D array of 16 floats
    float *M = (float *) argv[6];
    float M00 = M[0];
    float M33 = M[15];
    // The string type is special
    STRING_DESC *strdesc = (STRING_DESC *) argv[7];
    char str* = strdesc->s;
    int len = strdesc->bufflen;

The struct `STRING_DESC` is defined in the header file `shadeop.h`. Note
that to be able to write to a string argument, i.e. if you have an output
parameter of type `string`, you need to allocate dynamic storage space
yourself for a new string and assign its pointer to the field `s` of the
`STRING_DESC` struct. The string is null terminated like any C or C++ string,
but the `bufflen` field should be set equal to the length of the character
array you allocated. Note that a string of length *n* characters needs a
character array of at least length *n*+1 to include the terminating null
character.

    char hello[] ="Hello Aqsis";
    int len = strlen(hello);
    char *outstr = malloc(len+1);
    strcpy(hello, outstr);
    strdesc->s = outstr;
    strdesc->bufflen = len;
