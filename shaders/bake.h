/* bake.h - helper macro for texture baking */
#define BAKE(name,s,t,func,dest) \
{ \
string objname = ""; \
attribute ("identifier:name", objname); \
string passname = ""; \
option ("user:pass", passname); \
uniform float bakingpass = match ("bake", passname); \
color foo; \
if (bakingpass != 0) { \
dest = func(s,t); \
string bakefilename = concat (objname, ".", bakename, ".bake"); \
bake (bakefilename, s, t, dest); \
} else { \
string filename = concat (objname, ".", bakename, ".tx"); \
dest = texture (filename, s, t); \
}\
}
