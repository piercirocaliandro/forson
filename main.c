/*
main.c -- option handling, main function and clean-up
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

/****************************/
/*GLOBAL VARIABLE DEFINITION*/
/****************************/

/*GLOBAL SYMBOL TABLE FOR SENTENCE GENERATION*/
symbol_list_entry *symbol_table = NULL;
/*ID OF STARTING SYMBOL OF TARGET GRAMMAR*/
/*IT IS SET BY THE PARSER                */
symbol_id starting_symbol = (symbol_id)0;

/*VALUE SET BY LEXICON SCANNER TO SPECIFY WHERE TO APPEND LEXICON UNITS*/
symbol_id current_symbol_for_lexicon;

/*STRUCTURES FOR CHARACTERIZING SYMBOL TYPES*/
int rc_values[] = RC_VALUES;

/*STRUCTURES FOR CHARACTERIZING RULE TYPES*/
char *rule_type_names[NUMBER_OF_RULE_TYPES] = RULE_TYPE_NAMES;

/*STRUCTURES FOR VERBOSITY POLICY ENFORCEMENT*/
int verb_policy[NUMBER_OF_SOURCES]=VERB_POLICY;
int verbosity = DEFAULT_VERBOSITY;

/*FLAG FOR DECIDING WHETHER TO PRINT SYMBOL TABLE PRIOR TO SENTENCE GENERATION*/ 
short int print_table_flag = DEFAULT_PRINT_TABLE_FLAG;
/*FLAG FOR INDICATING USE OF stdout INSTEAD OF FILE*/
short int standard_output_flag = DEFAULT_STANDARD_OUTPUT_FLAG;
/*FLAG FOR INDICATING USE OF AN EXTRA LEXICON FILE*/
short int input_lexicon_flag = 0;
/*FLAG FOR INDICATING THE REQUEST OF A COVERAGE SENTENCE GENERATION*/
/*AS OPPOSED TO THE DEFAULT RANDOM GENERATION*/
short int coverage_flag = 0;
/*FLAG FOR INDICATING THAT SPACES SHOULD NOT BE GENERATED IN SENTENCES*/
short int no_spaces_flag = 0;

/*I/O STREAMS USED THROUGHOUT THE SOURCES*/
FILE *output_stream=NULL, *input_grammar_stream=NULL, *input_lexicon_stream=NULL;
FILE *message_stream=NULL, *null_stream=NULL;

/*PATH (FILENAME) TO USER SPECIFIED INPUT GRAMMAR FILE*/
char *input_grammar_file_path = NULL;
/*PATH (FILENAME) TO USER SPECIFIED INPUT LEXICON FILE*/
char *input_lexicon_file_path = NULL;
/*PATH (FILENAME) TO USER SPECIFIED OUTPUT FILE*/
char *output_file_path = NULL;


/***************************************************************/


/***************/
/*MAIN FUNCTION*/
/***************/

int
main(int argc, char **argv)
{
	int i=0, j, at_exit_return=0;
	char *sentence_separator = DEFAULT_SENTENCE_SEPARATOR;
	int repeat = DEFAULT_REPEAT;
	symbol_list_entry *s = NULL;

	/*REGISTER CLEANUP FUNCTION*/
	at_exit_return = atexit(clean_up);
	if(at_exit_return != 0)
		error(UNEXPECTED_ERROR, 0, "could not register clean-up function");

	/*ASSIGN MESSAGE STREAM DEFAULT FALLBACK*/
	message_stream = stdout;

	/*ASSIGN NULL STREAM TO DEFAULT LINUX THROW-AWAY FILE*/
	null_stream = fopen(DEFAULT_NULL_PATH, "w");
	if(null_stream == NULL)
	{
		error(UNEXPECTED_ERROR, errno, "could not open /dev/null");
	}

	/* OPTION HANDLING, QUITE SELF-DESCRIPTIVE*/
	while(1)
	{
		int option_index=0;
		static const struct option long_options[]= 
		{	
			{"coverage",	no_argument,		0,	'c'},
			{"help",	no_argument,		0,	'h'},
			{"message",	required_argument,	0,	'm'},
			{"no-spaces",	no_argument,		0,	'n'},
			{"output", 	required_argument,	0,	'o'},
			{"print-tables",no_argument,		0,	'p'},
			{"repeat",	required_argument, 	0, 	'r'},
			{"separator",	optional_argument,	0,	's'},
			{"standard-output", no_argument,	0,	'O'},
			{"verbosity",	required_argument,	0,	'v'},
			{"version",	no_argument,		0,	'e'},
			{0,		0,			0,	0}
		};
		static const char *short_options = "cehm:no:Opr:s::v:";
 
		i=getopt_long(argc, argv, short_options, long_options, &option_index);
		if(i==-1) break;
		
		switch(i)
		{
		case 'c':
			coverage_flag = 1;
			break;
		case 'e':
			print_version();
			exit(0);
		case 'h':
			print_usage();
			exit(0);
		case 'm':
			message_stream = open_file_write(optarg);
			break;
		case 'n':
			no_spaces_flag = 1;
			break;
		case 'o':
			if(standard_output_flag == 1)
				error(BAD_ARGUMENTS, 0, "previously used -O option, incompatible with -o");
			else
			{
				output_file_path = optarg;
				break;
			}
		case 'O':
			if(output_file_path != NULL)
				error(BAD_ARGUMENTS, 0, "previously used -o option, incompatible with -O");
			else
			{
				output_stream = stdout;
				standard_output_flag = 1;
				break;
			}
		case 'p':
			print_table_flag = 1;
			break;
		case 'r':
			repeat = read_number(optarg);
			break;
		case 's':
			if(optarg != NULL)
				sentence_separator = optarg;
			else
				sentence_separator = "";
			break;
		case 'v':
			verbosity=read_number(optarg);
			verbosity=(verbosity > MAX_VERBOSITY)? MAX_VERBOSITY:verbosity;
			break;
		case '?':
			/*ERROR MESSAGE ALREADY PRINTED BY getopt_long*/
			exit(BAD_ARGUMENTS);
		} 	
	}
	
	/*TEST IF PROGRAM HAS BEEN SUPPLIED WITH ENOUGH NON-OPTION ARGUMENTS*/
	if(optind >= argc)
		error(BAD_ARGUMENTS, 0, "%s", "not enough arguments");

	/*ASSIGN PATH NAME AND OPEN STREAM FOR GRAMMAR INPUT FILE*/
	input_grammar_file_path = argv[optind];
	assert(input_grammar_file_path != NULL);
	input_grammar_stream = open_file_read(input_grammar_file_path);

	/*IF PROVIDED, THE LEXICON FILE IS OPENED            */
	/*NOTE THAT IT MUST PROVIDED -AFTER- THE GRAMMAR FILE*/
	if(argc > optind+1)
	{
		input_lexicon_file_path = argv[optind+1];
		input_lexicon_flag = 1;
		assert(input_lexicon_file_path != NULL);		

		/*OPEN STREAM AND CHECK FOR ERRORS*/
		input_lexicon_stream = open_file_read(input_lexicon_file_path);		
	}

	/*IF OUTPUT PATH NOT ASSIGNED, A DEFAULT FALLBACK IS USED*/
	if (output_file_path == NULL)
	{ 
		output_file_path = DEFAULT_OUTPUT_PATH;
	}
	
	/*CREATE INTERNAL DATA STRUCTURE FROM INPUT GRAMMAR FILE*/
	build_tables();

	/*CHECK CONSISTENCY OF DATA STRUCTURE IN MEMORY*/
	/*FUNCTION DOES NOT RETURN IN CASE OF ERRORS*/
	check_grammar(symbol_table, starting_symbol);

	/*NOW WE SURELY HAVE AN OUTPUT PATH, AND THE PROGRAM HAS RECEIVED GOOD ARGUMENTS*/
	/*SO OPEN THE SELECTED OUTPUT FILE. WE ARE SURE AT THIS POINT WE WON'T CREATE   */
	/*A USELESS FILE                                                                */
	if(standard_output_flag == 0)
	{
		output_stream = fopen(output_file_path, "w");
	}
	if (output_stream == NULL)
	{
		error(UNEXPECTED_ERROR, errno, "%s", output_file_path);
	}


	/*PRELIMINARY ASSERTION CHECKING*/	
	assert(symbol_table != NULL);
	assert(message_stream != NULL);
	assert(output_stream != NULL);
	assert(input_grammar_stream != NULL);
	assert(repeat >= 0);
	assert(starting_symbol !=0);
	assert(starting_symbol <= (symbol_table->rulecount));


	/*PRINTING OF GRAMMAR STRUCTURE TABLE*/
	if(print_table_flag == 1)
	{	
		fprintf(message_stream, "\n");
		print_symbol_list(symbol_table);
		
		if(input_lexicon_flag != 0)
		{
			fprintf(message_stream, "\n");
			print_lexicon_table(symbol_table);
		}
	}

	/*EXTRACTING STARTING SYMBOL*/
	s = get_symbol(symbol_table, starting_symbol);
	assert(s != NULL);
	if(repeat > 0 || coverage_flag == 1)
	{
		if(must_print_message(MAIN))
			fprintf(message_stream, "starting sentence generation, starting symbol is: %s\n", s->name);
	}

	/*INITIALIZE RANDOM NUMBER GENERATOR*/
	set_random_seed();

	/*MAIN CICLE*/
	if(coverage_flag == 1)
	{
		int count = 1;

		if(must_print_message(MAIN))
			fprintf(message_stream, "sentence %d:\n", count);

		while(1)
		{
			rule_list_entry *r_check = NULL, *r_check_deep = NULL;
			purdom(starting_symbol, symbol_table);

			r_check = get_unvisited_rle(s);
			r_check_deep = get_with_deep_unvisited_rle(s, symbol_table);

			if(r_check == NULL && r_check_deep == NULL)
			{
				if(must_print_message(MAIN))
				{
					fprintf(message_stream, "complete coverage reached\n");
				}
				break;
			}
			else
			{
				if(must_print_message(MAIN))
				{
					fprintf(message_stream, "more sentences needed, sentence %d:\n", ++count);
				}

				fprintf(output_stream, sentence_separator);
			}
		}
	}
	else
	{
		for(j=0; j < repeat; j++)
		{
			symbol_list_entry* test_sle = get_symbol_by_name(symbol_table, "add");
			symbol_id test_symb = test_sle->id;
			// grow(starting_symbol, symbol_table);
			grow(test_symb, symbol_table);
			if(j < repeat-1)
				fprintf(output_stream, sentence_separator);
		}
		fputs("\n", output_stream);
	}
	/*CLEAN UP AND EXIT*/
	exit(EXIT_SUCCESS);
}


/*FUNCTION REGISTERED TO BE CALLED "AT EXIT"*/
void
clean_up(void)
{
	int i=0;
	
	/*FREE DINAMICALLY ALLOCATED MEMORY IN DATA STRUCTURES*/
	if(must_print_message(CLEAN_MIN))
		fprintf(message_stream, "starting cleaning...\n");

	if(symbol_table != NULL)
	{
		clean_symbol_list(symbol_table);
	}

	if(must_print_message(CLEAN_MIN))
		fprintf(message_stream, "done cleaning, closing file descriptors and exiting...\n");

	/*CLOSE FILE DESCRIPTORS (IF OPEN) AND CHECK FOR ERRORS*/
	if(message_stream != NULL)
	{
		i = fclose(message_stream);
		if(i != 0)
			error(UNEXPECTED_ERROR, errno, "%s", "failed to close file descriptor for message stream");
	}

	if(input_grammar_stream != NULL)
	{
		i = fclose(input_grammar_stream);
		if(i != 0)
			error(UNEXPECTED_ERROR, errno, "%s", "failed to close file descriptor for input grammar stream");
	}	

	if(input_lexicon_flag == 1)
	{
		if(input_lexicon_stream != NULL)
		{
			i = fclose(input_lexicon_stream);
			if(i != 0)
				error(UNEXPECTED_ERROR, errno, "%s", "failed to close file descriptor for input lexicon stream");
		}
	}

	if(standard_output_flag == 0)
	{
		if(output_stream != NULL)
		{
			i = fclose(output_stream);
			if(i != 0)
				error(UNEXPECTED_ERROR, errno, "%s", "failed to close file descriptor for output stream");
		}
	}

	if(null_stream != NULL)
	{
		i = fclose(null_stream);
		if(i != 0)
			error(UNEXPECTED_ERROR, errno, "%s", "failed to close file descriptor for throw-away stream");
	}
}
