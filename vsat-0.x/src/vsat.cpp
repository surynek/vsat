/* Pavel Surynek, 2009 */
/* pavel.surynek@mff.cuni.cz */
/* vsat.cpp */

/*
  A SAT solver called vsat.
 */


#include "vsat.h"


int STAT_DECISIONS = 0;
int STAT_PROPAGATIONS = 0;
int STAT_BACKTRACKS = 0;


void insert_watch(WNode *head, WNode *insert)
{
	WNode *next = head->m_next;

	next->m_prev = insert;
	head->m_next = insert;

	insert->m_prev = head;
	insert->m_next = next;
}


void remove_watch(WNode *UNUSED(head), WNode *remove)
{
	WNode *next = remove->m_next;
	WNode *prev = remove->m_prev;

	prev->m_next = next;
	next->m_prev = prev;
}


void skip_comment_DIMACS(FILE *fr)
{
	while (fgetc(fr) != '\n');
}


void swap_affects(Clause *clause_1, Clause *clause_2)
{
	Clause *temp, **tmp;

	temp = *clause_1->m_sort;
	*clause_1->m_sort = *clause_2->m_sort;
	*clause_2->m_sort = temp;

	tmp = clause_1->m_sort;
	clause_1->m_sort = clause_2->m_sort;
	clause_2->m_sort = tmp;
}
