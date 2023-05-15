/*
listops.c -- functions for internal data structure management
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

extern int rc_values[];
extern char *symbol_type_names[];
extern FILE *message_stream;

/*******************************/
/*SYMBOL LIST RELATED FUNCTIONS*/
/*******************************/

/*CREATES THE HEAD NODE OF THE SYMBOL TABLE'S LINKED LIST IMPLEMENTATION*/
/*RETURNS A POINTER TO THE NEWLY ALLOCATED STRUCTURE                    */
symbol_list_entry *
initialize_new_symbol_list()
{
	char *str = "<HEAD NODE>";
	
	symbol_list_entry *s = xmalloc(sizeof(symbol_list_entry)); 
	memset(s, 0, sizeof(symbol_list_entry));
	/*IN HEAD NODE 'rules' IS USED AS TAIL POINTER;*/
	/*         'rulecount' IS USED AS NODE COUNTER;*/
	s->rules = (rule_list_entry *) s;		// let it point to itself
	s->name = xmalloc(strlen(str));
	strcpy(s->name, str);
	
	return s;
}

/*ALLOCATES AND INSERTS A NEW ELEMENT IN SYMBOL TABLE                     */
/*TYPE AND NAME MUST BE SPECIFIED. UNIQUE SYMBOL ID IS RETURNED           */
/*ALL OTHER DATA FIELD IN struct 'symbol_list_entry' ARE SET TO ZERO      */
/*SYMBOL IDs ARE PROGRESSIVE. REMOVING SYMBOLS IS NOT ALLOWED (NOR NEEDED)*/
symbol_id
insert_symbol(symbol_list_entry *l, char *new_name, symbol_type t)
{
	symbol_list_entry *sle = NULL;
	symbol_id new_id = 0;

	/*GO TO TAIL NODE*/
	sle = (symbol_list_entry *)l->rules;
	/*UPDATE COUNTER IN HEAD NODE*/
	l->rulecount++;
	new_id = (symbol_id)l->rulecount;

	if(must_print_message(LISTOPS))
		fprintf(message_stream, "adding symbol: \"%s\"\n", new_name);

	/*ALLOCATE MEMORY AND UPDATE TAIL POINTER IN HEAD NODE*/	
	sle->next = xmalloc(sizeof(symbol_list_entry));
	memset(sle->next, 0, sizeof(symbol_list_entry));	
	l->rules = (rule_list_entry *) sle->next;
	
	/*SET UP NEW SYMBOL NODE*/
	sle = sle->next;

	sle->id = new_id;
	sle->name = xmalloc(strlen(new_name) + sizeof(char));
	strcpy(sle->name, new_name);
	
	switch(t)
	{
	case LEXICAL:
		set_symbol_type(sle, LEXICAL);
		break;
	case LITERAL:
		set_symbol_type(sle, LITERAL);
		break;
	case UNDEFINED:
		set_symbol_type(sle, UNDEFINED);
		break;
	case NT:
		set_symbol_type(sle, NT);
		break;
	default:
		break;
	}

	/*RETURN NEW SYMBOL'S UNIQUE ID*/
	return new_id;
}


/*REMOVE SYMBOL WITH SYMBOL_ID id FROM symbol_table    */
/*ALL SYMBOLS WITH ID > id WILL HAVE AN ID DECREMENT   */
/*ALL REFERENCES IN RULES MUST BE UPDATED TO NEW VALUE */
void
remove_sle(symbol_list_entry *symbol_table, symbol_id id)
{
	symbol_list_entry *s_to_rem = NULL, **pointer_s_to_rem = NULL;
	int i;
	
	assert(id != (symbol_id) 0);
	assert(symbol_table != NULL);
	assert(id <= symbol_table->rulecount);
	
	if(id != 1)
	{
		pointer_s_to_rem = &(get_symbol(symbol_table, id-1)->next);
	}
	else
	{
		pointer_s_to_rem = &(symbol_table->next);
	}

	s_to_rem = get_symbol(symbol_table, id);
	if(must_print_message(LISTOPS))
	{
		fprintf(message_stream, "removing symbol \"%s\" from symbol list, updating rules...\n", s_to_rem->name);
	}	

	*pointer_s_to_rem = s_to_rem->next;
	
	free(s_to_rem->name);
	free(s_to_rem);

	/*UPDATE IDs*/
	for(i=id+1; i <= symbol_table->rulecount; i++)
	{
		symbol_list_entry *sle = NULL;

		sle = get_symbol(symbol_table, i);
		assert(sle != NULL);

		sle->id--;
	}
	symbol_table->rulecount--;

	/*UPDATE RULES*/
	for(i=1; i <= symbol_table->rulecount; i++)
	{
		symbol_list_entry *sle = NULL;
		int j;

		sle = get_symbol(symbol_table, i);
		assert(sle != NULL);

		if(is_NT(sle) == 0)
		{
			continue;
		}

		for(j = 1; j <= sle->rulecount; j++)
		{
			rule_list_entry *rle = NULL;
			int k;

			rle = get_rle(sle, j);
			assert(rle != NULL);

			for(k=0; k < rle->length; k++)
			{
				symbol_id test = (symbol_id) 0;

				test = extract_symbol_rle(rle, k);
				if(test > id)
				{
					modify_symbol_rle(rle, k, test-1);
				}
			}
		}
	}
	
	if(must_print_message(LISTOPS))
	{
		fprintf(message_stream, "...done\n", s_to_rem->name);
	}
}


/*SEARCH FOR SYMBOL id IN LIST l. RETURNS POINTER OR NULL ON FAILURE*/
symbol_list_entry *
get_symbol(symbol_list_entry *l, symbol_id id)
{	
	assert(id != 0);
	assert(l != NULL);
	
	if(must_print_message(LISTOPS))
		fprintf(message_stream, "getting symbol at id: %d\n", id);
	
	while(l != NULL)
	{
		if(l->id == id)
			return l;
		else 
			l = l->next;
	}
	return NULL;
}

/*SEARCH FOR SYMBOL name IN LIST l. RETURNS POINTER OR NULL ON FAILURE*/
symbol_list_entry *
get_symbol_by_name(symbol_list_entry *l, char *name)
{	
	assert(l != NULL);
	assert(name != NULL);
	assert(name[0] != '\0');
	
	if(must_print_message(LISTOPS))
		fprintf(message_stream, "getting symbol with name %s\n", name);

	if(l->id == 0 && l->next == NULL)
		return NULL;
	if(l->id == 0) 
		l=l->next;
	while(l != NULL)
	{
		if(strcmp(name, l->name) == 0)
			return l;
		else 
			l=l->next;
	}
	return NULL;
}

/*LOCATES THE FIRST (RULE-WISE AND LEFT TO RIGHT IN EVERY RULE)     */
/*OCCURRENCE OF SYMBOL WITH ID id IN SYMBOL sle                     */
/*RESULT IS STORED IN loc, A MEMORY LOCATION OF LEAST 2*sizeof(int) */
void
locate_symbol_in_sle(symbol_id id, symbol_list_entry *sle, int *loc)
{
	int i;

	assert(id > 0);
	assert(sle != NULL);
	assert(is_NT(sle) == 1);

	for(i=1; i <= sle->rulecount; i++)
	{
		rule_list_entry *rle = NULL;
		int j;

		rle = get_rle(sle, i);
		assert(rle != NULL);
		
		for(j=0; j < rle->length; j++)
		{
			symbol_id curr = (symbol_id) 0;
			
			curr = extract_symbol_rle(rle,j);
			assert(curr != (symbol_id) 0);

			if(curr == id)
			{
				/*RULE INDEX IS STORED IN FIRST POSITION           */
				/*SYMBOL INDEX IN RULE IS STORED IN SECOND POSITION*/
				loc[0] = i; loc[1] = j;
				return;
			}
		}
	}

	/*IF THE SYMBOL ISN'T FOUND STORE {0,0} AND RETURN           */
	/*SINCE RULE INDEX 0 DOES NOT EXIST, NO AMBIGUITY IS POSSIBLE*/
	loc[0] = 0; loc[1] = 0;
	return;
}

/*VISITS ALL SYMBOLS AND RULES IN symbol_table WHICH ARE REACHABLE FROM l*/
/*THE visited FIELD IS SET TO visited_value*/
void 
visit_symbol_list(symbol_list_entry *l, symbol_list_entry *symbol_table, int visited_value)
{
	int i;
	rule_list_entry *rle = NULL;

	assert(l != NULL);
	
	if(is_UNDEFINED(l))
		error(BAD_INPUT, 0, "symbol not declared as a token but has no rules: \"%s\"", l->name);

	l->visited = visited_value;			

	for(i=1; i <= l->rulecount; i++)
	{
		int j;
		symbol_list_entry *s;


		rle = get_rle(l, i);
		assert(rle != NULL);
		
		rle->visited = visited_value;
		for(j=0; j < rle->length; j++)
		{
			s = get_symbol(symbol_table, extract_symbol_rle(rle,j));
			assert(s != NULL);
			if(s->visited != visited_value)
				visit_symbol_list(s, symbol_table, visited_value);
		}
	}
}

/***********************************************************/
/*FUNCTIONS FOR DETERMINING AND SETTING THE TYPE OF SYMBOLS*/
/***********************************************************/


/*BOOLEAN FUNCTIONS TO TEST SYMBOL TYPE*/
int is_LITERAL(symbol_list_entry *l)
{
	return (l->rulecount == rc_values[LITERAL])? 1:0;
}

int is_LEXICAL(symbol_list_entry *l)
{
	return ((l->rulecount <= rc_values[LEXICAL])
		&& (l->rulecount != rc_values[UNDEFINED])
		&& (l->rulecount != rc_values[RANDOM_LEXICAL]))? 1:0;
}

int is_UNDEFINED(symbol_list_entry *l)
{
	return (l->rulecount == rc_values[UNDEFINED])? 1:0;
}

int is_NT(symbol_list_entry *l)
{
	return (l->rulecount >= rc_values[NT])? 1:0;
}

int is_RANDOM_LEXICAL(symbol_list_entry *l)
{
	return (l->rulecount == rc_values[RANDOM_LEXICAL])? 1:0;
}


/*SETS SYMBOL TYPE (USUALLY FROM UNDEFINED TO A SPECIFIC TYPE)                        */
/*REASSIGNING TYPES DOES NOT LEAD TO DESTRUCTION OF INFORMATION IN THE rulecount FIELD*/
void
set_symbol_type(symbol_list_entry *l, symbol_type t)
{
	assert(l != NULL);

	switch(t)
	{
	case NT:
		l->rulecount = (is_NT(l))? l->rulecount : rc_values[NT];
		break;
	case LEXICAL:
		l->rulecount = (is_LEXICAL(l))? l->rulecount : rc_values[LEXICAL];
		break;
	case LITERAL:
		l->rulecount = (is_LITERAL(l))? l->rulecount : rc_values[LITERAL];
		break;
	case RANDOM_LEXICAL:
		l->rulecount = rc_values[RANDOM_LEXICAL];
		break;
	case UNDEFINED:
		l->rulecount = rc_values[UNDEFINED];
		break;
	default:
		break;
	}
}

/*USED TO HIDE IMPLEMENTATION OF LEXICON ENTRY BOOK-KEEPING*/
int
get_lexicon_numerosity(symbol_list_entry *l)
{
	int n=0;	

	assert(l != NULL);
	assert(is_LEXICAL(l) == 1);

	n = rc_values[LEXICAL] - l->rulecount;

	assert(n >= 0);
	return n;
}

/*USED TO HIDE IMPLEMENTATION OF LEXICON ENTRY BOOK-KEEPING*/
int
increment_lexicon_numerosity(symbol_list_entry *l)
{
	assert(l != NULL);
	assert(is_LEXICAL(l) == 1);

	return l->rulecount--;
}


/**********************************************************/
/*FUNCTIONS FOR DETERMINING  THE TYPE OF RULE LIST ENTRIES*/
/**********************************************************/

int
is_ONLY_TERMINALS(rule_list_entry *r)
{
	assert(r != NULL);

	return (
		r->type == ALIAS || 
		r->type == EMPTY ||
		r->type == TERMINAL
		);
}

int
is_DERIVES_NONTERMINALS(rule_list_entry *r)
{
	assert(r != NULL);

	return (
		r->type == ALIAS || 
		r->type == EMPTY ||
		r->type == TERMINAL
		);
}


/*****************************/
/*RLE LIST RELATED FUNCTIONS*/
/*****************************/

/*INSERT RLE new_rle IN RULE LIST l (RELATIVE TO AN NT SYMBOL)*/
void
insert_rle(symbol_list_entry *l, rule_list_entry *new_rle)
{
	short int i=0; 
	rule_list_entry **rle_pointer=NULL;
	
	assert(l != NULL);
	assert(new_rle!= NULL);
	assert(is_NT(l) == 1);
		
	if(must_print_message(LISTOPS))
		fprintf(message_stream, "inserting rle in: %d (%s)\n", l->id, l->name);
	
	i = l->rulecount;	
	rle_pointer = &(l->rules);
	
	while(i-- > 0)
	{
		rle_pointer = &((*rle_pointer)->next);
	}
	*rle_pointer = new_rle;	
	l->rulecount++;
}


/*REMOVE RLE IN POSITION pos IN NT SYMBOL l*/
void
remove_rle(symbol_list_entry *l, int pos)
{
	rule_list_entry *r_to_rem = NULL, **pointer_r_to_rem = NULL;
	
	assert(l != NULL);
	assert(pos>0);
	assert(is_NT(l));
	assert(l->rulecount >= 1);

	if(pos != 1)
	{
		pointer_r_to_rem = &(get_rle(l, pos-1)->next);
	}
	else
	{
		pointer_r_to_rem = &(l->rules);
	}

	r_to_rem = get_rle(l, pos);
	
	*pointer_r_to_rem = r_to_rem->next;
	
	l->rulecount--;
	clean_rule(r_to_rem->rule);
	free(r_to_rem);
}


/*FETCH AND RETURN RLE IN POSITION pos FROM RULE LIST OF SYMBOL POINTED BY l*/
/*pos MUST BE GREATER THAN 0*/
rule_list_entry *
get_rle(symbol_list_entry *l, int pos)
{
	rule_list_entry *rle=NULL;

	assert(l != NULL);
	assert(pos <= l->rulecount);
	assert(pos != 0);

	if(is_NT(l) != 1)
		return NULL;

	rle = l->rules;

	while(--pos > 0)
	{
		rle = rle->next;
	}

	return rle;
}

/*LINK RLE l TO RLE new_rle AND RETURN l*/
/*IF l IS NULL new_rle IS RETURNED      */
rule_list_entry *
link_rle(rule_list_entry *l, rule_list_entry *new_rle)
{
	rule_list_entry *rle = NULL;
	
	if(l == NULL)
		return new_rle;
	
	rle=l;
	while(rle->next !=NULL)
	{
		rle=rle->next;
	}
	rle->next = new_rle;

	return l; 
}



/************************/
/*RLE RELATED FUNCTIONS*/
/************************/

/*ALLOCATES, INITIALIZES AND RETURNS A NEW RLE*/
rule_list_entry *
initialize_new_rle()
{
	rule_list_entry *rle = NULL;
	
	rle = xmalloc(sizeof(rule_list_entry));
	memset(rle, 0, sizeof(rule_list_entry));
	
	rle->rule = initialize_new_rule();
	rle->probability = DEFAULT_PROBABILITY_INITIALIZATION;	

	return rle;
}


/*INSERTS (APPENDS) THE NEW SYMBOL sym TO RLE r*/
void
insert_symbol_rle(rule_list_entry *r, symbol_id sym)
{
	assert(r != NULL);
	
	insert_symbol_rule(r->rule, sym, r->length++);
}


/*REMOVES THE SYMBOL IN POSITION POS. RETURNS RESULTANT RULE LENGTH*/
int
remove_symbol_rle(rule_list_entry *r, int pos)
{
	rule_t *new_rule = NULL;
	
	assert(r != NULL);
	assert(r->length > 0);
	assert(pos < r->length);
	assert(pos >= 0);
	
	new_rule = remove_symbol_rule(r->rule, pos, r->length);
	assert(new_rule != NULL);

	clean_rule(r->rule);

	r->rule = new_rule;
	r->length--;	

	return r->length;
}


/*EXTRACTS THE SYMBOL IN POSITION pos FROM RLE r*/
symbol_id
extract_symbol_rle(rule_list_entry *r, int pos)
{
	assert(r != NULL);
	assert(pos >= 0);
	assert(pos < r->length);

	return extract_symbol_rule(r->rule, pos);
}


/*MODIFY VALUE IN POSITION pos OF rle TO new_value*/
void
modify_symbol_rle(rule_list_entry *rle, int pos, symbol_id new_value)
{
	assert(rle != NULL);
	assert(pos >= 0);
	assert(pos < rle->length);
	assert(new_value >= 0);

	modify_symbol_rule(rle->rule, pos, new_value);
}

/************************/
/*RULE RELATED FUNCTIONS*/
/************************/

/*USEFUL FOR HIDING IMPLEMENTATION OF rule*/
void
insert_symbol_rule(rule_t *rule, symbol_id sym, int pos)
{
	int fragment=0, offset=0;

	assert(rule != NULL);

	fragment = pos/(RULE_FRAGMENT_SIZE-1);
	offset = pos%(RULE_FRAGMENT_SIZE-1);
	
	while(fragment-- > 0)
	{
		if(offset == 0 && fragment == 0)
		{
			rule[RULE_FRAGMENT_SIZE-1] = (rule_t) xmalloc(RULE_FRAGMENT_SIZE * sizeof(rule_t));
			rule = (rule_t *) rule[RULE_FRAGMENT_SIZE-1];
			break;
		}
		rule = (rule_t *) rule[RULE_FRAGMENT_SIZE-1];
	}

	rule[offset] = sym;
}


/*USEFUL FOR HIDING IMPLEMENTATION OF rule. RETURNS id OF REMOVED SYMBOL*/
rule_t *
remove_symbol_rule(rule_t *rule, int pos, int length)
{
	int i, j=0;
	rule_t *new_rule = NULL;

	assert(rule != NULL);
	assert(length > 0);
	assert(pos < length);
	assert(pos >= 0);

	new_rule = initialize_new_rule();
	assert(new_rule != NULL);

	for(i=0; i<length; i++)
	{
		if(i == pos)
			continue;
		else
		{
			symbol_id s = (symbol_id) 0;

			s = extract_symbol_rule(rule, i);
			assert(s != (symbol_id)0);
			insert_symbol_rule(new_rule, s, j++);
		}
	}

	return new_rule;
}

/*USEFUL FOR HIDING IMPLEMENTATION OF rule*/
symbol_id
extract_symbol_rule(rule_t *rule, int pos)
{
	int fragment=0, offset=0;

	assert(rule != NULL);

	fragment = pos/(RULE_FRAGMENT_SIZE-1);
	offset = pos%(RULE_FRAGMENT_SIZE-1);

	while(fragment-- > 0)
	{
		rule = (rule_t *) rule[RULE_FRAGMENT_SIZE-1];
	}

	return rule[offset];
}

/*USEFUL FOR HIDING IMPLEMENTATION OF rule*/
void
modify_symbol_rule(rule_t *rule, int pos, symbol_id new_value)
{
	int fragment=0, offset=0;

	assert(rule != NULL);

	fragment = pos/(RULE_FRAGMENT_SIZE-1);
	offset = pos%(RULE_FRAGMENT_SIZE-1);

	while(fragment-- > 0)
	{
		rule = (rule_t *) rule[RULE_FRAGMENT_SIZE-1];
	}

	rule[offset] = new_value;
}

/*ALLOCATES, INITIALIZES AND RETURNS A NEW RULE*/
rule_t *
initialize_new_rule()
{
	rule_t *rule = NULL;

	rule = xmalloc(RULE_FRAGMENT_SIZE * sizeof(rule_t));
	memset(rule, 0, RULE_FRAGMENT_SIZE);

	return rule;
}



/******************************************/
/*LEXICON ARGZ STRUCTURE RELATED FUNCTIONS*/
/******************************************/


/*RETURNS A POINTER TO A NEWLY ALLOCATED LEXICON STRUCTURE*/
lexicon_argz_structure *
initialize_new_lexicon_argz_structure()
{
	lexicon_argz_structure *lazs = NULL;

	lazs = xmalloc(sizeof(lexicon_argz_structure));
	assert(lazs != NULL);
	memset(lazs, 0, sizeof(lexicon_argz_structure));

	return lazs;
}



/********************/
/*CLEANING FUNCTIONS*/
/********************/


/*CLEANS UP SYMBOL TABLE AND LEXICON STRUCTURES (FOR TERMINAL SYMBOLS)*/
void
clean_symbol_list(symbol_list_entry *l)
{
	assert(l != NULL);
	
	if(l->next != NULL)
		clean_symbol_list(l->next);

	if(l->rules != NULL && l->id != 0 && is_NT(l))
		clean_rle_list(l->rules);
	else if(l->rules != NULL && l->id != 0 && is_LEXICAL(l))
		free(((lexicon_argz_structure *)l->rules)->argz);

	if(must_print_message(CLEAN_MAX))
		fprintf(message_stream, "freeing symbol_list_entry ADDRESS: %p, ID: %d, NAME: %s\n", l, l->id, l->name);

	free(l->name);
	free(l);	
}


/*CLEANS UP LINKED LIST OF RULES (CALLED FOR EVERY NON-TERMINAL SYMBOL)*/
void
clean_rle_list(rule_list_entry *l)
{
	assert(l != NULL);
	
	if (l->next != NULL)
		clean_rle_list(l->next);
	if(must_print_message(CLEAN_MAX))
		fprintf(message_stream, "freeing rule_list_entry at ADDRESS: %p\n", l);

	if(l->rule != 0)
	{
		clean_rule(l->rule);
	}
	free(l);	
}


/*CLEAN UP A SINGLE RULE STRUCTURE*/
void clean_rule(rule_t *r)
{
	assert(r != NULL);
	
	if (r[RULE_FRAGMENT_SIZE-1] != 0)
		clean_rule((rule_t *) r[RULE_FRAGMENT_SIZE-1]);
	free(r);
}

