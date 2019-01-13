/* Pavel Surynek, 2009 */
/* pavel.surynek@mff.cuni.cz */
/* vsat_main.cpp */

/*
  A SAT solver called vsat.
 */


#include "vsat.h"

void test1(void)
{
//	formula.print();
/*
	exit(0);

	int cl1[] = {2, 4, 6, 8, 9, 0};
	int cl2[] = {1, 3, 5, 7,  9, 0};
	int cl3[] = {2, -3, -7, 8, 9, 0};
	int cl4[] = {4, 5, -6, 7, -8, 0};
	int cl5[] = {1, -2, -7, 8, 9, 0};

	Formula formula;
	formula.init(10, 40, 200);

	formula.add_2_clause(-3, 2);
	formula.add_2_clause(5, -6);
	formula.add_3_clause(3, 4, -6);
	formula.add_3_clause(-2, -3, 5);
	formula.add_3_clause(7, 8, -2);

	formula.add_3_clause(4, 6, -3);
	formula.add_2_clause(2, 1);
	formula.add_2_clause(-1, -2);

	formula.add_3_clause(2, 6, -7);
	formula.add_3_clause(4, 5, -8);
	formula.add_3_clause(1, -2, -8);
	formula.add_3_clause(-2, -3, -7);

	formula.add_2_clause(3, 4);
	formula.add_2_clause(5, 6);
	formula.add_2_clause(7, -8);

	formula.add_3_clause(2, -6, 7);
	formula.add_3_clause(4, -7, -8);
	formula.add_3_clause(-1, 2, 9);
	formula.add_3_clause(1, 4, 6);

	formula.add_3_clause(3, 6, 7);
	formula.add_3_clause(4, 7, -8);
	formula.add_3_clause(-1, -2, 9);
	formula.add_3_clause(-1, -4, 6);

	formula.add_general_clause(cl1);
	formula.add_general_clause(cl2);
	formula.add_general_clause(cl3);
	formula.add_general_clause(cl4);
	formula.add_general_clause(cl5);

	formula.print();

	for (int i = 0; i < formula.m_cC; ++i)
	{
		printf("%d\n", Formula::calc_value(formula.m_clauses[i]));
	}
*/

//	formula.print();
/*
	for (int i = 0; i < formula.m_cC; ++i)
	{
		printf("%d\n", Formula::calc_value(formula.m_clauses[i]));
	}
*/
/*
	printf("First literal in first clause set TRUE\n");
	formula.set_TRUE_propagate(formula.m_clauses[0].m_literals[0]);
	formula.print();

	for (int i = 0; i < formula.c_C; ++i)
	{
		printf("%d\n", Formula::calc_value(formula.m_clauses[i]));
	}

	printf("Second literal in first clause set TRUE\n");
	formula.set_TRUE_propagate(formula.m_clauses[0].m_literals[1]);
	formula.print();

	for (int i = 0; i < formula.c_C; ++i)
	{
		printf("%d\n", Formula::calc_value(formula.m_clauses[i]));
	}

	printf("First literal in fifth clause set FALSE\n");
	Literal **check_point = formula.get_check_point();
	formula.set_FALSE_propagate(formula.m_clauses[5].m_literals[0]);

	formula.print();

	for (int i = 0; i < formula.c_C; ++i)
	{
		printf("%d\n", Formula::calc_value(formula.m_clauses[i]));
	}


	printf("Undoing last change.\n");

	formula.unset_unpropagate(check_point);
	formula.print();

	for (int i = 0; i < formula.c_C; ++i)
	{
		printf("%d\n", Formula::calc_value(formula.m_clauses[i]));
	}
*/
}


enum COMMAND
{
	UNDEFINED = 0,
	SOLVE     = 1
};


void print_intro(void)
{
	printf("VSAT %s\n", VERSION);
	printf("Copyright (C) 2009 Pavel Surynek\n");
	printf("--------------------------------\n");
}


int main(int argc, char **argv)
{
	COMMAND cmd;
	char *filename;

	clock_t begin, end;

	begin = clock();

	print_intro();

	if (argc == 2)
	{
		cmd = SOLVE;
		filename = argv[1];
	}
	else
	{
		cmd = UNDEFINED;
	}


	switch (cmd)
	{
	case SOLVE:
	{
		Formula formula;
		formula.load_DIMACS(filename);

//		formula.print();
/*
		bool solved = formula.solve_1();

		end = clock();

		if (solved)
		{
			printf("SAT\n");
		}
		else
		{
			printf("UNSAT\n");
		}
*/
		Clause *path[128];

		for (int c = 0; c < formula.m_C; ++c)
		{
			formula.find_conflicting_path(&formula.m_clauses[c], path, 10);
			formula.print_path(path, 10);
			formula.reset_path(path, 10);
		}

//		formula.print();

#ifdef STAT
		printf("Decisions:%d\n", STAT_DECISIONS);
		printf("Backtracks:%d\n", STAT_BACKTRACKS);
		printf("Propagations:%d\n", STAT_PROPAGATIONS);
#endif
		printf("Time:%.3f\n", (end - begin) / (double)CLOCKS_PER_SEC);
		break;
	}
	default:
	{
		printf("No command specified.\n");
		break;
	}
	}

	return 0;
}
