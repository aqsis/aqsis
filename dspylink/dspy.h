//
// Define and declare the various structures needed to communicate with dspy style display drivers.
//

#ifndef	DSPY_H_INCLUDED
#define	DSPY_H_INCLUDED

#define PkDspyNone		0
#define PkDspyFloat32		1
#define PkDspyUnsigned32	2
#define PkDspySigned32		3
#define PkDspyUnsigned16	4
#define PkDspySigned16		5
#define PkDspyUnsigned8		6
#define PkDspySigned8		7
#define PkDspyString		8
#define PkDspyMatrix		9
#define PkDspyArrayBegin	10
#define PkDspyArrayEnd	        11

typedef unsigned long PtDspyUnsigned32;
typedef long PtDspySigned32;
typedef double PtDspyFloat64;
typedef float PtDspyFloat32;
typedef unsigned short PtDspyUnsigned16;
typedef short PtDspySigned16;
typedef unsigned char PtDspyUnsigned8;
typedef char PtDspySigned8;

typedef void *PtDspyImageHandle;

typedef struct
{
	PtDspyUnsigned32 width;
	PtDspyUnsigned32 height;
	PtDspyFloat32 aspectRatio;
} PtDspySizeInfo;

typedef struct
{
  PtDspyUnsigned8 overwrite;
  PtDspyUnsigned8 interactive;
} PtDspyOverwriteInfo;

typedef enum
{
	PkSizeQuery,
	PkOverwriteQuery,
	PkNextDataQuery,
	PkRedrawQuery,
        PkRenderingStartQuery,
        PkSupportsCheckpointing
} PtDspyQueryType;

typedef struct uparam {
	RtToken		name;
	char		vtype, vcount;
	RtPointer	value;
	int		nbytes;
} UserParameter;

typedef struct
{
   char *name;
   unsigned type;
} PtDspyDevFormat;

#define PkDspyFlagsWantsScanLineOrder 1
#define PkDspyFlagsWantsEmptyBuckets 2
#define PkDspyFlagsWantsNullEmptyBuckets 4
typedef struct
{
	int flags;
} PtFlagStuff;

typedef enum
{
	PkDspyErrorNone = 0,
	PkDspyErrorNoMemory,
	PkDspyErrorUnsupported,
	PkDspyErrorBadParams,
	PkDspyErrorNoResource,
	PkDspyErrorUndefined
} PtDspyError;
	

typedef PtDspyError (*DspyImageOpenMethod)(PtDspyImageHandle*,const char*,const char*,int,int,int,const UserParameter*,int,PtDspyDevFormat*,PtFlagStuff*);
typedef PtDspyError (*DspyImageQueryMethod)(PtDspyImageHandle,PtDspyQueryType,size_t,void*);
typedef PtDspyError (*DspyImageDataMethod)(PtDspyImageHandle,int,int,int,int,int,const unsigned char*);
typedef PtDspyError (*DspyImageCloseMethod)(PtDspyImageHandle);
typedef PtDspyError (*DspyImageDelayCloseMethod)(PtDspyImageHandle);

#endif // DSPY_H_INCLUDED
