
#include <string.h>
#include <stdio.h>

char *optarg;
int opterr;
int optind = 1;


#define BADCH  (int)'?'
#define EMSG   ""
#define tell(s)   fputs(*nargv,stderr);fputs(s,stderr); \
      fputc(optopt,stderr);fputc('\n',stderr);return(BADCH);


int getopt(int nargc,const char **nargv,const char *ostr)
{
	static char *place = EMSG; /* option letter processing */
	register char  *oli;    /* option letter list index */
	int optopt;

	if(!*place)
	{        /* update scanning pointer */
		if(optind >= nargc || *(place = (char*)nargv[optind]) != '-' || !*++place)
			return(EOF);
		if (*place == '-')
		{ /* found "--" */
			++optind;
			return(EOF);
		}
	}           /* option letter okay? */
	if ((optopt = (int)*place++) == (int)':' || !(oli = strchr(ostr,(char)optopt)))
	{
		if(!*place)
			++optind;
		tell(": illegal option -- ");
	}
	if (*++oli != ':')
	{    /* don't need argument */
		optarg = NULL;
		if (!*place)
			++optind;
	}
	else
	{            /* need an argument */
		if (*place)
			optarg = place;  /* no white space */
		else if (nargc <= ++optind)
		{   /* no arg */
			place = EMSG;
			tell(": option requires an argument -- ");
		}
		else
			optarg = (char*)nargv[optind];  /* white space */
		place = EMSG;
		++optind;
	}
	return(optopt);        /* dump back option letter */
}
