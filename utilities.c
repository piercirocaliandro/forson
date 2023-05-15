/*
utilities.c -- various utility functions definition
Copyright 2005 Alfonso Tarantini
This file is part of Forson.

Forson is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Forson is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Forson; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include <generation.h>
#include <time.h>

extern char *optarg;
extern FILE *message_stream;
extern int verb_policy[NUMBER_OF_SOURCES];
extern char * source_names[NUMBER_OF_SOURCES];
extern int verbosity;


/*INITIALIZE RANDOM NUMBER GENERATOR*/
void
set_random_seed()
{	
	srandom(time(NULL));
}


/*ATTEMPT TO OPEN INDICATED FILENAME AND CHECK FOR ERRORS*/
FILE *
open_file_write(char *string)
{
	FILE *f = NULL;
	
	f=fopen(string, "w");
	if(f == NULL)
	{
		error(BAD_ARGUMENTS, errno, "%s", string);
		return NULL;
	}
	else 
		return f;
}

/*ATTEMPT TO OPEN INDICATED FILENAME AND CHECK FOR ERRORS*/
FILE *
open_file_read(char *string)
{
	FILE *f = NULL;
		
	f = fopen(string, "r");	
	if (f == NULL)
	{
		error(BAD_ARGUMENTS, errno, "%s", string);
		return NULL;
	}
	else 
		return f;
}


/*READS A STRING AND RETURNS A POSITIVE NUMBER OR EXITS WITH AN ERROR*/
int
read_number(char *string)
{
	int i=0, j=1, n=0;
	char *read_number_err_mess =
        "positive integer required";

	assert(string != NULL);

	while(string[i] != '\0')
	{
		j = ((j==1) && (isdigit(string[i]) == 0)? 0:1);
		i++;
	}
	if (j==0)
		error(BAD_ARGUMENTS, 0, "%s: %s", read_number_err_mess, optarg);

	n = atoi(string);
	if (n<0)
	{
		error(BAD_ARGUMENTS, 0, "%s: %s", read_number_err_mess, optarg);
		return 0;
	}
	else
		return n;	
}


/*DECIDES IF TO PRINT MESSAGES FROM VARIOUS SOURCES ACCORDING TO A*/
/*VERBOSITY POLICY.*/
int
must_print_message(source_type class)
{	
	return (verb_policy[class] <= verbosity)? 1:0;
}

/*WRAPPER FUNCTION FOR MALLOC, CHECKING IT DOES NOT RETURN ZERO*/
void *
xmalloc(size_t size)
{
	void *m;	

	m = malloc(size);
	
	if(m == NULL)
	{
		error(UNEXPECTED_ERROR, 0, "%s", "unexpected error: out of memory");
		return NULL;
	}
	else
		return m;
}

/*WRAPPER FUNCTION FOR MALLOC, CHECKING IT DOES NOT RETURN ZERO*/
void *
xcalloc(size_t nmemb, size_t elem_sz){
	void *m;
	m = calloc(nmemb, elem_sz);

	if(m==NULL){
		error(UNEXPECTED_ERROR, 0, "%s", "unexpected error: out of memory (xcalloc)");
		return NULL;
	}else	
		return m;
}

/*DISPLAYS VERSION INFORMATION. USED WITH -v, --version OPTION*/
void
print_version()
{
	char *mess = 
	"Forson 0.2\nCopyright (C) 2005 Alfonso Tarantini\nForson comes with NO WARRANTY,\nto the extent permitted by law.\nYou may redistribute copies of Forson\nunder the terms of the GNU General Public License.\nFor more information about these matters,\nsee the file named COPYING.\n";

	fprintf(stdout, mess);
}

/*DISPLAYS USAGE AND OPTION INFORMATION. USED WITH -h, --help OPTION*/
void
print_usage()
{
	char * line1 = 
		"Usage: forson [OPTION] GRAMMAR FILE [LEXICON FILE]\n";
	char * line2 = 
		"Generates sentences conforming to the syntax contained in an input grammar file in bison's format \n";
	char * line3 = 
		"\n";
	char * line4 =
		"Mandatory arguments to long options are mandatory for short options too\n";
	char * line5 = 
		"-c, --coverage		enables the generation of sentences to cover all grammar rules\n";
	char * line6 = 
		"			default is random generation\n";
	char * line7 = 
		"-h, --help		displays this help message\n";
	char * line8 =
		"-m, --message FILE	instructs forson to print messages to FILE\n";
	char * line9 =
		"			default is stdout\n";
	char * line10 =
		"-n, --no-spaces         instructs forson to not generate blank text in sentences\n";
	char * line11 =
		"-o, --out FILE		filename for output of the generated sentences\n";
	char * line12=
		"			default is file \"o.out\"\n";
	char * line13=
		"-O, --standard-output	instructs forson to output generated sentences to stdout instead of a file\n";
	char * line14=
		"-p, --print-tables	instructs forson to print it's internal symbol table to the message stream\n";
	char * line15=
		"-r, --repeat N		instructs forson to generate N random sentences\n";
	char * line16=
		"			default is 1\n";
	char * line17=
		"			ignored if the -c option is set\n";
	char * line18=
		"-s, --separator [str]	sets the separator between sentences to \"str\"\n";
	char * line19=
		"			ignored if repeat is set to 1 (default)\n";
	char * line20=
		"			default is 2 \"newlines\"\n";
	char * line21=
		"-v, --verbosity N	sets verbosity level (value overrided to a maximum of 6)\n";
	char * line22=
		"			default is 0\n";
	char * line23=
		"e, --version		prints version information and exits\n";
	char * line24=
		"\n";
	char * line25=
		"Report bugs to <isit81@fastwebnet.it>\n";

	printf(line1);
	printf(line2);
	printf(line3);
	printf(line4);
	printf(line5);
	printf(line6);
	printf(line7);
	printf(line8);
	printf(line9);
	printf(line10);
	printf(line11);
	printf(line12);
	printf(line13);
	printf(line14);
	printf(line15);
	printf(line16);
	printf(line17);
	printf(line18);
	printf(line19);
	printf(line20);
	printf(line21);
	printf(line22);
	printf(line23);
	printf(line24);
	printf(line25);
}
