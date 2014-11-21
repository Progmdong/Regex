/*************************************************************************
	> File Name: nfa.c
	> Author: Dandi
	> Mail: progmdong@gmail.com
	> Created Time: Wed 12 Nov 2014 07:24:22 PM PST
 ************************************************************************/
#ifndef _NFA_H
#define _NFA_H
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>

#define EPSILON 256
#define DUMMY 257

typedef struct Tolist_tag Tolist;
typedef struct State_tag State;

struct Tolist_tag
{
    State *state;
    struct Tolist_tag *next;
    int symbol;
};

struct State_tag
{
    struct Tolist_tag *to_states;
    int already_on;
};


struct{
    State* start;
    State* end;
}NFA;

char* re2post(char *re);
void postfix2nfa(char *postfix);

void traverse_nfa();

#endif
