/*
build_tables.c -- construction of data structures from input files
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

/*OUTPUT STREAMS USED FOR SCANNER ECHOING. DEFINED IN SCANNER SOURCES, BUT REASSIGNED HERE*/
extern FILE *yyout, *yy_lexiconout;

extern FILE *message_stream, *input_grammar_stream, *input_lexicon_stream, *null_stream;

extern char *input_grammar_file_path;
extern char *input_lexicon_file_path;

extern char *yy_lexicontext;
extern int yy_lexiconlineno;

extern short int input_lexicon_flag;
extern symbol_id current_symbol_for_lexicon;

extern symbol_id starting_symbol;

extern symbol_list_entry *symbol_table;

/*BUILD SYMBOL TABLE AND LEXICON DATA STRUCTURES*/
void
build_tables()
{
	int ret_v = 0;

	if(must_print_message(MAIN))
		fprintf(message_stream, "building symbol table...\n");

	symbol_table = initialize_new_symbol_list();
	assert(symbol_table != NULL);
	
	/*SET INPUT STREAM FOR GRAMMAR SCANNER TO USER SPECIFIED GRAMMAR FILE*/
	yyrestart(input_grammar_stream);
	/*REASSIGN OUTPUT STREAM FOR PARSER ECHO ACTIONS (OUTPUT DISCARDED)*/
	yyout = null_stream;

	/*CALL THE GRAMMAR FILE PARSER. CALL MAY NOT RETURN IN CASE OF SEMANTICAL ERRRORS*/
	ret_v = yyparse();
	if(must_print_message(MAIN))
		fprintf(message_stream, "done parsing: %d\n", ret_v);
	if(ret_v != 0)
		error(BAD_INPUT, 0, "%s", "error in y file");

	/*RUN THE LEXICON FILE SCANNER*/
	if(input_lexicon_flag != 0)
	{
		if(must_print_message(MAIN))
			fprintf(message_stream, "scanning input lexical file...\n");

		do_lexicon_scanning();

		if(must_print_message(MAIN))
			fprintf(message_stream, "done, lexicon table built\n");
	}
}


/*CHECK GRAMMAR CONSISTENCY*/
void
check_grammar(symbol_list_entry *work_sle, symbol_id starting_symbol)
{
	int i;
	short unsigned max_rules = 0;
	int check_err_ret_val = 1;
	symbol_list_entry *s = NULL, *t = NULL;

	assert(work_sle != NULL);
	if(must_print_message(MAIN))
		fprintf(message_stream, "checking grammar for correctness...\n");
	s = get_symbol(work_sle, starting_symbol);
	assert(s != NULL);
	
	/*SET visited FIELD TO ONE IN ALL SYMBOLS AND RULES*/
	/*ALSO CHECKS FOR UNDEFINED SYMBOLS (EXITS WITH ERROR)*/
	visit_symbol_list(s, work_sle, 1);
	
	/*CHECK FOR UNREACHABLE SYMBOLS*/
	for(i=1; i <= work_sle->rulecount; i++)
	{
		t = get_symbol(work_sle, i);
		assert(t != NULL);
		if(t->visited == 0)
		{
			if(is_NT(t))
			{
				error(BAD_INPUT, 0, "unreachable nonterminal symbol: \"%s\"", t->name);
			}
			else
			{
				if(must_print_message(WARNING))
					fprintf(message_stream, "warning: token declared but never used: \"%s\"\n", t->name);
			}
		}
	}
	/*SET visited FIELD BACK TO ZERO IN ALL SYMBOLS AND RULES */
	visit_symbol_list(s, work_sle, 0);

	/*CHECK FOR SYMBOLS USED FOR ERROR TRACKING ONLY */
	/*THEY MUST BE REMOVED FROM TARGET GRAMMAR       */
	/*FROM A SENTENCE GENERATION POINT OF VIEW       */
	while(check_err_ret_val == 1)
	{
		check_err_ret_val = check_error_only(work_sle);
	}
	if(must_print_message(MAIN))
		fprintf(message_stream, "...done\n");	

	/*REMOVE MULTIPLE COPIES OF RULES, AND NORMALIZE probability VALUES*/
	normalize_rules(symbol_table);

	/*CHECK FOR INFINITE LOOPS IN GRAMMAR DATA STRUCTURE            */
	/*CICLE THROUGH ALL SYMBOLS AND REPORT IF A LOOP IS FOUND       */
	/*ALSO COMPUTE THE MAXIMUM OF THE NUMBER OF RULES IN A SYMBOL   */
	for(i = 1; i <= work_sle->rulecount; i++)
	{
		int c = 0;
		symbol_list_entry *sle_to_check = NULL;

		sle_to_check = get_symbol(symbol_table, i);

		assert(sle_to_check != NULL);

		c = check_infinite_loops(sle_to_check);

		if(c == 1)
		{
			error(BAD_INPUT, 0,
			"infinite generation derives from symbol \"%s\"",
			sle_to_check->name);
		}

		if(is_NT(sle_to_check) == 1)
		{
			if(max_rules < sle_to_check->rulecount)
			{
				max_rules = (short unsigned)sle_to_check->rulecount;
			}
		}
	}

	/*ASSIGN THE MAXIMUM NUMBER OF RULES IN A SYMBOL */
	/*TO THE HEAD NODE'S 'visited' FIELD             */
	work_sle->visited = max_rules;

	if(must_print_message(MAIN))
		fprintf(message_stream, "...grammar is OK\n");
}


/*CHECKS FOR SYMBOLS USED ONLY FOR ERROR CHECKING (NO RULES)    */
/*IT RETURNS 0 IF ALL WORK IS DONE, 1 IF IT NEADS TO RUN AGAIN  */
int
check_error_only(symbol_list_entry *work_sle)
{
	int i;
	symbol_list_entry *current_symbol = NULL;

	assert(work_sle != NULL);

	if(must_print_message(MAIN))
		fprintf(message_stream, "Checking for error-only symbols...\n");

	for(i=1; i <= work_sle->rulecount; i++)
	{
		current_symbol = get_symbol(work_sle, i);
		assert(current_symbol != NULL);
	
		/*NOTHING IS DONE IF NOT A NON TERMINAL SYMBOL*/
		if(is_NT(current_symbol) == 0)
		{
			continue;
		}

		/*current_symbol SURELY NON-TERMINAL AT THIS POINT */
		/*CHECK IF IT HAS AT LEAST ONE RULE                */
		/*IF NOT, IT WAS USED ONLY FOR ERROR TRACKING SO   */
		/*IT MUST BE EPURATED FROM ALL RULES IT APPEARS IN */
		assert(current_symbol->rulecount >= 0);

		if(current_symbol->rulecount == 0)
		{
			/*IF THE STARTING SYMBOL IS ERROR-ONLY, WE'RE DONE*/
			if(current_symbol->id == starting_symbol)
			{
				error(BAD_INPUT, 0, "Starting symbol has no valid rules: it was found to have only rules containing the 'error' reserved token");
			}
			else
			{
				if(must_print_message(WARNING))
				{
					fprintf(message_stream, "warning: a non-terminal symbol was found to have only rules containing the 'error' reserved token\nthe symbol must be epurated from all rules: \"%s\"\n", current_symbol->name);
				}
				epure_symbol(current_symbol, work_sle);
				return 1;
			}
			
		}
	}

	/*IF NO SYMBOLS WITHOUT RULES FOUND, JOB IS DONE: RETURN 0*/
	return 0;
}


/*TRASFORMS SYMBOL TABLE BY PUTTING RULES IN A "CANONICAL" FORM */
void
normalize_rules(symbol_list_entry *work_sle)
{
	int i;

	if(must_print_message(MAIN))
		fprintf(message_stream, "Normalising rules...\n");

	assert(work_sle != NULL);

	/*CYCLE THROUGH ALL SYMBOLS IN TABLE*/
	for (i=1; i <= work_sle->rulecount; i++)
	{
		int accum = 0, sum = 0, j;
		symbol_list_entry *current_symbol = NULL;

		current_symbol = get_symbol(work_sle, i);

		/*NOTHING TO BE DONE IF SYMBOL IS NOT A NON-TERMINAL*/
		if(is_NT(current_symbol) == 0)
		{
			continue;
		}
		assert(current_symbol->rulecount > 0);

		/*CHECK FOR TWO RULES WITH THE SAME COMPONENTS. THE ONE WITH THE HIGHEST*/
		/*POSITION IS REMOVED. THE probability FIELDS WILL BE SUMMED  AND       */
		/*THE RESULT IS STORED IN THE REMAINING RULE*/
		for(j = 1; j <= current_symbol->rulecount-1; j++)
		{
			rule_list_entry *first = NULL; 
			int i; 

			first = get_rle(current_symbol, j);
			assert(first != NULL);
			
			for(i=j+1; i <= current_symbol->rulecount; i++)
			{
				int equal = 1;
				rule_list_entry *second = NULL;

				second = get_rle(current_symbol, i);
				assert(second != NULL);

				if(first->length == second->length)
				{
					int h;
					for(h = 0; h < first->length; h++)
					{
						symbol_id f = 0, s = 0;
						f=extract_symbol_rle(first, h);
						s=extract_symbol_rle(second, h);
						assert(f != 0);
						assert(s != 0);

						if(f != s)
						{
							equal = 0;
							break;
						}	
					}
				}
				else
				{
					equal = 0;
				}

				/*FOUND TWO IDENTICAL RULES*/
				if(equal == 1)
				{
					if(must_print_message(WARNING))
						fprintf(message_stream, "warning: found two identical rules (probabilities have been merged) in symbol: \"%s\", rules %d and %d\n", current_symbol->name, j, i);

					first->probability += second->probability;
					remove_rle(current_symbol, i);
					i--;
				}
				else
					continue;
			}
		}

		/*SUM UP THE VALUES DEFINED IN GRAMMAR INPUT FILE (OR DEFAULTS)*/
		for(j=1; j <= current_symbol->rulecount; j++)
		{
			rule_list_entry	*current_rule=NULL;

			current_rule = get_rle(current_symbol, j);
			assert(current_rule != NULL);

			assert(current_rule->probability >= 0);

			sum += current_rule->probability;
		}

		/*NORMALIZE AGAINST INT_MAX */
		for(j=1; j <= current_symbol->rulecount; j++)
		{
			rule_list_entry	*current_rule=NULL;
			float share = 0.0;

			current_rule = get_rle(current_symbol, j);
			assert(current_rule != NULL);
			assert(sum != 0);

			share = (float)(current_rule->probability) / (float)sum;

			if(j == current_symbol->rulecount)
				accum = INT_MAX;
			else
				accum += ((int)((float)INT_MAX*share));

			current_rule->probability = accum;
			
			/*TAKE ADVANTAGE OF THIS CICLE ALSO TO CHARACTERIZE THE RULE TYPE*/
			{
				current_rule->type = determine_rule_type(current_symbol->id, current_rule, symbol_table);
			}
		}
	}

	if(must_print_message(MAIN))
		fprintf(message_stream, "...done\n");
}


/*CHECK FOR UNTERMINATING LOOPS OF NON_TERMINALS IN RULES                */
/*NOT CHECKING FOR THESE CONDITIONS IN TARGET GRAMMAR                    */
/*WOULD LEED TO SEGMENTATION FAULT DUE TO INFINITE RECURSION OF "grow()" */
/*IF FOUND RETURN 1, ELSE RETURN 0                                       */
int
check_infinite_loops(symbol_list_entry *sle)
{
	int i;

	assert(sle != NULL);
	assert(sle->id > 0);

	/*MARK THE SYMBOL (FOR SIMULATING "PRESENCE IN THE STACK" CHECKING)*/
	/*THE FUNCTION WILL UNMARK THE SYMBOL WHEN IT RETURNS              */
	sle->visited = 1;

	/*IF SYMBOL IS NOT A NON-TERMINAL, IT TRIVIALLY DOES NOT LEAD TO CYCLES*/
	if(is_NT(sle) == 0)
	{
		sle->visited = 0;
		return 0;
	}

	/*CYCLE THROUGH RULES. IF THE SYMBOL CONTAINS EVEN ONE "ONLY-TERMINAL" RULE, */
	/*IT CANNOT BE HELD RESPONSIBLE FOR LOOPS. FOR THIS REASON WE MUST REITERATE */
	/*THE ALGORITHM FOR EVERY SYMBOL ( IN check_grammar() )                      */
	/*IF A RULE CONTAINS NON-TERMINALS, THEY MUST BE CHECKED RECURSIVELY         */
	for(i = 1 ; i <= sle->rulecount; i++)
	{
		rule_list_entry *rle = NULL;		

		rle = get_rle(sle, i);
		assert(rle != NULL);

		/*LITTLE PERFORMANCE HACK. EXPLOITS THE FACT THAT */
		/*RULE TYPE HAS ALREADY BEEN DETERMINED           */
		if(is_ONLY_TERMINALS(rle))
		{
			sle->visited = 0;
			return 0;
		}
		/*MAIN ALGORITHM FOR RULE CHECKING.                                */
		/*IF THE RULE CONTAINS A SYMBOL ALREADY MARKED, IT MEANS RECURSION.*/
		/*THE RULE IS DANGEROUS                                            */
		/*IF THE RULE CONTAINS ONLY SYMBOLS NOT ALREADY MARKED, THE RULE IS*/
		/*MARKED AS DANGEROUS IF EVEN ONE OF THEM LEADS TO A LOOP          */
		{
			int j, dangerous = 0;

			for(j = 0; j < rle->length; j++)
			{
				symbol_id s = 0;
				symbol_list_entry *sle = NULL;

				s = extract_symbol_rle(rle, j);
				assert(s != 0);
				sle = get_symbol(symbol_table, s);
				assert(sle != NULL);

				if(is_NT(sle) == 1)
				{
					if(sle->visited == 0)
					{
						dangerous = 
						(dangerous || check_infinite_loops(sle));
					}
					else
					{
						dangerous = 1;
					}
				}
			}
			/*IF THE RULE IS NOT MARKED AS DANGEROUS, THEN IT DERIVES SYMBOLS*/
			/*WICH LEAD TO TERMINALS. THE SYMBOL DOES NOT LEAD TO AN INF LOOP*/
			if(dangerous == 0)
			{
				sle->visited = 0;
				return 0;
			}
		}
	}
	
	/*IF THE FUNCTION HAS NOT ALREADY RETURNED, THEN THE SYMBOL DOES LEAD TO AN INF LOOP*/
	sle->visited = 0;
	return 1;
}


/*ELIMINATES ALL OCURRENCES OF SYMBOL sle FROM symbol_table               */
/*IF A RULE CONTAINED ONLY sle, THE RULE ITSELF SHOULD BE REMOVED AS WELL */
void epure_symbol(symbol_list_entry *sle, symbol_list_entry *symbol_table)
{
	int loc[2], i;
	symbol_list_entry *current_symbol = NULL;
	/*INITIALIZE LOCATION BUFFER*/
	loc[0] = loc[1] = 0;

	assert(sle != NULL);
	assert(symbol_table != NULL);
	
	/*CYCLE THROUGH ALL SYMBOLS IN THE TABLE*/
	for(i=1; i <= symbol_table->rulecount; i++)
	{
		current_symbol = get_symbol(symbol_table, i);
		assert(current_symbol != NULL);

		if(is_NT(current_symbol) == 0)
		{
			continue;
		}

		locate_symbol_in_sle(sle->id, current_symbol, loc);
		if(loc[0] != 0)
		{
			remove_rle(current_symbol, loc[0]);
			i--;
		}
	}

	remove_sle(symbol_table, sle->id);
}


/*MANAGE THE PARSING OF THE LEXICON FILE BY CYCLING SCANNER FUNCTION 'yy_lexiconlex()'*/
/*THERE IS NO NEED FOR A BISON SCANNER FOR THIS FUNCTIONALITY                         */
void
do_lexicon_scanning()
{
	yy_lexiconrestart(input_lexicon_stream);
	/*REASSIGN OUTPUT STREAM FOR PARSER ECHO ACTIONS (OUTPUT DISCARDED)*/
	yy_lexiconout = null_stream;

	while(1)
	{
		int i=0;

		i = yy_lexiconlex();

		if(i == END_OF_FILE)
		{
			break;
		}
		else if(i < END_OF_FILE)
		{
			switch(i)
			{
				case UNEXPECTED_IDENTIFIER:
					if(must_print_message(WARNING))
						fprintf(message_stream, "warning: %s: line %d: unexpected identifier, scanning stopped: %s\n", input_lexicon_file_path, yy_lexiconlineno, yy_lexicontext);
					return;
				case UNEXPECTED_SEMICOLON:
					if(must_print_message(WARNING))
						fprintf(message_stream, "warning: %s: line %d: unexpected semicolon, scanning stopped: %s\n", input_lexicon_file_path, yy_lexiconlineno, yy_lexicontext);
					return;
				case UNEXPECTED_LEXICAL_UNIT:
					if(must_print_message(WARNING))
						fprintf(message_stream, "warning: %s: line %d: unexpected lexical unit, scanning stopped: %s\n", input_lexicon_file_path, yy_lexiconlineno, yy_lexicontext);
					return;
				case UNEXPECTED_EOF:
					if(must_print_message(WARNING))
						fprintf(message_stream, "warning: %s: line %d: unexpected EOF, scanning stopped\n", input_lexicon_file_path, yy_lexiconlineno);
					return;
				case EMPTY_STRING_LITERAL:
					if(must_print_message(WARNING))
						fprintf(message_stream, "warning: %s: line %d: found empty string literal, scanning stopped\n", input_lexicon_file_path, yy_lexiconlineno);
					return;
				case UNEXPECTED_CHARACTER:
					if(must_print_message(WARNING))
						fprintf(message_stream, "warning: %s: line %d: found unexpected character, scanning stopped\n", input_lexicon_file_path, yy_lexiconlineno);
					return;
				default:
					/*NOTHING ELSE SHOULD BE RETURNED BY SCANNER*/
					assert(0);
					abort();
			}
		}
		else if(i == ASSOCIATED_LEXICAL_UNIT)
		{
			if((int)current_symbol_for_lexicon > 0)
			{
				add_lexical_unit(current_symbol_for_lexicon);
			}
		}
		else
		{
			assert(0);
			abort();
		}
	}
}


/*ADD A LEXICAL UNIT TO THE argz STRUCTURE FOR TERMINAL SYMBOLS */
/*TEXT IS READ FROM STRING 'yy_lexicontext', SET BY FLEX SCANNER*/
void
add_lexical_unit(symbol_id s_id)
{
	symbol_list_entry *sle = NULL;
	error_t err = 0;
	int num = 0;

	assert((int)s_id > 0);

	sle = get_symbol(symbol_table, s_id);
	assert(sle != NULL);

	num = get_lexicon_numerosity(sle);
	assert(num >= 0);

	if(num == 0)
	{
		lexicon_argz_structure *lazs = NULL;

		lazs = initialize_new_lexicon_argz_structure();
		assert(lazs != NULL);
		sle->rules = (rule_list_entry *) lazs;

		increment_lexicon_numerosity(sle);
		err = argz_add(&(lazs->argz), &(lazs)->argz_size, yy_lexicontext);
	}
	else
	{
		lexicon_argz_structure *lazs = NULL;

		lazs = (lexicon_argz_structure *) sle->rules;
		assert(lazs != NULL);

		increment_lexicon_numerosity(sle);
		err = argz_add(&(lazs->argz), &(lazs)->argz_size, yy_lexicontext);
	}

	if(err != 0)
	{
		error(UNEXPECTED_ERROR, 0, "unexpected error: out of memory");
	}
}


/*RETURN THE CORRECT TYPE LABEL FOR A RULE, DEPENDING ON IT'S CONTENTS*/
/*GLOBAL SYMBOL TABLE PROVIDED FOR SYMBOL LOOK-UP                     */
rule_type
determine_rule_type(symbol_id self_id, rule_list_entry *rle, symbol_list_entry *symbol_table)
{
	assert(symbol_table != NULL);
	assert(rle != NULL);
	assert(self_id != 0);

	if(rle->length == 0)
	{
		return EMPTY;
	}
	else if(rle->length == 1)
	{
		symbol_id s = 0;
		symbol_list_entry *sle = NULL;

		s = extract_symbol_rle(rle, 0);
		assert(s != 0);

		if(s == self_id)
		{
			return AUTO_COPY;
		}
		else
		{
			sle = get_symbol(symbol_table, s);
			assert(sle != NULL);

			if(is_NT(sle))
			{
				return COPY;
			}
			else if(is_LITERAL(sle))
			{
				return ALIAS;
			}
			else
				return TERMINAL;
		}
	}
	else
	{
		int left_rec_flag = 0, right_rec_flag = 0;
		int i, only_terminal_flag = 1;
		symbol_id s = 0;

		s = extract_symbol_rle(rle, 0);
		assert(s != 0);

		if(s == self_id)
		{
			left_rec_flag = 1;
		}

		s = extract_symbol_rle(rle, (rle->length)-1);
		assert(s != 0);
		if(s == self_id)
		{
			right_rec_flag = 1;
		}
		if(left_rec_flag && right_rec_flag)
			return MULTIPLE_RECURSIVE;
		else if(left_rec_flag)
			return LEFT_RECURSIVE;
		else if(right_rec_flag)
			return RIGHT_RECURSIVE;

		for(i=0; i < rle->length; i++)
		{
			symbol_list_entry *curr_sym_pointer = NULL;
			symbol_id curr_sym_id = 0;

			curr_sym_id = extract_symbol_rle(rle, i);
			assert(curr_sym_id);
			curr_sym_pointer = get_symbol(symbol_table, curr_sym_id);
			assert(curr_sym_pointer != NULL);

			if(is_NT(curr_sym_pointer))
			{
				if(curr_sym_id == self_id)
					return RECURSIVE;
				else
					only_terminal_flag = 0;
			}
		}
		if(only_terminal_flag)
			return TERMINAL;
	}

	/*NO PARTICULAR CONDITIONS HOLD, ASSIGN 'STANDARD' TYPE LABEL*/
	return STANDARD;
}

