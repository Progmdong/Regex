/*************************************************************************
	> File Name: nfa.c
	> Author: Dandi
	> Mail: progmdong@gmail.com
	> Created Time: Wed 12 Nov 2014 07:24:22 PM PST
 ************************************************************************/

#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include"nfa.h"

#define EPSILON 256
#define DUMMY 257


State state_pool[1000];
int water_level = 0;
#define alloc_state() (water_level<1000)?state_pool+(water_level++):NULL



void  add_transition( State *from, int symbol, State *to )
{
    if( from==NULL || to==NULL || symbol>=DUMMY )
        return;
    Tolist *old_first = from->to_states;
    Tolist *new_first = (Tolist*)malloc( sizeof(struct Tolist_tag ));
    from->to_states = new_first;
    new_first->state = to;
    new_first->symbol = symbol;
    new_first->next = old_first;

}

/*
 * Convert infix regexp re to postfix notation.
 * Insert . as explicit concatenation operator.
 * Cheesy parser, return static buffer.
 */
char*
re2post(char *re)
{
	int nalt, natom;
	static char buf[8000];
	char *dst;
	struct {
		int nalt;
		int natom;
	} paren[100], *p;
	
	p = paren;
	dst = buf;
	nalt = 0;
	natom = 0;
	if(strlen(re) >= sizeof buf/2)
		return NULL;
	for(; *re; re++){
		switch(*re){
		case '(':
			if(natom > 1){
				--natom;
				*dst++ = '.';
			}
			if(p >= paren+100)
				return NULL;
			p->nalt = nalt;
			p->natom = natom;
			p++;
			nalt = 0;
			natom = 0;
			break;
		case '|':
			if(natom == 0)
				return NULL;
			while(--natom > 0)
				*dst++ = '.';
			nalt++;
			break;
		case ')':
			if(p == paren)
				return NULL;
			if(natom == 0)
				return NULL;
			while(--natom > 0)
				*dst++ = '.';
			for(; nalt > 0; nalt--)
				*dst++ = '|';
			--p;
			nalt = p->nalt;
			natom = p->natom;
			natom++;
			break;
		case '*':
		case '+':
		case '?':
			if(natom == 0)
				return NULL;
			*dst++ = *re;
			break;
		default:
			if(natom > 1){
				--natom;
				*dst++ = '.';
			}
			*dst++ = *re;
			natom++;
			break;
		}
	}
	if(p != paren)
		return NULL;
	while(--natom > 0)
		*dst++ = '.';
	for(; nalt > 0; nalt--)
		*dst++ = '|';
	*dst = 0;
	return buf;
}

typedef struct Expr
{
    State *head_state;
    State *tail_state;
}Expr;

Expr create_expr()
{
    Expr e;
    e.head_state = alloc_state();
    e.tail_state = alloc_state();
    return e;
}


void postfix2nfa(char *postfix)
{
    Expr stack[1000], *top;
    #define push(s) *top++ = s
    #define pop()  *--top
    top = stack; //init stack
    char *ch = postfix;
    for(; *ch; ch++)
    {
       switch(*ch)
        {
            default://normal char
            {
                Expr e = create_expr();
                add_transition( e.head_state, *ch, e.tail_state  );
                push(e);
                break;
            }
            case '|':
            {
                Expr e2 = pop();
                Expr e1 = pop();
                Expr e = create_expr();
                add_transition( e.head_state, EPSILON, e1.head_state  );
                add_transition( e.head_state, EPSILON, e2.head_state  );
                add_transition( e1.tail_state, EPSILON, e.tail_state  );
                add_transition( e2.tail_state, EPSILON, e.tail_state  );
                push(e);
                break;
            }
            case '.':
            {
                Expr e2 = pop();
                Expr e1 = pop();
                add_transition( e1.tail_state, EPSILON, e2.head_state  );
                Expr e = { e1.head_state, e2.tail_state  };
                push( e );
                break;
            }
            case '*':
            {
                Expr e1 = pop();
                Expr e = create_expr();
                add_transition( e.head_state, EPSILON, e1.head_state );
                add_transition( e.head_state, EPSILON, e.tail_state );
                add_transition( e1.tail_state, EPSILON, e.tail_state  );
                add_transition( e1.tail_state, EPSILON, e1.head_state );
                push(e);
                break;
            }
            case '+':
            {
                Expr e1 = pop();
                Expr e = create_expr();           
                add_transition( e.head_state, EPSILON, e1.head_state );
                add_transition( e1.tail_state, EPSILON, e.tail_state  );
                add_transition( e1.tail_state, EPSILON, e1.head_state );
                push(e);
                break;
            }
        }  
    }
    
    Expr e = pop();

    #undef push
    #undef pop
    NFA.start = e.head_state;
    NFA.end = e.tail_state;
    return ;
}

void traverse_nfa()
{
    int i=0;
    for(;i<water_level;++i)
    {
        State *s = &state_pool[i];
        Tolist *p = s->to_states;
        while(p!=NULL)
        {
            printf("state%d-----%c---->state%d\n",i,p->symbol, state_pool-p->state);
            p = p->next;
        }
    }
}

#undef alloc_state
#undef alloc_tomap
