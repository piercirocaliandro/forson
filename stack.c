/*
stack.c -- implementation of a dinamically growing, pushdown stack
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

/*ALLOCATES MEMORY FOR A STACK STRUCTURE, WIPES IT TO ZERO AND RETURNS A POINTER*/
stack *
initialize_new_stack()
{
	stack *new_stack = NULL;

	new_stack = xmalloc(sizeof(stack));
	memset(new_stack, 0, sizeof(stack));
	new_stack->buffer = (symbol_id *) xmalloc(STACK_FRAGMENT_SIZE * sizeof(symbol_id));
	memset(new_stack->buffer, 0, STACK_FRAGMENT_SIZE * sizeof(symbol_id));

	return new_stack;
}


/*POP THE SYMBOL ON TOP, RETURNS ZERO IF STACK IS EMPTY*/
symbol_id
pop(stack *st)
{
	int fragment, offset;
	symbol_id *buf = NULL;

	assert(st != NULL);

	if(st->size == 0)
	{
		return 0;
	}

	st->size--;

	fragment = st->size/(STACK_FRAGMENT_SIZE-1);
	offset = st->size%(STACK_FRAGMENT_SIZE-1);

	buf = st->buffer;

	while(fragment-- > 0)
	{
		assert(buf != NULL);
		buf = (symbol_id *) buf[STACK_FRAGMENT_SIZE-1];
	}

	return buf[offset];
}


/*PUSH A NEW SYMBOL ON THE STACK. symb CAN'T BE ZERO*/
int
push(stack *st, symbol_id symb)
{
	int fragment, offset;
	symbol_id *buf = NULL;

	assert(st != NULL);
	assert(symb != (symbol_id) 0);

	fragment = st->size/(STACK_FRAGMENT_SIZE-1);
	offset = st->size%(STACK_FRAGMENT_SIZE-1);


	buf = st->buffer;
	while(fragment-- > 0)
	{

		buf = (symbol_id *) buf[STACK_FRAGMENT_SIZE-1];
		assert(buf != NULL);
	}

	if(buf[STACK_FRAGMENT_SIZE-1]==0)
	{
		//printf("xmalloc begin\n");
		buf[STACK_FRAGMENT_SIZE-1] = (symbol_id) xmalloc(STACK_FRAGMENT_SIZE * sizeof(symbol_id));
		//printf("xmalloc end\n");
		memset((symbol_id *)buf[STACK_FRAGMENT_SIZE-1], 0, STACK_FRAGMENT_SIZE * sizeof(symbol_id));
		//printf("memset end\n");
		// buf[STACK_FRAGMENT_SIZE-1] = (symbol_id) calloc(STACK_FRAGMENT_SIZE, sizeof(symbol_id));
		// if(buf[STACK_FRAGMENT_SIZE-1] == 0)
		// {
		// 	error(UNEXPECTED_ERROR, 0, "%s", "unexpected error: out of memory");
		// }
	}
	buf[offset] = symb;
	st->size++;	

	return st->size;
}


/*RETURNS NUMBER OF SYMBOLS CURRENTLY ON THE STACK*/
int get_size(stack *st)
{
	assert(st != NULL);

	return st->size;
}


/*FREES MEMORY USED BY A STACK STRUCTURE*/
int clean_stack(stack *st)
{
	assert(st != NULL);

	if(st->buffer != NULL)
	{
		clean_buffer(st->buffer);
	}
	free(st);
}


/*FREES MEMORY USED BUFFERS OF THE STACK STRUCTURE, RECURSIVELY*/
int clean_buffer(symbol_id *buf)
{
	assert(buf != NULL);

	if((symbol_id *)buf[STACK_FRAGMENT_SIZE-1] != NULL)
	{
		clean_buffer((symbol_id *)buf[STACK_FRAGMENT_SIZE-1]);
	}

	free(buf);
}

