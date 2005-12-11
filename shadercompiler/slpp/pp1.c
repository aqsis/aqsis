/************************************************************************/
/*									*/
/*	File:	pp1.c							*/
/*									*/
/*	Main program module.						*/
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
/*	Synopsis:							*/
/*		PP [inputfile] -[cdeilostuvz?]				*/
/*			-c 0|1|2|3|4|5|6 Enable desired option:		*/
/*				0 -> Replace arguments in strings.	*/
/*				1 -> Expand macros inside #pragma asm.	*/
/*				2 -> Allow recursive comments.		*/
/*				3 -> Rescan macro expansions for	*/
/*					directives.			*/
/*				4 -> Allow macro definitions and undefs	*/
/*					to stack.			*/
/*				5 -> Perform Trigraph translation.	*/
/*				6 -> Allow C++ style "//" eol comments	*/
/*			-d: define symbols (-d symbol[=value].)		*/
/*			-e: Don't abort on errors.			*/
/*			-i <list>: Specify path for #include searches.	*/
/*			-l <t>: Control line # emission to the Output	*/
/*				file.  If <t> == 'n' don't emit any	*/
/*				explicit line # info.  If <t> == 'a',	*/
/*				then use the abbreviated form:		*/
/*					"#<n> |filename|"		*/
/*				If <t> == 'l' then use the "long" form:	*/
/*					"#line <n> |filename|"		*/
/*				The "long" form is the default.		*/
/*			-o: Specify output file name (def: <infile>.pp)	*/
/*			-s: Product statistics info at end (DEBUG only.)*/
/*			-t: Specify add/remove token characters.	*/
/*			-u: Undefine an initial symbol (e.g. CPM)	*/
/*			-v: Invert VERBOSE toggle.			*/
/*			-z: Output DEBUG messages (DEBUG only.)		*/
/*			-?: Print usage information.			*/
/*									*/
/*	If [inputfile] is absent, standard input is assumed.		*/
/*									*/
/*	Description:							*/
/*		PP is a general purpose preprocessor that handles	*/
/*		ANSI C style preprocessor directives.			*/
/*									*/
/*	Example:							*/
/*		PP test.c						*/
/*									*/
/*	History:							*/
/*		Inspired by the PP.C program by:			*/
/*									*/
/*			Robert T. Pasky					*/
/*			36 Wiswall Rd					*/
/*			Newton Centre, MA 02159				*/
/*			(617) 964-3641					*/
/*									*/
/*		which, in turn, was based upon the Ratfor		*/
/*		version found in K&P's Software Tools.			*/
/*									*/
/*		30-Mar-85 GO	First entered.				*/
/*									*/
/*	Conventions:							*/
/*		The convention used within this program is to		*/
/*		capitalize global variable names.  Function		*/
/*		names, per the C convention, are not capitalized.	*/
/*									*/
/*	Future features:						*/
/*		Generalize on a means for generating the output		*/
/*		name.  Currently ".pp" is appended to the input		*/
/*		file name, after the input file extension (if any)	*/
/*		is removed.  Dave Regan suggests prepending a '$'	*/
/*		to the input file name and using that as the output	*/
/*		name, retaining the input file's extension (if any).	*/
/*		(e.g. ABC.XYZ becomes $ABC.XYZ)  Must think on this.	*/
/*									*/
/*		Some performance enhancement may be gained by using	*/
/*		a better symbol table allocation or simply a better	*/
/*		hashing function.  I will have to check some		*/
/*		reference materials...					*/
/*									*/
/*	Functions contained in this module:				*/
/*									*/
/*		main		Main module.				*/
/*		getnext		Get next parameter from argv/argc.	*/
/*		init		Initialize the preprocessor.		*/
/*		usage		Print usage information.		*/
/*									*/
/*	* Unix is a trademark/footnote of Bell Laboratories.		*/
/*	* CP/M is a trademark of Digital Resarch, Inc.			*/
/*	* Q/C is a trademark of Quality Computer Systems.		*/
/*	* VMS is a trademark of Digital Equipment Corporation.		*/
/*									*/
/************************************************************************/

#define		MAIN			/* This is the main() module */

#include	"pp.h"
#include	"ppext.h"

#ifdef	__TURBOC__
extern	unsigned	_stklen = 32767;
#endif	/* __TURBOC__ */

/************************************************************************/
/*									*/
/*	main								*/
/*									*/
/*	The main program.						*/
/*									*/
/************************************************************************/

#ifdef	MSC_OPT
#pragma	optimize("e",off)		/* Disable global reg optimizing */
#pragma	optimize("g",off)		/* Disable global common subs */
#pragma	optimize("l",off)		/* Disable loop optimizations */
#endif	/* MSC_OPT */

void
#ifdef	SLPP_LIBRARY
PreProcess(argc,argv)
#else
main(argc,argv)
#endif
int			argc;
char			**argv;
{
	static	char		*one_string = "1";

	register int		t;	/* General holder for token	*/
	register struct	symtab	*p;	/* Ptr into symbol table	*/
	register struct	ppdir	*sp;	/* Ptr to predefined symbol	*/
	int			ifile;
	int			ofile;
	char			*s;
	char			*s2;
	int			i;
#if	DEBUG

	int			n;
#endif	/* DEBUG */

	int			skip;	/* Boolean for option loop	*/

	init();				/* Initialize preprocessor	*/

	ifile	=			/* No input file specified	*/
	    ofile	= FALSE;		/* No output file specified	*/

	while(--argc != 0)
	{
		s = *++argv;
		if(s[0] == '-')
		{
			skip = FALSE;
			while((! skip) && (*++s != '\0'))
			{
				switch((int) *s)
				{
						/* -[c 0|1|2|3|4|5|6] */
						case 'C':
						case 'c':
						s2 = getnext(s,&argc,&argv,NO);
						switch((int) *s2)
						{
								case '0':
								s2 = "arg_string";
								break;

								case '1':
								s2 = "asm_expand";
								break;

								case '2':
								s2 = "comment_recurse";
								break;

								case '3':
								s2 = "macro_rescan";
								break;

								case '4':
								s2 = "macro_stack";
								break;

								case '5':
								s2 = "trigraph";
								break;

								case '6':
								s2 = "eol_comment";
								break;

								default:
								usage(TRUE);
						}
						pragopt(EMPTY,FALSE,s2);
						skip = TRUE;
						break;

						/* -[d symbol [= value]] */
						case 'D':
						case 'd':
						s = getnext(s,&argc,&argv,NO);
						s2 = strchr(s,'=');	/* Location of val */
						if(s2)
							*s2++ = '\0';	/* Terminate string */
						else
							s2 = one_string;	/* Default */

						if(lookup(s,NULL) != NULL)
							warning("Symbol already defined: ",s);
						else
							sbind(s,s2,NO_PARAMS);

						skip = TRUE;	/* Skip to next param */
						break;

						/* -[e] don't abort on errors */
						case 'E':
						case 'e':
						Eflag = TRUE;
						break;

						/* -iI <#include search path> */
						case 'I':
						case 'i':
						if(Ipcnt > NIPATHS)
							fatal("Too many pathnames","");
						Ipath[Ipcnt++] = getnext(s,&argc,&argv,NO);
						skip = TRUE;	/* Skip to next param */
						break;

						/* -[l a|l|n] Spec #line output mode */
						case 'L':
						case 'l':
						s2 = getnext(s,&argc,&argv,NO);
						switch((int) *s2)
						{
								case 'A':
								case 'a':
								Lineopt = LINE_ABR;
								break;

								case 'L':
								case 'l':
								Lineopt = LINE_EXP;
								break;

								case 'N':
								case 'n':
								Lineopt = FALSE;
								break;

								default:
								usage(TRUE);
						}
						skip = TRUE;	/* Skip to next param */
						break;

						/* -[o file] spec output file name */
						case 'O':
						case 'o':
						s2 = getnext(s,&argc,&argv,NO);
						strcpy(Outfile,s2);	/* Copy filename */
						ofile = TRUE;
						skip = TRUE;	/* Skip to next param */
						break;

#if	DEBUG
						/* -[s] give statistics at end */
						case 'S':
						case 's':
						Stats = Verbose = TRUE;	/* Implies Verbose */
						break;
#endif	/* DEBUG */
						/* -[t Astr|Rstr] Add or delete chars from LETTER class */
						case 'T':
						case 't':
						s2 = getnext(s,&argc,&argv,NO);
						switch((int) *s2++)
						{
								case 'a':
								case 'A':
								i = TRUE;
								break;

								case 'r':
								case 'R':
								i = FALSE;
								break;

								default:
								usage(TRUE);
						}

						for(; *s2 != '\0'; s2++)
						{
							if(i)
								typetab[*s2 + 1] |= C_L;
							else
								typetab[*s2 + 1] &= ~C_L;
						}

						skip = TRUE;	/* Skip to next param */
						break;

						/* -[u symbol] */
						case 'U':
						case 'u':
						s = getnext(s,&argc,&argv,NO);

						if(lookup(s,NULL) == NULL)
							warning("Symbol not defined: ",s);
						else
							unsbind(s);
						skip = TRUE;	/* Skip to next param */
						break;

						/* -[v] verbose mode toggle */
						case 'V':
						case 'v':
						Verbose = !Verbose;
						break;

#if	DEBUG
						/* -[z] enable debug */
						case 'Z':
						case 'z':
						Debug = TRUE;
						printf("Debug is on\n");
						break;
#endif	/* DEBUG */

						case '?':
						usage(FALSE);	/* Give usage info and quit */

						default:
						fprintf(STDERR,"FATAL: Bad option: %s\n",s);
						usage(TRUE);
				}
			}
		}
		else if(! ifile)
		{
			/* Try to get input file */
#if	HOST == H_CPM
			if(! inc_open(s,-1,0))	/* Open file here */
#else	/* HOST != H_CPM */

			if(! inc_open(s))	/* Open input file */
#endif	/* HOST == H_CPM */

			{
				fatal("Unable to open input file: ",s);
			}
			ifile = TRUE;	/* Got an input file */
		}
		else
		{
			/* Too many file names given */
			usage(TRUE);
		}
	}

	Nextch = A_trigraph ? trigraph : gchbuf;	/* Next char source */

	if(! ifile)
	{
		/* Must have at least an input file name */
		usage(TRUE);
	}

	if(! ofile)
	{
		/* No output name given; use input name and modify it */
		strcpy (Outfile,Filestack[0]->f_name);
		/* terminate the file name before any extension */
		if((s = strrchr(Outfile,'.')) != NULL)
			*s = '\0';
		strcat(Outfile,".pp");
	}
	else
	{
		if(strcmp(Outfile,Filestack[0]->f_name) == EQUAL)
			fatal("Input and output filenames are the same: ",Outfile);
		else if((Output = fopen(Outfile,"w")) == NULL)
			fatal("Unable to create output file: ",Outfile);
	}

#if	HOST == H_CPM
	/* Create a bigger than average buffer */
	if((s = malloc(OUTBUFSIZE)) == NULL)
		out_of_memory();
	setbuf(Output,s);
	setbsize(Output,OUTBUFSIZE);
#endif	/* HOST == H_CPM */

	if(Verbose)
	{
		printf("%s%s\n\n","PP Preprocessor, ",VERSION);
		printf("Output will be on <%s>\n",Outfile);
		printf("*** Read    %s\n",Filestack[Filelevel]->f_name);
	}

	Do_name = TRUE;			/* Force name output on #line */

	init_path();			/* Initialize search path */
	Ipath[Ipcnt] = NULL;		/* Terminate last include path */

	for(Lastnl = TRUE, t = gettoken(GT_STR); t != EOF;
	        t = gettoken(GT_STR))
	{
		if((Ifstate != IFTRUE) && (t != '\n') && istype(t,C_W))
		{}
		else if(Lastnl && (t == DIRECTIVE_CHAR))
		{
			t = getnstoken(GT_STR);
			if(t == LETTER)
			{
				if((sp = predef(Token,pptab)) != NULL)
				{
					/*
					 *	If unconditionally do it or if emitting code...
					 */
					if(sp->pp_ifif || (Ifstate == IFTRUE))
					{
						/* Do #func */					(void) (*(sp->pp_func))
						(sp->pp_arg);
					}
				}
				else if(Ifstate == IFTRUE)
					non_fatal("Illegal directive","");

				scaneol();	/* Suck till EOL ('\n' next) */
			}
			else if(t != '\n')
			{
				non_fatal("Bad directive","");
				scaneol();
			}
			else
				pushback('\n');	/* Leave for fetch to get */
		}
		else if((t != EOF) && (Ifstate == IFTRUE))
		{
#if	(TARGET == T_QC) OR (TARGET == T_QCX) OR (TARGET == T_TCX)
			if(t == LETTER && Macexpand)
#else	/* !((TARGET == T_QC) OR (TARGET == T_QCX) OR (TARGET == T_TCX)) */

			if(t == LETTER)
#endif	/* (TARGET == T_QC) OR (TARGET == T_QCX) OR (TARGET == T_TCX) */

			{
				if((p = lookup(Token,NULL)) != NULL)
				{
					/* Call macro */			(void) docall(p,NULL,NULL);
				}
				else
					/* Just output token if nothing */
					puttoken(Token);
			}
			else
			{
				puttoken(Token);
				if(t == '\n')
					Lastnl = TRUE;	/* Turn on if '\n' */
				else if(! istype(t,C_W))
					Lastnl = FALSE;	/* Turn off if !ws */
			}
		}
		else
		{
			while((t != '\n') && (t != EOF))
			{
				/* Absorb to EOL if False #ifxx */
				t = gettoken(GT_STR);
			}
			Lastnl = TRUE;
		}
	}

	if(Iflevel != 0)
	{
		/* Unterminated #if */
		non_fatal("Unterminated conditional","");
	}

	if(Verbose)
	{
		printf("\nActive symbols at end:\t%d\tMaximum symbols used:\t%d\n",
		       Nsyms,Maxsyms);
#if	HOST == H_CPM

		printf("Free memory: %5u\n",maxsbrk());
#endif	/* HOST == H_CPM */

	}

	if(Verbose || Errors)
	{
		if(Errors)
			printf("\n%d errors detected\n",Errors);
		else
			printf("\nNo errors detected\n");
	}

#if	DEBUG
	if(Stats)
	{
		printf("\nSymbol table bucket statistics:");
		for(i = 0; i < NUMBUCKETS; i++)
		{
			if((i % 8) == 0)
				printf("\n");	/* New line */
			for(n = 0, p = Macros[i]; p != 0; p = p->s_link, n++)
				;	/* Count items in list */
			printf("  %3d:%-3d",i,n);
		}
		printf("\n\n");
	}
#endif	/* DEBUG */

	if((Output != stdout) && (fclose(Output) == EOF))
		fatal("Unable to close output file: ",Outfile);
#ifdef	SLPP_LIBRARY

	return;
#else

	exit(Eflag ? 0 : Errors);
#endif	/* SLPP_LIBRARY */

}

#ifdef	MSC_OPT
#pragma	optimize("e",on)		/* Enable global reg optimizing */
#pragma	optimize("g",on)		/* Enable global common subs */
#pragma	optimize("l",on)		/* Enable loop optimizing */
#endif	/* MSC_OPT */

/************************************************************************/
/*									*/
/*	getnext								*/
/*									*/
/*	Get next parameter from parameter line and return its address.	*/
/*	If the character following the current parameter (cp) is non-	*/
/*	null, then return that parameter, otherwise look ahead in the	*/
/*	argv list to fetch the next one.  Give a usage() error if the	*/
/*	parameter list is empty or if the fetched parameter's first	*/
/*	character is a '-' and the <swvalid> parameter is NO.		*/
/*									*/
/************************************************************************/

char	*
getnext(cp,argc,argv,swvalid)
char			*cp;
int			*argc;
char			***argv;
int			swvalid;	/* True if -x token valid */
{
	if(*++cp == '\0')
	{
		if(*argc != 0)
		{
			/* Parameters remain -- use next one */
			--*argc;	/* Count it down */
			cp = *++*argv;	/* Return its address */
		}
		else
			usage(TRUE);	/* Otherwise give usage error */
	}

	if(!swvalid && (*cp == '-'))
		usage(TRUE);		/* Complain if switch starts w/'-' */

	return (cp);
}

/************************************************************************/
/*									*/
/*	init								*/
/*									*/
/*	Initialize the preprocessor.					*/
/*									*/
/************************************************************************/

void
init()
{
	static	char		*one_string = "1";

	char			*fromptr;
	int			i;
#if	(HOST != H_CPM) AND (HOST != H_MPW)

	time_t			long_time;	/* time_t def'd in "time.h" */
#endif	/* (HOST != H_CPM) AND (HOST != H_MPW) */

	char			str[TOKENSIZE + 1];
	char			*toptr;

#if	DEBUG

	Debug = FALSE;
#endif	/* DEBUG */

	Verbose = FALSE;		/* Set verbose state */

	Eflag = FALSE;			/* Say to abort on errors */
	Lineopt = LINE_EXP;		/* Default to "long" #line form */

#if	(TARGET == T_QC) OR (TARGET == T_QCX) OR (TARGET == T_TCX)

	Do_asm = FALSE;			/* Not inside #pragma asm/endasm */
	Macexpand = TRUE;		/* Macro expansion enabled */
	Asmexpand = FALSE;		/* Disabled inside asm/endasm */
#endif	/* (TARGET == T_QC) OR (TARGET == T_QCX) OR (TARGET == T_TCX) */

	Outline = 1;			/* Line number of next output line */

	Filelevel = -1;			/* Current file level */

	Pbbuf = Pbbufp =
	            (struct pbbuf *) malloc(sizeof(struct pbbuf) * PUSHBACKSIZE);

	if(Pbbufp == NULL)
		out_of_memory();

	Pbbufp->pb_type = PB_TOS;	/* Top of stack marker */

	A_astring	=		/* Replace args within strings	*/
	    A_crecurse	=		/* No recursive comments	*/
	        A_eolcomment	=		/* No eol comments		*/
	            A_rescan	=		/* No macro gen'd directives	*/
	                A_stack		=		/* No stacking of macro def's	*/
	                    A_trigraph	= FALSE;	/* No Trigraph translation	*/

	Nsyms		=		/* Number of symbols generated	*/
	    Errors		=		/* Zero the error counter	*/
	        Iflevel		=		/* #if stack pointer		*/
	            Ipcnt		=		/* Number of -i paths		*/
	                Unique		= 0;		/* Zero unique # counter	*/

	Ifstack[0].i_state = Ifstate =
	                         IFTRUE;			/* Currently TRUE #ifxxx assumed */

#if	HOST == H_CPM

	Orig_user = bdos1(BDOS_USER,0xFF);	/* Save user and disk */
	Orig_disk = bdos1(BDOS_GETDISK,0);
#endif	/* HOST == H_CPM */

	/*
	 *	Initialize the Time and Date variables to hold the ANSI strings that
	 *	each will print out in response to __TIME__ and __DATE__ respectively.
	 */
#if	(HOST == H_CPM) OR (HOST == H_MPW)

	strcpy(_Time,"HH:MM:SS");	/* Fake a time */
	strcpy(Date,"Mmm DD YYYY");
#else	/* !((HOST == H_CPM) OR (HOST == H_MPW)) */

	(void) time(&long_time);	/* Seconds since whenever... */
	strncpy(str,asctime(localtime(&long_time)),26);	/* Get time/date */

	strncpy(_Time,&str[11],8);	/* Pull time portion out of string */
	_Time[8] = '\0';

	strncpy(Date,&str[4],7);	/* Pull month and day out of string */
	strncpy(&Date[7],&str[20],4);	/* Pull year out of string */
	Date[11] = '\0';
#endif	/* (HOST == H_CPM) OR (HOST == H_MPW) */

	/************************************/
	/* Define the automatic definitions */
	/************************************/

#if	TARGET == T_UNIX

	sbind("unix",one_string,NO_PARAMS);	/* #define unix 1 */
#endif	/* TARGET == T_UNIX */

#if	TARGET == T_BSD

	sbind("unix",one_string,NO_PARAMS);	/* #define unix 1 */
	sbind("BSD",one_string,NO_PARAMS);	/* #define BSD 1 */
#endif	/* TARGET == T_BSD */

#if	TARGET == T_VMS

	sbind("VMS",one_string,NO_PARAMS);	/* #define VMS 1 */
#endif	/* TARGET == T_VMS */

#if	(TARGET == T_QC) OR (TARGET == T_QCX)

	sbind("CPM",one_string,NO_PARAMS);	/* #define CPM 1 */
	sbind("QC",one_string,NO_PARAMS);	/* #define QC 1 */
	/* Generate asm() macro */
	sbind("asm",";\n#pragma asm\n_PARAM_\n#pragma endasm\n",
	      makeparam("_PARAM_",PF_RQUOTES));
#endif	/* (TARGET == T_QC) OR (TARGET == T_QCX) */

#if	TARGET == T_TCX

	sbind("TC",one_string,NO_PARAMS);	/* #define TC 1 */
	/* Generate asm() macro */
	sbind("asm",";\n#pragma asm\n_PARAM_\n#pragma endasm\n",
	      makeparam("_PARAM_",PF_RQUOTES));
#endif	/* TARGET == T_TCX */

	/*	Now bind the automatic line/file generators, etc.	*/

	str[1] = '\0';
	str[0] = (char) LINE_TOKEN;
	sbind("__LINE__",str,NO_PARAMS);
	str[0] = (char) FILE_TOKEN;
	sbind("__FILE__",str,NO_PARAMS);
	str[0] = (char) TIME_TOKEN;
	sbind("__TIME__",str,NO_PARAMS);
	str[0] = (char) DATE_TOKEN;
	sbind("__DATE__",str,NO_PARAMS);
	str[0] = (char) NOW_TOKEN;
	sbind("__NOW__",str,NO_PARAMS);
	str[0] = (char) NEXT_TOKEN;
	sbind("__NEXT__",str,NO_PARAMS);
	str[0] = (char) PREV_TOKEN;
	sbind("__PREV__",str,NO_PARAMS);

	/*	Bind macro symbols for the configuration setting variables	*/

	for(i = 0; pragtab[i].pp_name != NULL; i++)
	{
		if(pragtab[i].pp_func == pragopt)
		{
			strcpy(str,"__");
			for(toptr = &str[2], fromptr = pragtab[i].pp_name;
			        *fromptr; fromptr++)
			{
				*toptr++ = (islower(*fromptr) ?
				            toupper(*fromptr) : *fromptr);
			}
			*toptr = '\0';
			strcat(str,"__");
			sbind(str,"0",NO_PARAMS);
		}
	}
}

/************************************************************************/
/*									*/
/*	usage								*/
/*									*/
/*	Print usage information and exit with argument passed.		*/
/*									*/
/************************************************************************/

void
usage(v)
int			v;
{
	printf(
#if	DEBUG
	    "Usage: pp <input> -[cdeilostuvz?]\n%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
#else	/* !DEBUG */
	    "Usage: pp <input> -[cdeilotuv?]\n%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
#endif	/* DEBUG */
	    "   -c 0|1|2|3|4|5|6 Enable the desired configuration option:\n",
	    "              0 -> Replace macro arguments in strings.\n",
	    "              1 -> Expand macros inside #pragma asm.\n",
	    "              2 -> Allow recursive comments.\n",
	    "              3 -> Rescan macro expansions for directives.\n",
	    "              4 -> Allow macro defs and undefs to stack/unstack.\n",
	    "              5 -> Perform Trigraph input character translation.\n",
	    "              6 -> Permit C++ style \"//\" eol comments.\n",
	    "   -d s[=v] Define symbol <s> to have value <v> (default 1).\n",
	    "   -e       Don't abort on error.\n",
	    "   -i       Set path for #include files.\n",
	    "   -l a|l|n Specify #line output mode (abbrev/long/none).\n",
	    "   -o file  Specify output file name.\n",
#if	DEBUG
	    "   -s       Generate statistics summary at end.\n",
#endif	/* DEBUG */
	    "   -t str   Add/remove LETTER chars (Accc or Rccc).\n",
	    "   -u s     Undefine an initial symbol.\n",
	    "   -v       Verbose mode toggle.\n",
#if	DEBUG
	    "   -z       Output debug messages.\n",
#endif	/* DEBUG */
	    "   -?       Output this message.\n",
	    " Output file, if not specified, is <input>.pp\n");

	exit(v);
}
