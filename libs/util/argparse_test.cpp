#include <iostream>
#include <aqsis/util/argparse.h>

static bool checkArrays(const ArgParse::apintvec& foo,
                        const ArgParse::apfloatvec& bar,
                        const ArgParse::apstringvec& english,
                        const ArgParse::apstringvec& german)
{
	if (foo.size() != 4 || bar.size() != 4 ||
	        english.size() != 4 || german.size() != 4)
	{
		return false;
	}

	for (int i = 0; i < 4; i++)
	{
		ArgParse::apstring en, de;
		switch (i+1)
		{
				case 1:
				en = "one";
				de = "eins";
				break;
				case 2:
				en = "two";
				de = "zwei";
				break;
				case 3:
				en = "three";
				de = "drei";
				break;
				case 4:
				en = "four";
				de = "vier";
				break;
		}
		if (foo[i] != ArgParse::apint(i+1) ||
		        bar[i] != ArgParse::apfloat(i+1) ||
		        english[i] != en || german[i] != de)
		{
			return false;
		}
	}

	return true;
}

int main(int argc, const char** argv)
{
	ArgParse::apflag showhelp = false;
	ArgParse::apint myint = 0;

	ArgParse ap;
	ap.allowOneCharOptionsToBeCombined();
	ap.usageHeader(ArgParse::apstring("Usage: ") + argv[0] + " [options]");
	ap.argFlag("help", "\aprint this help, then exit", &showhelp);
	ap.argInt("integer", "=value\aa random integer you can give for no reason",
	          &myint);
	ap.alias("help", "h");
	ap.alias("integer", "i");
	if (!ap.parse(argc-1, argv+1))
	{
		Aqsis::log() << ap.errmsg() << std::endl << ap.usagemsg();
		return 1;
	}

	if (ap.leftovers().size() > 0)
	{
		Aqsis::log() << "Extra crud \"" << ap.leftovers()[0] << "\" on command line";
		Aqsis::log() << std::endl << ap.usagemsg();
		return 1;
	}

	if (myint != 0)
	{
		cout << "Your favorite integer is " << myint << endl;
	}

	if (showhelp)
	{
		cout << ap.usagemsg();
		return 0;
	}

	int bad = 0;

	cout << "Testing flags and integers... ";
	myint = 3;
	showhelp = false;
	int ok = 0;
	const char* av[100];

	if (ap.parse(0, av) && !showhelp && myint == 3 && ap.leftovers().size() == 0)
		ok++;

	av[0] = "-h";
	if (ap.parse(1, av) && showhelp && myint == 3 && ap.leftovers().size() == 0)
		ok++;

	showhelp = false;
	av[0] = "--help";
	if (ap.parse(1, av) && showhelp && myint == 3 && ap.leftovers().size() == 0)
		ok++;

	showhelp = false;
	av[0] = "--integer";
	av[1] = "5";
	if (ap.parse(2, av) && !showhelp && myint == 5 && ap.leftovers().size() == 0)
		ok++;

	myint = 3;
	av[0] = "-i";
	if (ap.parse(2, av) && !showhelp && myint == 5 && ap.leftovers().size() == 0)
		ok++;

	myint = 3;
	av[2] = "--help";
	if (ap.parse(3, av) && showhelp && myint == 5 && ap.leftovers().size() == 0)
		ok++;

	av[0] = "-h";
	av[1] = "-i";
	av[2] = "5";
	myint = 3;
	showhelp = false;
	if (ap.parse(3, av) && showhelp && myint == 5 && ap.leftovers().size() == 0)
		ok++;

	av[0] = "-hi";
	av[1] = "5";
	myint = 3;
	showhelp = false;
	if (ap.parse(2, av) && showhelp && myint == 5 && ap.leftovers().size() == 0)
		ok++;

	av[0] = "-hi=5";
	myint = 3;
	showhelp = false;
	if (ap.parse(1, av) && showhelp && myint == 5 && ap.leftovers().size() == 0)
		ok++;

	av[0] = "-help";
	if (!ap.parse(1, av) &&
	        ap.errmsg() == "-help: 'e' is an unrecognized option")
		ok++;

	av[0] = "--integer=5";
	myint = 3;
	showhelp = false;
	if (ap.parse(1, av) && !showhelp && myint == 5 && ap.leftovers().size() == 0)
		ok++;

	av[0] = "--integer:5";
	myint = 3;
	if (ap.parse(1, av) && !showhelp && myint == 5 && ap.leftovers().size() == 0)
		ok++;

	av[0] = "-ih";
	av[1] = "5";
	if (!ap.parse(2, av) &&
	        ap.errmsg() == "-ih: 'i' requires an argument")
		ok++;

	av[0] = "--help=5";
	if (!ap.parse(1, av) &&
	        ap.errmsg() == "--help=5: doesn't take an argument")
		ok++;

	av[0] = "--nohelp";
	showhelp = true;
	myint = 3;
	if (ap.parse(1, av) && !showhelp && myint == 3 && ap.leftovers().size() == 0)
		ok++;

	av[1] = "--help";
	if (!ap.parse(2, av) &&
	        ap.errmsg() == "--help: negated flag used with non-negated flag")
		ok++;

	if (ok == 16)
	{
		cout << "PASS" << endl;
	}
	else
	{
		cout << "FAIL" << endl;
		bad++;
	}

	ok = 0;
	cout << "Testing strings and floats... ";

	ArgParse::apfloat tex;
	ArgParse::apfloat metafont;
	ArgParse::apstring kpathsea;
	ArgParse t1;
	t1.argFloat("tex", "\aversion of TeX", &tex);
	t1.argFloat("metafont", "\aversion of Metafont", &metafont);
	t1.argString("kpathsea", "\aversion of Kpathsea", &kpathsea);

	av[0] = "-tex";
	av[1] = "3.14159";
	av[2] = "--metafont=2.7182";
	av[3] = "--kpathsea";
	av[4] = "3.3.1";
	if (t1.parse(5, av) && t1.leftovers().size() == 0 &&
	        tex == 3.14159 && metafont == 2.7182 && kpathsea == "3.3.1")
		ok++;

	av[1] = "blech";
	if (!t1.parse(5, av) &&
	        t1.errmsg() ==  "-tex: \"blech\" is not a valid floating-point number")
		ok++;

	if (ok == 2)
	{
		cout << "PASS" << endl;
	}
	else
	{
		cout << "FAIL" << endl;
		bad++;
	}

	ok = 0;
	cout << "Testing usage message... ";

	ArgParse::apflag junkf;
	ArgParse t2;
	t2.usageHeader("GNU `tar' saves blah, blah, blah\n\n"
	               "Main operation mode:", 26);
	t2.argFlag("t", "\alist the contents of an archive", &junkf);
	t2.argFlag("x", "\aextract files from an archive", &junkf);
	t2.argFlag("delete",
	           "\adelete from the archive (not on mag tapes!)", &junkf);
	t2.alias("t", "list");
	t2.alias("x", "extract");
	t2.alias("x", "get");
	t2.usageHeader("\nOperation modifiers:", 29);
	t2.argFlag("G", "\ahandle old GNU-format incremental backup", &junkf);
	t2.argFlag("g", "=FILE\ahandle new GNU-format incremental backup", &junkf);
	t2.alias("G", "incremental");
	t2.alias("g", "listed-incremental");
	t2.usageHeader("\nArchive format selection:", 37);
	t2.argFlag("V", "=NAME\acreate archive with volume name NAME\n      "
	           "        PATTERN\aat list/extract time, a globbing PATTERN",
	           &junkf);
	t2.alias("V", "label");
	t2.allowOneCharOptionsToBeCombined();
	if (t2.usagemsg() ==
	        "GNU `tar' saves blah, blah, blah\n"
	        "\n"
	        "Main operation mode:\n"
	        "  -t, --list              list the contents of an archive\n"
	        "  -x, --get, --extract    extract files from an archive\n"
	        "      --delete            delete from the archive (not on mag tapes!)\n"
	        "\n"
	        "Operation modifiers:\n"
	        "  -G, --incremental          handle old GNU-format incremental backup\n"
	        "  -g, --listed-incremental=FILE\n"
	        "                             handle new GNU-format incremental backup\n"
	        "\n"
	        "Archive format selection:\n"
	        "  -V, --label=NAME                   create archive with volume name NAME\n"
	        "              PATTERN                at list/extract time, a globbing PATTERN\n")
		ok++;

	if (ok == 1)
	{
		cout << "PASS" << endl;
	}
	else
	{
		cout << "FAIL" << endl;
		bad++;
	}

	ok = 0;
	cout << "Testing arrays... ";
	ArgParse::apintvec foo;
	ArgParse::apfloatvec bar;
	ArgParse::apstringvec baz;
	ArgParse t3;
	t3.argInts("foo", "", &foo);
	t3.argFloats("bar", "", &bar);
	t3.argStrings("baz", "", &baz);

	av[0] = "eins";
	av[1] = "-baz=one";
	av[2] = "-foo=1";
	av[3] = "zwei";
	av[4] = "drei";
	av[5] = "-baz";
	av[6] = "two";
	av[7] = "vier";
	av[8] = "-foo=2";
	av[9] = "-bar=1";
	av[10] = "-bar=2";
	av[11] = "-bar";
	av[12] = "3";
	av[13] = "-bar:4";
	av[14] = "-foo=3";
	av[15] = "--baz=three";
	av[16] = "-baz:four";
	av[17] = "-foo=4";
	if (t3.parse(18, av) && checkArrays(foo, bar, baz, t3.leftovers()))
		ok++;

	foo.clear();
	bar.clear();
	baz.clear();
	ArgParse t4;
	t4.argInts("foo", "", &foo, ArgParse::SEP_ARGV);
	t4.argFloats("bar", "", &bar, ArgParse::SEP_ARGV);
	t4.argStrings("baz", "", &baz, ArgParse::SEP_ARGV);
	av[0] = "eins";
	av[1] = "zwei";
	av[2] = "-baz=one";
	av[3] = "two";
	av[4] = "-foo";
	av[5] = "1";
	av[6] = "2";
	av[7] = "3";
	av[8] = "-baz=three";
	av[9] = "-bar=1";
	av[10] = "-bar=2";
	av[11] = "-bar";
	av[12] = "3";
	av[13] = "4";
	av[14] = "-foo=4";
	av[15] = "--baz";
	av[16] = "four";
	av[17] = "--";
	av[18] = "drei";
	av[19] = "vier";
	if (t4.parse(20, av) && checkArrays(foo, bar, baz, t4.leftovers()))
		ok++;

	foo.clear();
	bar.clear();
	baz.clear();
	ArgParse t5;
	t5.argInts("foo", "", &foo, ',');
	t5.argFloats("bar", "", &bar, ',');
	t5.argStrings("baz", "", &baz, ',');
	av[0] = "-foo";
	av[1] = "1,2";
	av[2] = "eins";
	av[3] = "-baz=one,two,three";
	av[4] = "zwei";
	av[5] = "-bar:1,2,3,4";
	av[6] = "--baz=four";
	av[7] = "drei";
	av[8] = "-foo";
	av[9] = "3,4";
	av[10] = "vier";
	if (t5.parse(11, av) && checkArrays(foo, bar, baz, t5.leftovers()))
		ok++;

	if (ok == 3)
	{
		cout << "PASS" << endl;
	}
	else
	{
		cout << "FAIL" << endl;
		bad++;
	}

	return bad;
}
