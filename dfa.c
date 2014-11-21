/*************************************************************************
	> File Name: dfa.c
	> Author: Dandi
	> Mail: progmdong@gmail.com
	> Created Time: Thu 13 Nov 2014 07:08:10 PM PST
 ************************************************************************/

#include<stdio.h>
#include"dfa.h"

typedef struct{
    State **states;
    int size;
}StateSet;

struct DState_tag
{
    StateSet *state_set;
    struct DState_tag *to[256];
    struct DState_tag *next;
    struct DState_tag *left;
    struct DState_tag *right;
    int is_marked;
};

static DState dstate_pool[1000];
static int water_level=0;
#define alloc_dstate() (water_level<1000)?dstate_pool+(water_level++):NULL;

static DState *dstate_tree;

static int ptr_cmp(const void* a, const void* b)
{
    State* lhs = *(State**)a;
    State* rhs = *(State**)b;
    if(lhs<rhs)return -1;
    if(lhs>rhs)return 1;
    return 0;
}
 
StateSet *move(StateSet *from, int c)
{
    int i=0;
    static State *buff[1000];
    int k=0;

    for(;i<from->size;++i)
    {
        Tolist *p = (from->states[i])->to_states;
        while(p!=NULL)
        {
            if(p->symbol==c)
                buff[k++] = p->state;
            p = p->next;
        }
    }
    if(k==0)return NULL;
    StateSet* set = (StateSet*)malloc( sizeof(StateSet) );
    set->states = (State**)malloc( k*sizeof(State*) );
    memcpy( set->states, buff, k*sizeof(State*) );
    qsort(set->states, k, sizeof(State*), ptr_cmp);
    set->size = k;

}    
StateSet *epsilon_closure(StateSet *s)
{
    static int already_on_tag=0;
    already_on_tag++;
    static State *buff[1000];
    int i=0;
    for(;i<s->size;i++)
    {
        s->states[i]->already_on = already_on_tag;
        buff[i] = s->states[i];
    }
    int n=s->size;
    int n2 = epsilon_closure_helper(buff,0, n, already_on_tag);
    while(n2!=n)
    {
        int tmp = n2;
        n2 = epsilon_closure_helper(buff,n,n2,already_on_tag);
        n = tmp;
    }
    if(n==s->size)
        return s;
    StateSet* set = (StateSet*)malloc( sizeof(StateSet) );
    set->states = (State**)malloc( n*sizeof(State*) );
    memcpy(set->states, buff, n*sizeof(State*));
    qsort(set->states, n, sizeof(State*), ptr_cmp);
    set->size = n;
    free(s->states);
    free(s);
    return set;

}

int epsilon_closure_helper( State **ss,int start, int n, int tag )
{
    int i=start;
    int k=n;
    for(;i<n;i++)
    {
        Tolist *p = ss[i]->to_states;
        while(p!=NULL)
        {
            if(p->symbol==EPSILON)
            {
                if(p->state->already_on!=tag)
                {
                    ss[k++]=p->state;
                    p->state->already_on = tag;
                }    
            }
            p = p->next;
        }
    }
    return k;
}

int set_cmp(StateSet *lhs, StateSet *rhs)
{
    int i=0;

    for(;;i++)
    {
        if(i>=lhs->size && i>=rhs->size)return 0;
        if(i>=lhs->size || lhs->states[i]<rhs->states[i])return -1;
        if(i>=rhs->size || lhs->states[i]>rhs->states[i])return 1;
    }
    return 0;
}

DState *search_or_insert(DState *root, StateSet *set)
{
    if(root==NULL)
    {
        DState *new_ds = alloc_dstate();
        new_ds->state_set = set;
        new_ds->left = new_ds->right = NULL;
        new_ds->is_marked = 0;
        return new_ds;
    }
    int cmp = set_cmp(root->state_set, set);
    if(cmp>0)
    {
        DState *ret = search_or_insert(root->right,set);
        if(root->right==NULL)
        {
            root->right = ret;
        }
        return ret;
    }
    
    else if(cmp<0)
    {
        DState *ret = search_or_insert(root->left,set);
        if(root->left==NULL)
        {
            root->left = ret;
        }
        return ret;
    }
    else
    {
        root->is_marked = 1;
        return root;
    }   
}



static DState visit_list;

void nfa2dfa()
{   
    StateSet *tmp = (StateSet*)malloc(sizeof(StateSet));
    tmp->size = 1;
    tmp->states = (State**)malloc(sizeof(State*));
    tmp->states[0] = NFA.start;
    StateSet *start_set = epsilon_closure(tmp);
    StateSet *s = start_set;
    dstate_tree = search_or_insert(dstate_tree, s);
    visit_list.next = dstate_tree;
    while(1)
    {
        if(visit_list.next==NULL)
            break;
        DState *ds = visit_list.next;
        visit_list.next = visit_list.next->next;
        s = ds->state_set;
        int i=0;
        for(;i<256;i++)
        {
            StateSet *ss = move(s,i);
            if(ss!=NULL)
            {
                ss = epsilon_closure(ss);
                DState *ds2 = search_or_insert(dstate_tree,ss);
                ds->to[i] = ds2;
                if(ds2->is_marked==0)
                {
                    ds2->next = visit_list.next;
                    visit_list.next = ds2;
                }
            }
        }
    }
}

void traverse_dfa()
{
    int i=0;
    for(;i<water_level;i++)
    {
        int j=0;
        for(;j<256;j++)
        {
            if(dstate_pool[i].to[j]!=NULL)
            printf("dstate%d----%c---->dstate%d\n",i,j,dstate_pool[i].to[j]-dstate_pool);
        }
    }
}

#undef alloc_dstate
