/************************************************************************/
/*									*/
/*	File:	pp.h							*/
/*									*/
/*	Header file for PP.						*/
/*									*/
/*	Written by:							*/
/*			Gary Oliver					*/
/*			3420 NW Elmwood Dr.				*/
/*			PO Box 826					*/
/*			Corvallis, Oregon 97339				*/
/*			(503)758-5549					*/
/*	Maintained by:							*/
/*			Kirk Bailey					*/
/*			Logical Systems					*/
/*			P.O. Box 1702					*/
/*			Corvallis, OR 97339				*/
/*			(503)753-9051					*/
/*									*/
/*	This program is hereby placed in the public domain.  In		*/
/*	contrast to other claims of "public domain", this means no	*/
/*	copyright is claimed and you may do anything you like with PP,	*/
/*	including selling it!  As a gesture of courtesy, please retain	*/
/*	the authorship information in the source code and		*/
/*	documentation.							*/
/*									*/
/************************************************************************/

#define	VERSION		"Logical Systems Version 96.1"	/* Version info */
#ifdef	MAIN
#define	EXTERN			/* Inside main() routine, make internal */
#define	I_BRZERO	={0}
#define	I_ZERO		=0
#else	/* !MAIN */
#define	EXTERN		extern	/* Define for creating external refs	*/
#define	I_BRZERO
#define	I_ZERO
#endif	/* MAIN */

/*	Define TRUE and FALSE if not already defined			*/

#ifndef	TRUE
#define	TRUE		1
#endif	/* TRUE */

#ifndef	FALSE
#define	FALSE		0
#endif	/* FALSE */

/*
 *	Define AND and OR as logical "and" and "or", or boolean if the
 *	preprocessor being used to compile PP doesn't support "&&" and "||" in
 *	preprocessor directives (at least one version of Lattice for instance).
 */

#define	AND		&&
#define	OR		||

/*
 *	If the value of PP is FALSE, this program may be run through
 *	the original QC compiler without parameterized macro expansion.
 *	After a version is built, recompile with PP as TRUE to make
 *	things run more efficiently (ctype.h, for example.)
 */

#define	PP		TRUE	/* TRUE if to depend upon PP features	*/

#define	DEBUG		FALSE	/* TRUE if to include debug code	*/

/*
 *	The host system the preprocessor will be running on.  Note that this
 *	choice implies both the host operating system and the specific "C"
 *	compiler used to compile PP.  Note that not all combinations have
 *	been (recently), tested!
 */

#define	H_BSD		1	/* BSD 4.X UNIX				*/
#define	H_CPM		2	/* QC Z80 native compiler under CP/M	*/
#define	H_MPW		3	/* MPW on Mac's				*/
#define	H_MSDOS		4	/* MSC or Borland "C" on MS-DOS		*/
#define	H_UNIX		5	/* AT+T System 5 UNIX			*/
#define	H_VMS		6	/* DEC VAX VMS				*/
#define H_XENIX		7	/* Microsoft XENIX			*/

#define	HOST		H_MSDOS

/*
 *	The target system for which the preprocessor is generating code.  Note
 *	that this choice implies both the target "operating system" (if any),
 *	and the	target processor.  In particular, this choice affects which
 *	symbols PP predefines.  Note that not all combinations have been
 *	(recently), tested!
 */

#define	T_BSD		1	/* BSD 4.X UNIX				*/
#define	T_MPW		2	/* MPW "C" on Mac's			*/
#define	T_QC		3	/* QC "C"				*/
#define	T_QCX		4	/* QCX "C" cross compiler		*/
#define	T_TCX		5	/* TCX "C" cross compiler		*/
#define	T_UNIX		6	/* AT+T System 5 UNIX			*/
#define	T_VMS		7	/* DEC VAX VMS				*/
#define T_XENIX		8	/* Microsoft XENIX			*/

#define	TARGET		T_TCX

#if	((HOST == H_BSD) OR (HOST == H_UNIX) OR (HOST == H_XENIX) || (defined(WIN32)))
#define	STDERR	stderr
#else	/* ! (H_BSD OR H_UNIX OR H_XENIX) */
#define	STDERR	stdout
#endif	/* H_BSD OR H_UNIX OR H_XENIX */

#if	HOST != H_MPW
#define	VERBOSE	TRUE		/* TRUE to make verbose by default	*/
#else	/* MPW */
#define	VERBOSE	FALSE		/* TRUE to make verbose by default	*/
#endif	/* HOST != H_MPW */

#if	HOST == H_CPM
#include	<stdio.h>	/* Standard stuff			*/
#include	<string.h>

#if	PP
#include	<ctype.h>	/* Character types			*/
#endif	/* PP */

#define	EVALINT		int	/* For QC: evaluate using ints		*/
#define	PP_SYSIO		/* Defined if to use direct I/O routines*/
#define	BUFFERSIZE	512	/* Disk buffer size			*/
#define	OUTBUFSIZE	2048	/* Output buffer size			*/
#define	strchr		index	/* ANSI C defs				*/
#define	strrchr		rindex
#define	DFLT_PATH	"A0"	/* Enables -I code			*/
#define	PATHFILE	"path.ppi"	/* Location of path info	*/
#define	ENDFILE	(0x1A)		/* CP/M End of file character		*/
#define	BDOS_SELDISK	14	/* Bdos select disk function		*/
#define	BDOS_GETDISK	25	/* Bdos get disk function		*/
#define	BDOS_USER	32	/* Bdos get/set user function		*/
#endif	/* HOST == H_CPM */

#if	HOST != H_CPM

#define	ENV_PATH	"PPINC"	/* Env. variable for include path	*/
#if	HOST == H_MSDOS
#define	DFLT_PATH	"/include"	/* Default include location	*/
#else	/* ! H_MSDOS */
#if	HOST == H_VMS
#define	DFLT_PATH	"crel$include:"	/* Default include location	*/
#else	/* !H_VMS */
#ifndef DFLT_PATH		/* If MAKEFILE doesn't override		*/
#define	DFLT_PATH	"/usr/include"	/* Default include location	*/
#endif	/* !defined(DFLT_PATH) */
#endif	/* HOST == H_VMS */
#endif	/* HOST == MS_DOS */

#define	PP_SYSIO		/* Defined if to use direct I/O routines*/
#if	HOST == H_BSD
#include	<sys/param.h>
#define	BUFFERSIZE	BLKDEV_IOSIZE	/* 2K bytes Ultrix2.2/SunOS4.0	*/
#else	/* ! H_BSD */
#define	BUFFERSIZE	512	/* Disk buffer size			*/
#endif	/* HOST == H_BSD */
#include	<stdio.h>	/* Standard I/O info			*/
#include	<stdlib.h>	/* Standard library info		*/
#include	<string.h>
#include	<ctype.h>	/* Char type info			*/

#if	HOST == H_XENIX
#include	<sys/types.h>	/* Need for 'time_t'			*/
#endif	/* HOST == H_XENIX */

#if	HOST == H_MPW
#define	SLASHCHAR	':'
#define	SLASHSTR	":"
#else	/* ! H_MPW */
#define	SLASHCHAR	'/'
#if	HOST == H_VMS
#define	SLASHSTR	""
#else	/* ! H_VMS */
#define	SLASHSTR	"/"
#endif	/* HOST == H_VMS */
#include	<time.h>	/* System time info			*/
#endif	/* HOST == H_MPW */

#if	HOST == H_BSD		/* Berkeley 4.x				*/
#include	<sys/types.h>
#define	strchr		index
#define	strrchr		rindex
#define	memmov		bcopy	/* Used move memory routine		*/
#endif	/* HOST == H_BSD */

#endif	/* HOST != H_CPM */

#define	PATHPUNC	';'	/* Include file path separator		*/

#if	(HOST!=H_BSD)AND(HOST!=H_MPW)AND(HOST!=H_UNIX)AND(HOST!=H_XENIX)
#define	IGNORE_CR		/* Defined if to ignore CR's		*/
#endif	/* !H_BSD AND !H_MPW AND !H_UNIX AND !H_XENIX */

#ifndef	EVALINT
#define	EVALINT		long	/* Default values in eval are longs	*/
#endif	/* EVALINT */

/*
 *	Character type.
 */

#define	C_L		0x01	/* Letter (A-Zz-z)			*/
#define	C_D		0x02	/* Digit (0-9)				*/
#define	C_X		0x04	/* Hex digit (0-9 a-f|A-F)		*/
#define	C_W		0x08	/* White space (ht, sp, etc.)		*/ 
/*			0x10	   Not used				*/
#define	C_C		0x20	/* Special processing inside gchbuf()	*/
#define	C_M		0x40	/* A META token (see gettoken())	*/
#define	C_N		0x80	/* A NUL (no output)			*/

#if	PP
#define	istype(c,v)	((typetab+1)[(c)]&(v))
extern	int	gchfile();
#else	/* !PP */
extern	int	istype();
#endif	/* PP */

#ifdef	MAIN
char	typetab[] =
    {
        ( char ) ( C_C | C_N ), 					     /* EOF         */
        0, 0, 0, 0, 	     /* ^@ ^A ^B ^C */
        0, 0, 0, 0, 	     /* ^D ^E ^F ^G */
        0, C_W, C_C, 0, 	     /* ^H ^I ^J ^K */
#if	HOST == H_MPW
        C_W, C_C, 0, 0, 	     /* ^L ^M ^N ^O */
#else	/* ! H_MPW */
        C_W, C_W | C_C, 0, 0, 	     /* ^L ^M ^N ^O */
#endif	/* HOST == H_MPW */
        0, 0, 0, 0, 	     /* ^P ^Q ^R ^S */
        0, 0, 0, 0, 	     /* ^T ^U ^V ^W */
#if	HOST == H_CPM
        0, 0, C_C, 0, 	     /* ^X ^Y ^Z ESC*/
#else	/* ! H_CPM */
        0, 0, 0, 0, 	     /* ^X ^Y ^Z ESC*/
#endif	/* HOST == H_CPM */
        0, 0, 0, 0, 	     /* FS GS RS VS */
        C_W, 0, 0, 0, 	     /* Sp !  "  #  */
        0, 0, 0, 0, 	     /* $  %  &  '  */
        0, 0, 0, 0, 	     /* (  )  *  +  */
        0, 0, 0, 0, 	     /* ,  -  .  /  */
        C_D | C_X, C_D | C_X, C_D | C_X, C_D | C_X,      /* 0  1  2  3  */
        C_D | C_X, C_D | C_X, C_D | C_X, C_D | C_X,      /* 4  5  6  7  */
        C_D | C_X, C_D | C_X, 0, 0, 	     /* 8  9  :  ;  */
        0, 0, 0, 0, 	     /* <  =  >  ?  */
        0, C_L | C_X, C_L | C_X, C_L | C_X,      /* @  A  B  C  */
        C_L | C_X, C_L | C_X, C_L | C_X, C_L, 	     /* D  E  F  G  */
        C_L, C_L, C_L, C_L, 	     /* H  I  J  K  */
        C_L, C_L, C_L, C_L, 	     /* L  M  N  O  */
        C_L, C_L, C_L, C_L, 	     /* P  Q  R  S  */
        C_L, C_L, C_L, C_L, 	     /* T  U  V  W  */
        C_L, C_L, C_L, 0, 	     /* X  Y  Z  [  */
        C_C, 0, 0, C_L, 	     /* \  ]  ^  _  */
        0, C_L | C_X, C_L | C_X, C_L | C_X,      /* `  a  b  c  */
        C_L | C_X, C_L | C_X, C_L | C_X, C_L, 	     /* d  e  f  g  */
        C_L, C_L, C_L, C_L, 	     /* h  i  j  k  */
        C_L, C_L, C_L, C_L, 	     /* l  m  n  o  */
        C_L, C_L, C_L, C_L, 	     /* p  q  r  s  */
        C_L, C_L, C_L, C_L, 	     /* t  u  v  w  */
        C_L, C_L, C_L, 0, 	     /* x  y  z  {  */
        0, 0, 0, 0, 	     /* |  }  ~  Rub*/
        /* Special token chars 0x80 - 0x??	*/
        ( char ) ( C_M | C_N ), 					/* END_MACRO token */
        ( char ) ( C_M | C_N ), 					/* END_ARG token   */
        ( char ) ( C_M | C_N ), 					/* TOGGLE_EXPAND   */
        ( char ) ( C_M | C_N ), 					/* __LINE__ token  */
        ( char ) ( C_M | C_N ), 					/* __FILE__ token  */
        ( char ) ( C_M | C_N ), 					/* __TIME__ token  */
        ( char ) ( C_M | C_N ), 					/* __DATE__ token  */
        ( char ) ( C_M | C_N ), 					/* __NOW__ token   */
        ( char ) ( C_M | C_N ), 					/* __NEXT__ token  */
        ( char ) ( C_M | C_N ) 					/* __PREV__ token  */
    };
#else
extern	char	typetab[];
#endif	/* MAIN */

#define EQUAL		0	/* Value returned by strcmp if equal	*/

/*
 *	Token types from gettoken()
 */

#define LETTER		'a'	/* (letter | '_') letter* digit*	*/
#define NUMBER		'0'	/* (digit | '.' | letter) digit+	*/
#define	DIRECTIVE_CHAR	'#'	/* Defines a PP directive		*/

#define	END_MACRO	0x80	/* Marks end of macro in pushback	*/
#define	TOGGLE_EXPAND	0x81	/* Marks edges of previously expanded	*/
#define	END_ARG		0x82	/* Marks end of argument macro scope	*/
#define	LINE_TOKEN	0x83	/* Return the Line number		*/
#define	FILE_TOKEN	0x84	/* Return the File name			*/
#define	TIME_TOKEN	0x85	/* Return the system time		*/
#define	DATE_TOKEN	0x86	/* Return the system date		*/
#define	NOW_TOKEN	0x87	/* Return the last result of NEXT_TOKEN	*/
#define	NEXT_TOKEN	0x88	/* Return a unique unsigned number	*/
#define	PREV_TOKEN	0x89	/* Return previous unique unsigned #	*/

/*
 *	Flag parameter to "gettoken()".
 */

#define	GT_STR		0x0001	/* gettoken() returns whole strings	*/
#define	GT_ANGLE	0x0003	/* GT_STR plus returns whole <>'s	*/

#if	HOST == H_CPM
#define FILENAMESIZE	20
#else	/* ! H_CPM */
#if	HOST == H_BSD
#define	FILENAMESIZE	MAXPATHLEN
#else	/* ! H_BSD */
#define	FILENAMESIZE	80	/* Long enough until we find better	*/
#endif	/* HOST == H_BSD */
#endif	/* HOST == H_CPM */

#define FILESTACKSIZE	14		/* Max depth of #include	*/
#define MACROSIZE	40000		/* Max length of #define	*/
#define TOKENSIZE	510		/* Max token length		*/
#define NUMBUCKETS	64		/* # of buckets (power of 2)	*/
#define	IFSTACKSIZE	32		/* #ifxx nesting		*/
#define	PUSHBACKSIZE	TOKENSIZE	/* Pushback buffer		*/
#define	EVALBUFSIZE	TOKENSIZE	/* Buffer size in doif()	*/
#define	MESSAGEBUFSIZE	TOKENSIZE	/* Buffer size in #pragma mess	*/
#define	PRAGBUFSIZE	TOKENSIZE	/* Room for pragma line		*/

/*
 *	Pushback buffer
 */

EXTERN	struct	pbbuf	*Pbbufp	I_ZERO;
EXTERN	struct	pbbuf	*Pbbuf	I_ZERO;

struct pbbuf
{
    char	pb_type;
#define	PB_CHAR		0	/* pb_val.pb_char is character		*/
#define	PB_STRING	1	/* pb_val.pb_str is ptr to string 	*/
#define	PB_TOS		2	/* Top of pushback buffer stack		*/
    union
    {
        char	*pb_str;
        int	pb_char;
    }
    pb_val;
};

EXTERN	FILE	*Output	I_ZERO;	/* Output file				*/

struct file
{
    int	f_line;	/* Line number				*/
#if	HOST == H_CPM		/* Path names for #includes		*/
    int	f_disk;	/* Disk number				*/
    int	f_user;	/* User number (< 0 if default)		*/
#endif	/* HOST == H_CPM */
#ifdef	PP_SYSIO		/* If to use direct I/O system calls?	*/
    int	f_fd;	/* A file descriptor			*/
#else	/* !PP_SYSIO */
    FILE	*f_file; /* FILE pointer				*/
#endif	/* PP_SYSIO */
    char	*f_bufp; /* Pointer into buffer			*/
    int	f_bufc;	/* Number of chars left			*/
    int	f_eof;	/* Flag that's non-zero if in last blk	*/
    char	f_lasteol;	/* TRUE if last char was EOL	*/
    char	f_buf[ BUFFERSIZE ];	/* Buffer for file	*/
    char	f_name[ FILENAMESIZE + 1 ];	/* File name	*/
};

/*
 *	Define static vars that are a bit more efficient to access.
 */

EXTERN	int	Bufc	I_ZERO;	/* Current file char count		*/
EXTERN	char	*Bufp	I_ZERO;	/* Current file char ptr 		*/
EXTERN	int	Lasteol	I_ZERO;	/* True if last char processed was EOL	*/
EXTERN	int	LLine	I_ZERO;	/* Last line number			*/
EXTERN	int ( *Nextch ) ();	/* Next char function		*/
#define	nextch		(*Nextch)	/* Macro to rd chars via Nextch	*/

EXTERN	struct	file	*Filestack[ FILESTACKSIZE + 1 ] I_BRZERO;
EXTERN	int	Filelevel I_ZERO;	/* Include level	*/
EXTERN	int	Do_name	I_ZERO;	/* True to put name on #line	*/

EXTERN	char	Outfile[ FILENAMESIZE + 1 ] I_BRZERO;
EXTERN	char	Token[ TOKENSIZE + 1 ] I_BRZERO;	/* Token buffer	*/
EXTERN	unsigned int	Tokenline I_ZERO;	/* Line # token began	*/
EXTERN	unsigned int	Tokenfile I_ZERO;	/* File # token began	*/
EXTERN	int	Lastnl	I_ZERO;	/* Last token processed was \n	*/

/*
 *	Macro proto pointers.
 */

struct	symtab
{
    struct	symtab	*s_link; /* Next in list for this chain		*/
    char	disable; /* TRUE to disable recognition for now	*/
    char	*s_body; /* Body of definition			*/
    struct	param	*s_params;	/* List of parameters		*/
    char	s_name[ 1 ];	/* Name is appended to structure*/
};

struct	param
{
    struct	param	*p_link; /* Next in list				*/
    char	p_flags; /* Flags:				*/
#define	PF_RQUOTES	0x01	/* Remove "" chars from parameter	*/
#define	PF_PNLINES	0x02	/* Preserve '\n' char in parameter	*/
    char	p_name[ 1 ];	/* Name is appended to struct	*/
};

/*
 *	Predefined symbol entry.
 */

struct	ppdir
{
    char	*pp_name;	/* #function name		*/
    char	pp_ifif;	/* FALSE if ! to do on false #if*/
    void	( *pp_func ) ();	/* Address of function		*/
    int	pp_arg;		/* Argument to function		*/
};

/*
 *	Predefined symbols.
 */

#define	YES		TRUE
#define	NO		FALSE
#define	EMPTY		0	/* Catch-all for non-used field		*/

/*
 *	Alphabetically ordered.
 */

#if	(TARGET == T_QC) OR (TARGET == T_QCX) OR (TARGET == T_TCX)
extern	void	doasm();
#endif	/* (TARGET == T_QC) OR (TARGET == T_QCX) OR (TARGET == T_TCX) */

extern void dodefine();
extern void doelse();
extern void doendif();
extern void doerror();
extern void doif();
extern void doifs();
extern void doinclude();
extern void doline();
extern void dopragma();
extern void doundef();

#ifdef	MAIN			/* If in main() module			*/
struct	ppdir	pptab[] =
    {
        /*	 Directive	Do within	Procedure	Arg to	  */
        /*	   name		FALSE #ifxx	name		function  */
        /* --------------	-----------	----------	--------  */
#if	(TARGET == T_QC) OR (TARGET == T_QCX) OR (TARGET == T_TCX)

        {"asm", NO, doasm, TRUE
        },
#endif	/* (TARGET == T_QC) OR (TARGET == T_QCX) OR (TARGET == T_TCX) */
        {"define", NO, dodefine, FALSE	},
        {"elif", YES, doelse, TRUE	},
        {"else", YES, doelse, FALSE	},
#if	(TARGET == T_QC) OR (TARGET == T_QCX) OR (TARGET == T_TCX)

        {"endasm", NO, doasm, FALSE
        },
#endif	/* (TARGET == T_QC) OR (TARGET == T_QCX) OR (TARGET == T_TCX) */
        {"endif", YES, doendif, EMPTY	},
        {"error", NO, doerror, EMPTY	},
        {"if", YES, doif, EMPTY	},
        {"ifdef", YES, doifs, TRUE	},
        {"ifndef", YES, doifs, FALSE	},
        {"include", NO, doinclude, EMPTY	},
        {"line", NO, doline, EMPTY	},
        {"undef", NO, doundef, EMPTY	},
        {"pragma", YES, dopragma, EMPTY	},
        {NULL}	/* The end */
    };

#if	(TARGET == T_QC) OR (TARGET == T_QCX) OR (TARGET == T_TCX)
extern	void	pragasm();
#endif	/* (TARGET == T_QC) OR (TARGET == T_QCX) OR (TARGET == T_TCX)*/

extern	void	pragendm();
extern	void	pragerror();
extern	void	pragmsg();
extern	void	pragopt();
extern	void	pragvalue();

/*
 *	Pragma keyword table.
 */
struct	ppdir	pragtab[] =
    {
        /*	 Keyword	Do within	Procedure	Arg to	  */
        /*	   name		FALSE #ifxx	name		function  */
        /* ------------------	-----------	----------	--------  */
        {"arg_string", NO, pragopt, EMPTY	},
#if	(TARGET == T_QC) OR (TARGET == T_QCX) OR (TARGET == T_TCX)

        {"asm", NO, pragasm, TRUE
        },
        {"asm_expand", NO, pragopt, EMPTY	},
#endif	/* (TARGET == T_QC) OR (TARGET == T_QCX) OR (TARGET == T_TCX) */
        {"comment_recurse", NO, pragopt, EMPTY	},
        {"define", NO, dodefine, FALSE	},
        {"elif", YES, doelse, TRUE	},
        {"else", YES, doelse, FALSE	},
#if	(TARGET == T_QC) OR (TARGET == T_QCX) OR (TARGET == T_TCX)

        {"endasm", NO, pragasm, FALSE
        },
#endif	/* (TARGET == T_QC) OR (TARGET == T_QCX) OR (TARGET == T_TCX) */
        {"endif", YES, doendif, EMPTY	},
        {"endmacro", NO, pragendm, EMPTY	},
        {"error", NO, pragerror, FALSE	},
        {"if", YES, doif, EMPTY	},
        {"ifdef", YES, doifs, TRUE	},
        {"ifndef", YES, doifs, FALSE	},
        {"macro", NO, dodefine, TRUE	},
        {"macro_rescan", NO, pragopt, EMPTY	},
        {"macro_stack", NO, pragopt, EMPTY	},
        {"message", NO, pragmsg, EMPTY	},
        {"trigraph", NO, pragopt, EMPTY	},
        {"undef", NO, doundef, EMPTY	},
        {"value", NO, pragvalue, EMPTY	},
        {NULL}	/* The end */
    };

#else	/* !MAIN */
extern	struct	ppdir	pptab[];
extern	struct	ppdir	pragtab[];
#endif	/* MAIN */

EXTERN	int	A_astring I_ZERO;	/* TRUE/args in strings	*/
EXTERN	int	A_crecurse I_ZERO;	/* TRUE/comments nest	*/
EXTERN	int	A_eolcomment I_ZERO;	/* TRUE/eol comments OK	*/
EXTERN	int	A_rescan I_ZERO; /* TRUE/direct. from macro's	*/
EXTERN	int	A_stack	I_ZERO;	/* TRUE/macro def's stack	*/
EXTERN	int	A_trigraph I_ZERO;	/* TRUE/trigraphs active*/

EXTERN	struct	symtab *Macros[ NUMBUCKETS ] I_BRZERO; /* Ptr/macro chains*/
EXTERN	int	Nsyms	I_ZERO;	/* Number of symbols in table	*/
EXTERN	int	Maxsyms	I_ZERO;	/* Max number of symbols used	*/

#define	NO_PARAMS 	(struct param *)NULL	/* For sbind of 0 params*/

struct	ifstk
{
    char	i_state; /* The ifstack state:			*/
#define	IFTRUE		0	/* True - include code within #ifxx	*/
#define	IFFALSE		1	/* False - no code within #ifxx		*/
#define	IFNEVER		2	/* Treat as false forever after		*/
    char	i_else;	/* True if encountered an #else		*/
};

EXTERN	struct	ifstk	Ifstack[ IFSTACKSIZE + 1 ] I_BRZERO;
EXTERN	int	Iflevel	I_ZERO;	/* Index into Ifstack		*/
EXTERN	int	Ifstate	I_ZERO;	/* Current state of #if		*/

#if	DEBUG
EXTERN	int	Debug I_ZERO;	/* -z flag			*/
#endif	/* DEBUG */

EXTERN	int	Lineopt; /* True if producing #line directives	*/
#define	LINE_EXP	1	/* If to expand to #line directives	*/
#define	LINE_ABR	2	/* If to abbreviate to # n "file"	*/

EXTERN	int	Outline	I_ZERO;	/* Line # of current output file*/
EXTERN	int	Errors	I_ZERO;	/* Total errors detected	*/
EXTERN	int	Eflag	I_ZERO;	/* True if to ignore errors	*/
#if	DEBUG
EXTERN	int	Stats	I_ZERO;	/* True to print stats at end	*/
#endif	/* DEBUG */
EXTERN	char	Date[ 12 ] I_BRZERO;	/* Date str for __DATE__*/
EXTERN	char	_Time[ 9 ] I_BRZERO;	/* Time str for __TIME__*/
EXTERN	unsigned int	Unique	I_ZERO;	/* Unique # for __NOW__/__NEXT__*/
EXTERN	int	Verbose	I_ZERO;	/* True to print verbose mess	*/
#if	(TARGET == T_QC) OR (TARGET == T_QCX) OR (TARGET == T_TCX)
EXTERN	int	Do_asm	I_ZERO;	/* True if in asm/endasm body	*/
EXTERN	int	Macexpand	I_ZERO;	/* True/macro expand on	*/
EXTERN	int	Asmexpand	I_ZERO;	/* Set Macexpand in asm	*/
#endif	/* (TARGET == T_QC) OR (TARGET == T_QCX) OR (TARGET == T_TCX) */

#define	NIPATHS		10	/* Up to 10 -i pathnames		*/
EXTERN	char	*Ipath[ NIPATHS + 1 ] I_BRZERO;	/* -i path list	*/
EXTERN	int	Ipcnt I_ZERO;	/* Count of -i parameters	*/

#if	HOST == H_CPM
EXTERN	int	Orig_user I_ZERO;	/* Original user	*/
EXTERN	int	Orig_disk I_ZERO;	/* Original disk	*/
#endif	/* HOST == H_CPM */

/*
 *	End of pp.h
 */
