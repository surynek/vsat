/* Pavel Surynek, 2009 */
/* pavel.surynek@mff.cuni.cz */
/* vsat.h */

/*
  A SAT solver called vsat.
 */

#ifndef __VSAT_H__
#define __VSAT_H__

#include <assert.h>
#include <map>

#define UNUSED(x)

#define STAT	1

#ifdef STAT
  extern int STAT_DECISIONS;
  extern int STAT_PROPAGATIONS;
  extern int STAT_BACKTRACKS;
#endif

class Literal;
class Watch;
class Clause;


class Variable
{
public:
	Literal *m_positive;
	Literal *m_negative;
};


class WNode
{
public:
	Watch *m_watch;
	WNode *m_next;
	WNode *m_prev;
};


class ONode
{
public:
	Clause *m_clause;
	ONode *m_next;	
};


enum Value
{
	V_FALSE      =  0,
	V_UNIT_FALSE =  1,
	V_TRUE       =  2,
	V_UNIT_TRUE  =  4,
	V_UNIT       =  8,
	V_UNDEF      = 16,
};


class Literal
{
public:
	Value m_value;
	Literal *m_base;
	Literal *m_cmpl;
	Variable *m_variable;
	WNode m_watch_1_list;
	WNode m_watch_2_list;

// Path specific members
	int m_cO;
	ONode *m_occs;
};


class Watch
{
public:
	Clause *m_clause;
	Literal **m_literal;
};


class Clause
{
public:
	Literal **m_literals;
	Watch m_watch_1;
	Watch m_watch_2;

// Path specific members
	Clause **m_sort;
	int m_affects;
};


void insert_watch(WNode *watch_list, WNode *watch_insert);
void remove_watch(WNode *watch_list, WNode *watch_remove);
void skip_comment_DIMACS(FILE *fr);

void swap_affects(Clause *clause_1, Clause *clause_2);


class Formula
{
public:
	void init(int n_V, int n_C, int n_O)
	{
		m_V = n_V;
		m_C = n_C;
		m_O = n_O;

		m_clauses = (Clause*)malloc(n_C * sizeof(Clause));
		assert(m_clauses != NULL);

		m_affects = (Clause**)malloc((n_C + 1)* sizeof(Clause*));
		assert(m_affects != NULL);

		m_P_literals = (Literal*)malloc((n_V + 1) * sizeof(Literal));
		assert(m_P_literals != NULL);

		m_N_literals = (Literal*)malloc((n_V + 1) * sizeof(Literal));
		assert(m_N_literals != NULL);

		m_variables = (Variable*)malloc((n_V + 1) * sizeof(Variable));
		assert(m_variables != NULL);

		m_occurences = (Literal**)malloc(n_O * sizeof(Literal*));
		assert(m_occurences != NULL);

		m_L_occurences = (ONode*)malloc(n_O * sizeof(ONode));
		assert(m_L_occurences != NULL);

		m_watch_nodes = (WNode*)malloc(2 * n_C * sizeof(WNode));
		assert(m_watch_nodes != NULL);

		m_unit_literals = (Literal**)malloc(2 * n_V * sizeof(Literal*));
		assert(m_unit_literals != NULL);

		m_last_unit = m_unit_literals;

		m_chng_literals = (Literal**)malloc(2 * n_V * sizeof(Literal*));
		assert(m_chng_literals != NULL);

		m_last_chng = m_chng_literals;

		m_cO = m_cC = m_cW = m_cA = 0;

		for (int i = 0; i <= n_V; ++i)
		{
			Variable &variable = m_variables[i];
			Literal &p_literal = m_P_literals[i];
			Literal &n_literal = m_N_literals[i];

			p_literal.m_value = V_UNDEF;
			p_literal.m_variable = &variable;
			p_literal.m_base = m_P_literals;

			p_literal.m_watch_1_list.m_next = &p_literal.m_watch_1_list;
			p_literal.m_watch_1_list.m_prev = &p_literal.m_watch_1_list;
			p_literal.m_watch_1_list.m_watch = NULL;

			p_literal.m_watch_2_list.m_next = &p_literal.m_watch_2_list;
			p_literal.m_watch_2_list.m_prev = &p_literal.m_watch_2_list;
			p_literal.m_watch_2_list.m_watch = NULL;
			p_literal.m_cO = 0;
			p_literal.m_occs = NULL;

			p_literal.m_cmpl = &n_literal;

			n_literal.m_value = V_UNDEF;
			n_literal.m_variable = &variable;
			n_literal.m_base = m_N_literals;

			n_literal.m_watch_1_list.m_next = &n_literal.m_watch_1_list;
			n_literal.m_watch_1_list.m_prev = &n_literal.m_watch_1_list;
			n_literal.m_watch_1_list.m_watch = NULL;

			n_literal.m_watch_2_list.m_next = &n_literal.m_watch_2_list;
			n_literal.m_watch_2_list.m_prev = &n_literal.m_watch_2_list;
			n_literal.m_watch_2_list.m_watch = NULL;
			n_literal.m_cO = 0;
			n_literal.m_occs = NULL;

			n_literal.m_cmpl = &p_literal;

			variable.m_positive = &p_literal;
			variable.m_negative = &n_literal;
		}

		m_head_clause = m_clauses;
		m_head_var = m_variables + 1;
	}


	void add_2_clause(int l1, int l2)
	{
		int lits[] = {l1, l2, 0};

		add_general_clause(lits);
	}


	void add_3_clause(int l1, int l2, int l3)
	{
		int lits[] = {l1, l2, l3, 0};

		add_general_clause(lits);
	}


	void add_general_clause(int *lits)
	{
		Clause &clause = m_clauses[m_cC];
		clause.m_literals = &m_occurences[m_cO];

		clause.m_sort = NULL;
		clause.m_affects = 0;

		int i = 0;
		int *l = lits;

		while (*l != 0)
		{
			++m_cO;

			Literal *&literal = clause.m_literals[i];

			if (*l < 0)
			{
				literal = &m_N_literals[-*l];
			}
			else
			{
				literal = &m_P_literals[*l];
			}
			++literal->m_cO;

			ONode &onode = m_L_occurences[m_cLO];
			onode.m_next = literal->m_occs;
			onode.m_clause = &clause;
			++m_cLO;

			literal->m_occs = &onode;

			++l;
			++i;
		}

		assert(i >= 1);

		if (i == 1)
		{
			if (clause.m_literals[0]->m_value != V_UNIT_FALSE)
			{
				clause.m_literals[0]->m_value = V_UNIT_TRUE;
				clause.m_literals[0]->m_cmpl->m_value = V_UNIT_FALSE;

				*m_last_unit++ = clause.m_literals[0];
				*m_last_chng++ = clause.m_literals[0];
			}
			else
			{
				m_conflicting = true;
			}
		}

		clause.m_watch_1.m_clause = &clause;
		clause.m_watch_1.m_literal = &clause.m_literals[0];

		m_watch_nodes[m_cW].m_watch = &clause.m_watch_1;
		insert_watch(&(*clause.m_watch_1.m_literal)->m_watch_1_list, &m_watch_nodes[m_cW]);
		++m_cW;

		clause.m_watch_2.m_clause = &clause;
		clause.m_watch_2.m_literal = &clause.m_literals[i - 1];

		m_watch_nodes[m_cW].m_watch = &clause.m_watch_2;
		insert_watch(&(*clause.m_watch_2.m_literal)->m_watch_2_list, &m_watch_nodes[m_cW]);
		++m_cW;

		m_occurences[m_cO] = NULL;
		++m_cO;
		++m_cC;
	}


	void set_TRUE(Literal *literal)
	{
		literal->m_value = V_TRUE;
		literal->m_cmpl->m_value = V_FALSE;
	}


	void set_FALSE(Literal *literal)
	{
		literal->m_value = V_FALSE;
		literal->m_cmpl->m_value = V_TRUE;
	}


	void unset(Literal *literal)
	{
		literal->m_value = V_UNDEF;
		literal->m_cmpl->m_value = V_UNDEF;
	}


	bool set_TRUE_propagate(Literal *literal)
	{
		*m_last_unit++ = literal;
		*m_last_chng++ = literal;

		return propagate_unit();
	}


	bool set_FALSE_propagate(Literal *literal)
	{
		*m_last_unit++ = literal->m_cmpl;
		*m_last_chng++ = literal->m_cmpl;

		return propagate_unit();
	}


	Literal** get_check_point(void)
	{
		return m_last_chng;
	}

	
	void unset_unpropagate(void)
	{
		unset_unpropagate(m_chng_literals);
	}


	void unset_unpropagate(Literal **check_point)
	{
		while (m_last_chng > check_point)
		{
			unset(*(--m_last_chng));
		}
		m_last_unit = m_unit_literals;
	}


	bool propagate_unit(void)
	{
		while (m_last_unit > m_unit_literals)
		{
#ifdef STAT
			++STAT_PROPAGATIONS;
#endif
			Literal *true_literal = *--m_last_unit;
			Literal *false_literal = true_literal->m_cmpl;

			set_TRUE(true_literal);

			WNode *wnode_1 = false_literal->m_watch_1_list.m_next;

			while (wnode_1->m_watch != NULL)
			{
				Literal **literal_0 = wnode_1->m_watch->m_literal;
				Literal **literal_1 = literal_0;
				Literal **literal_2 = wnode_1->m_watch->m_clause->m_watch_2.m_literal;

				WNode *node = wnode_1;
				wnode_1 = wnode_1->m_next;
				
				while (true)
				{
					if (*++literal_1 == NULL)
					{
						literal_1 = node->m_watch->m_clause->m_literals;
					}

					if ((*literal_1)->m_value == V_TRUE || (*literal_1)->m_value == V_UNIT_TRUE)
					{
						break;
					}
					else
					{
						if (literal_1 != literal_2)
						{
							if ((*literal_1)->m_value == V_UNDEF)
							{
								node->m_watch->m_clause->m_watch_1.m_literal = literal_1;
								
								remove_watch(&(*literal_0)->m_watch_1_list, node);
								insert_watch(&(*literal_1)->m_watch_1_list, node);
								break;
							}
						}		
						if (literal_1 == literal_0)
						{
							if ((*literal_2)->m_value != V_UNIT_TRUE)
							{
								if ((*literal_2)->m_value != V_UNDEF)
								{
									return false;
								}

								(*literal_2)->m_value = V_UNIT_TRUE;
								(*literal_2)->m_cmpl->m_value = V_UNIT_FALSE;
								
								*m_last_unit++ = *literal_2;
								*m_last_chng++ = *literal_2;
							}
							break;
						}
					}
				}
			}
			
			WNode *wnode_2 = false_literal->m_watch_2_list.m_next;
			
			while (wnode_2->m_watch != NULL)
			{
				Literal **literal_0 = wnode_2->m_watch->m_literal;
				Literal **literal_2 = literal_0;
				Literal **literal_1 = wnode_2->m_watch->m_clause->m_watch_1.m_literal;

				WNode *node = wnode_2;
				wnode_2 = wnode_2->m_next;
					
				while (true)
				{
					if (*++literal_2 == NULL)
					{
						literal_2 = node->m_watch->m_clause->m_literals;
					}
					if ((*literal_2)->m_value == V_TRUE || (*literal_2)->m_value == V_UNIT_TRUE)
					{
						break;
					}
					else
					{
						if (literal_2 != literal_1)
						{
							if ((*literal_2)->m_value == V_UNDEF)
							{
								node->m_watch->m_clause->m_watch_2.m_literal = literal_2;
								remove_watch(&(*literal_0)->m_watch_2_list, node);
								insert_watch(&(*literal_2)->m_watch_2_list, node);
								break;
							}
						}

						if (literal_2 == literal_0)
						{
							if ((*literal_1)->m_value != V_UNIT_TRUE)
							{
								if ((*literal_1)->m_value != V_UNDEF)
								{
									return false;
								}
								
								(*literal_1)->m_value = V_UNIT_TRUE;
								(*literal_1)->m_cmpl->m_value = V_UNIT_FALSE;
								
								*m_last_unit++ = *literal_1;
								*m_last_chng++ = *literal_1;
							}
							break;
						}
					}
				}
			}
		}

		return true;
	}


	bool solve_1(void)
	{
		if (m_conflicting)
		{
			return false;
		}
		else
		{
			if (!propagate_unit())
			{
				return false;
			}

			return solve_1_rec();
		}
	}


	bool solve_1_rec(void)
	{
		Clause *head_clause_save = m_head_clause;
		Variable *head_var_save = m_head_var;

		while (calc_value(*m_head_clause) == V_TRUE)
		{
			++m_head_clause;

			if (m_head_clause - m_clauses >= m_cC)
			{
				return true;
			}
		}
		while (m_head_var->m_positive->m_value != V_UNDEF)
		{
			++m_head_var;
		}

		assert(m_head_var - m_variables < m_V);

		Literal **check_point = get_check_point();
#ifdef STAT
		++STAT_DECISIONS;
#endif
		if (set_TRUE_propagate(m_head_var->m_positive))
		{
			if (solve_1_rec())
			{
				return true;
			}
			else
			{
				unset_unpropagate(check_point);
			}
		}
		else
		{
			unset_unpropagate(check_point);
		}

#ifdef STAT
		++STAT_DECISIONS;
#endif
		if (set_FALSE_propagate(m_head_var->m_positive))
		{
			if (solve_1_rec())
			{
				return true;
			}
			else
			{
				unset_unpropagate(check_point);
			}
		}
		else
		{
			unset_unpropagate(check_point);
		}

		m_head_clause = head_clause_save;
		m_head_var = head_var_save;

#ifdef STAT
		++STAT_BACKTRACKS;
#endif
		return false;
	}


	void find_conflicting_path(Clause *origin, Clause **path, int length)
	{
		path[0] = origin;
		origin->m_affects = -1;

		for (int l = 1; l < length; ++l)
		{
			Literal **literal = origin->m_literals;

			while (*literal != NULL)
			{
				set_TRUE_propagate(*literal);

				Literal **chng_literal = m_chng_literals;

				while (chng_literal < m_last_chng)
				{
					ONode *onode = (*chng_literal)->m_cmpl->m_occs;

					while (onode != NULL)
					{
						if (onode->m_clause->m_affects >= 0)
						{
							increase_affect(onode->m_clause, 1);
						}
						onode = onode->m_next;
					}
					++chng_literal;
				}
				unset_unpropagate();
				++literal;
			}
			print_affects();

			origin = path[l] = m_affects[1];
			reset_affects();

			m_affects[1]->m_affects = -1;
		}
	}


	void increase_affect(Clause *clause, int increase)
	{
		int aff, prev_aff;

		if (clause->m_sort == NULL)
		{
			clause->m_sort = &m_affects[++m_cA];
			m_affects[m_cA] = clause;
			clause->m_affects = increase;

			aff = m_cA;
			prev_aff = aff / 2;
		}
		else
		{
			clause->m_affects += increase;

			aff = clause->m_sort - m_affects;
			prev_aff = aff / 2;
		}
		
		while (prev_aff >= 1)
		{
			if (m_affects[aff]->m_affects > m_affects[prev_aff]->m_affects)
			{
				swap_affects(m_affects[aff], m_affects[prev_aff]);
				aff = prev_aff;
				prev_aff = aff / 2;
			}
			else
			{
				break;
			}
		}
	}


	void reset_affects(void)
	{
		for (int i = 1; i <= m_cA; ++i)
		{
			Clause *clause = m_affects[i];

			clause->m_sort = NULL;
			clause->m_affects = 0;
		}
		m_cA = 0;
	}


	void reset_path(Clause **path, int length) const
	{
		for (int i = 0; i < length; ++i)
		{
			path[i]->m_affects = 0;
		}
	}


	static Value calc_value(const Clause &clause)
	{
		Literal **literal = clause.m_literals;

		int undefs = 0;

		while (*literal != NULL)
		{
			switch ((*literal)->m_value)
			{
			case V_FALSE:
			{
				break;
			}
			case V_TRUE:
			{
				return V_TRUE;
				break;
			}
			case V_UNDEF:
			{
				++undefs;
				break;
			}
			default:
			{
				break;
			}
			}
			++literal;
		}

		switch (undefs)
		{
		case 0:
		{
			return V_FALSE;
			break;
		}
		case 1:
		{
			return V_UNIT;
			break;
		}
		default:
		{
			return V_UNDEF;
			break;
		}
		}
	}


	void print_affects(void) const
	{
		printf("Affects: ");

		for (int i = 1; i <= m_cA; ++i)
		{
			printf("%d ", m_affects[i]->m_affects);
		}
		printf("\n");
	}


	void print_path(Clause **path, int length) const
	{
		printf("Path: ");

		for (int i = 0; i < length; ++i)
		{
			printf("%d ", path[i] - m_clauses);
		}
		printf("\n");
	}


	void print_literals(Literal **literals) const
	{
		Literal **literal = literals;

		while (*literal != NULL)
		{
			if ((*literal)->m_base == m_P_literals)
			{
				printf("%d ", *literal - (*literal)->m_base);
			}
			else
			{
				printf("-%d ", *literal - (*literal)->m_base);
			}
			++literal;
		}

		printf("\n");
	}


	void print_units(void) const
	{
		printf("Unit literals:");

		Literal **literal = m_unit_literals;

		while (literal < m_last_unit)
		{
			if ((*literal)->m_base == m_P_literals)
			{
				printf("%d ", *literal - (*literal)->m_base);
			}
			else
			{
				printf("-%d ", *literal - (*literal)->m_base);
			}
			++literal;
		}

		printf("\n");
	}


	void print(void) const
	{
		printf("p cnf %d %d\n", m_V, m_cC);

		for (int i = 0; i < m_cC; ++i)
		{
			const Clause &clause = m_clauses[i];
			Literal **literal = clause.m_literals;

			printf("%d: ", i);

			while (*literal != NULL)
			{
				if ((*literal)->m_base == m_P_literals)
				{
					if (literal == clause.m_watch_1.m_literal)
					{
						printf("%d(w1) ", *literal - (*literal)->m_base);
					}
					else
					{
						if (literal == clause.m_watch_2.m_literal)
						{
							printf("%d(w2) ", *literal - (*literal)->m_base);
						}
						else
						{
							printf("%d ", *literal - (*literal)->m_base);
						}
					}
				}
				else
				{
					if (literal == clause.m_watch_1.m_literal)
					{
						printf("-%d(w1) ", *literal - (*literal)->m_base);
					}
					else
					{
						if (literal == clause.m_watch_2.m_literal)
						{
							printf("-%d(w2) ", *literal - (*literal)->m_base);
						}
						else
						{
							printf("-%d ", *literal - (*literal)->m_base);
						}
					}
				}
				++literal;
			}
			printf("0\n");
		}

		printf("literals\n");

		for (int i = 1; i <= m_V; ++i)
		{
			WNode *wnode_1 = m_P_literals[i].m_watch_1_list.m_next;
			WNode *wnode_2 = m_P_literals[i].m_watch_2_list.m_next;

			if (wnode_1->m_watch != NULL || wnode_2->m_watch != NULL)
			{
				printf("%d [", i);

				while (wnode_1->m_watch != NULL)
				{
					printf("%d", wnode_1->m_watch->m_clause - m_clauses);
					wnode_1 = wnode_1->m_next;

					if (wnode_1 != NULL)
					{
						printf(",");
					}
				}
				printf("]");

				printf("{");

				while (wnode_2->m_watch != NULL)
				{
					printf("%d", wnode_2->m_watch->m_clause - m_clauses);
					wnode_2 = wnode_2->m_next;

					if (wnode_2 != NULL)
					{
						printf(",");
					}
				}
				printf("} ");
			}
			else
			{
				printf(" %d ", i);
			}
		}
		printf("\n");

		for (int i = 1; i <= m_V; ++i)
		{
			WNode *wnode_1 = m_N_literals[i].m_watch_1_list.m_next;
			WNode *wnode_2 = m_N_literals[i].m_watch_2_list.m_next;

			if (wnode_1->m_watch != NULL || wnode_2->m_watch != NULL)
			{
				printf("-%d [", i);

				while (wnode_1->m_watch != NULL)
				{
					printf("%d", wnode_1->m_watch->m_clause - m_clauses);
					wnode_1 = wnode_1->m_next;

					if (wnode_1->m_watch != NULL)
					{
						printf(",");
					}
				}
				printf("]");

				printf("{");

				while (wnode_2->m_watch != NULL)
				{
					printf("%d", wnode_2->m_watch->m_clause - m_clauses);
					wnode_2 = wnode_2->m_next;

					if (wnode_2 != NULL)
					{
						printf(",");
					}
				}
				printf("} ");
			}
			else
			{
				printf("-%d ", i);
			}
		}
		printf("\n");
		
		printf("variables\n");
		for (int i = 1; i <= m_V; ++i)
		{
			printf("%d=%d ", i, m_P_literals[i].m_value);
		}

		printf("\n");
	}


	void print_DIMACS(void)
	{
		printf("p cnf %d %d\n", m_V, m_cC);

		for (int i = 0; i < m_cC; ++i)
		{
			const Clause &clause = m_clauses[i];
			Literal **literal = clause.m_literals;

			while (*literal != NULL)
			{
				if ((*literal)->m_base == m_P_literals)
				{
					printf("%d ", *literal - (*literal)->m_base);
				}
				else
				{
					printf("-%d ", *literal - (*literal)->m_base);
				}
				++literal;
			}
			printf("0\n");
		}
	}


	void load_DIMACS(char *filename)
	{
		FILE *fr;

		m_conflicting = false;

		fr = fopen(filename, "r");
		assert(fr != NULL);

		skip_comments_DIMACS(fr);

		int V, C, O, M;
		fscanf(fr, "p cnf %d %d\n", &V, &C);

		O = 0;
		M = 0;

		for (int i = 0; i < C; ++i)
		{
			skip_comments_DIMACS(fr);

			int l, m = 0;

			while (true)
			{
				fscanf(fr, "%d", &l);
				++O;
				++m;

				if (l == 0)
				{
					fscanf(fr, "\n");
					break;
				}
			}

			if (m > M)
			{
				M = m;
			}

			if (feof(fr))
			{
				C = i + 1;
				break;
			}
		}

		fclose(fr);

		init(V, C, O);

		int *cl;
		cl = (int*)malloc(M * sizeof(int));
		assert(cl != NULL);

		fr = fopen(filename, "r");
		assert(fr != NULL);

		skip_comments_DIMACS(fr);

		fscanf(fr, "p cnf %d %d\n", &V, &C);

		for (int i = 0; i < C; ++i)
		{
			skip_comments_DIMACS(fr);

			int l, m = 0;

			while (true)
			{
				fscanf(fr, "%d", &l);
				cl[m] = l;

				if (l == 0)
				{
					fscanf(fr, "\n");
					break;
				}

				++m;
				++O;
			}

			add_general_clause(cl);

			if (feof(fr))
			{
				break;
			}
		}

		free(cl);
		fclose(fr);
	}


	void save_DIMACS(char *filename)
	{
		FILE *fw;

		fw = fopen(filename, "w");
		assert(fw != NULL);

		fprintf(fw, "p cnf %d %d\n", m_V, m_cC);

		for (int i = 0; i < m_cC; ++i)
		{
			const Clause &clause = m_clauses[i];
			Literal **literal = clause.m_literals;

			while (*literal != NULL)
			{
				if ((*literal)->m_base == m_P_literals)
				{
					fprintf(fw, "%ld ", *literal - (*literal)->m_base);
				}
				else
				{
					fprintf(fw, "-%ld ", *literal - (*literal)->m_base);
				}
				++literal;
			}
			fprintf(fw, "0\n");
		}
	}


	void skip_comments_DIMACS(FILE *fr)
	{
		char ch;

		while (true)
		{
			fscanf(fr, "%c", &ch);

			if (ch == 'c')
			{
				while (fgetc(fr) != '\n');
			}
			else
			{
				ungetc(ch, fr);
				return;
			}
		}
	}


public:
	int m_V;
	Variable *m_variables;
	Literal *m_P_literals;
	Literal *m_N_literals;

	int m_O, m_cO;
	Literal **m_occurences;

	int m_cLO;
	ONode *m_L_occurences;

	int m_C, m_cC;
	Clause *m_clauses;

	int m_cW;
	WNode *m_watch_nodes;

	bool m_conflicting;

// unit propagation specific
	Literal **m_unit_literals;
	Literal **m_last_unit;

	Literal **m_chng_literals;
	Literal **m_last_chng;

// solver specific
	Clause *m_head_clause;
	Variable *m_head_var;

// Path specific members
	int m_cA;
	Clause **m_affects;
};

#endif
