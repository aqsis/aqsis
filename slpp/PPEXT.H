/************************************************************************/
/*									*/
/*	File:	ppext.h							*/
/*									*/
/*	External function prototype definitions for PP.			*/
/*									*/
/************************************************************************/

/*
 *	pp1.c
 */
extern	void			main();
extern	char			*getnext();
extern	void			init();
extern	void			usage();
/*lint -function(exit,usage) */

/*
 *	pp2.c
 */
extern	char			*docall();
extern	char			*_docall();
extern	void			dodefine();
extern	void			doerror();
extern	void			doundef();
extern	char			*esc_str();
extern	void			fbind();
extern	char			*flookup();
extern	struct	param		*getparams();
extern	unsigned	int	hash();
extern	struct	symtab		*lookup();
extern	struct	param		*makeparam();
extern	struct	ppdir		*predef();
extern	void			sbind();
extern	char			*strize();
extern	void			unfbind();
extern	void			unparam();
extern	void			unsbind();

/*
 *	pp3.c
 */
extern	void			cur_user();
extern	void			do_line();
extern	void			doinclude();
extern	void			doline();
extern	int			gchbuf();
extern	int			gchfile();
extern	int			gchpb();
extern	int			getchn();
extern	int			inc_open();
extern	void			init_path();
extern	int			popfile();
extern	char			*readline();
extern	void			scaneol();
extern	void			set_user();
extern	int			trigraph();

/*
 *	pp4.c
 */
extern	char			*addstr();
extern	int			getnstoken();
extern	int			gettoken();
#if	!PP
extern	int			istype();
#endif	/* !PP */
extern	void			memmov();
extern	void			pbcstr();
extern	void			pbstr();
extern	void			pushback();
extern	void			puttoken();
extern	int			type();

/*
 *	pp5.c
 */
extern	void			doelse();
extern	void			doendif();
extern	void			doif();
extern	void			doifs();

/*
 *	pp6.c
 */
#if	(TARGET == T_QC) OR (TARGET == T_QCX) OR (TARGET == T_TCX)
extern	void			doasm();
#endif	/* (TARGET == T_QC) OR (TARGET == T_QCX) OR (TARGET == T_TCX) */

extern	void			dopragma();

#if	(TARGET == T_QC) OR (TARGET == T_QCX) OR (TARGET == T_TCX)
extern	void			pragasm();
#endif	/* (TARGET == T_QC) OR (TARGET == T_QCX) OR (TARGET == T_TCX) */

extern	void			pragendm();
extern	void			pragerror();
extern	void			pragmsg();
extern	void			pragopt();
extern	void			pragvalue();

/*
 *	pp7.c
 */
extern	void			end_of_file();
/*lint -function(exit,end_of_file) */
extern	void			fatal();
/*lint -function(exit,fatal) */
extern	void			illegal_symbol();
extern	void			non_fatal();
extern	void			out_of_memory();
extern	void			prmsg();
extern	void			warning();

/*
 *	pp8.c
 */
extern	EVALINT			eval();
extern	EVALINT			evaltern();
extern	EVALINT			evallor();
extern	EVALINT			evalland();
extern	EVALINT			evalbor();
extern	EVALINT			evalbxor();
extern	EVALINT			evalband();
extern	EVALINT			evaleq();
extern	EVALINT			evalrel();
extern	EVALINT			evalsh();
extern	EVALINT			evalsum();
extern	EVALINT			evalmdr();
extern	EVALINT			evalfuns();
extern	EVALINT			evalucom();
extern	EVALINT			evalunot();
extern	EVALINT			evalumin();
extern	EVALINT			evalval();
extern	EVALINT			hexbin();
extern	int			ishex();
extern	int			isoct();
extern	int			item();
extern	int			look();
extern	int			match();
extern	char			*readexpline();
extern	int			test();
