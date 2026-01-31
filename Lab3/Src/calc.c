// Calculator app
#include "calc.h"
#include "display.h"
#include "touchpad.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "systick.h"
#define NMAX 10 // Maximum number of operands
#define DELAY_FOR_SORT 1000

typedef enum {
	APPMENU = 0, CLASSIC, GCD, FACTORIAL, FIBONACCI, SORT, AVERAGE
} app_case;
static Press_t mode;
static app_case current_case;  // Display time of each element of the sorted list

static Press_t operation;     // Selected operation
static Entry_t operand[NMAX]; // Numeric operands
static int count;             // Count of operands received
static Entry_t result;        // Calculation result
static Entry_t value1;
static Entry_t value2;
static Entry_t size;
static Time_t delay;
static int index_for_sort;

static enum {
	MENU, PROMPT, ENTRY, RUN, SHOW, WAIT
} state;

// Initialization
void Init_Calc(void) {
	DisplayEnable();
	TouchEnable();
	state = MENU;
	current_case = APPMENU;
}
// Runtime
void Task_Calc(void) {
	switch (current_case) {

	// Calculator app menu
	case APPMENU: {
		mode = TouchInput(CALC);
		switch ((int) mode) {
		case 1:
			current_case = CLASSIC;
			break;
		case 2:
			current_case = GCD;
			break;
		case 3:
			current_case = FACTORIAL;
			break;
		case 4:
			current_case = FIBONACCI;
			break;
		case 5:
			current_case = SORT;
			break;
		case 6:
			current_case = AVERAGE;
			break;

		default:
			DisplayPrint(CALC, 0, "Calculator app");
			DisplayPrint(CALC, 1, "Choose an op 1-6");
			break;
		}

		break;
	}

		// Classic 4-functions
	case CLASSIC: {
		switch (state) {
		case MENU:
			// Prepare for a new operation
			operation = NONE;
			for (int i = 0; i < NMAX; i++)
				operand[i] = 0;
			count = 0;
			result = 0;
			// Display menu
			DisplayPrint(CALC, 0, "Classic 4 function");
			DisplayPrint(CALC, 1, "1+ 2:- 3:X 4:/");
			state = PROMPT;
			break;

		case PROMPT:
			if (operation == NONE)
				operation = TouchInput(CALC);
			switch ((int) operation) {
			case 1: // Add
			case 2: // Subtract
			case 3: // Multiply
			case 4: // Divide

				if (count == 2)
					state = RUN;
				else {
					DisplayPrint(CALC, 0, "Enter a number:");
					state = ENTRY;
				}
				break;
			case 11: // back to menu
				current_case = APPMENU;
				break;
			default: // Do nothing
				break;
			}
			break;

		case ENTRY:
			bool done = TouchEntry(CALC, &operand[count]);
			DisplayPrint(CALC, 1, "%u", operand[count]);
			if (done) {
				count++;
				state = PROMPT;
			}
			break;
		case RUN:
			DisplayPrint(CALC, 0, "Calculating...");
			DisplayPrint(CALC, 1, "");
			state = SHOW;
			switch ((int) operation) {
			case 1: // Add
				result = CalcFunc(0, operand[0], operand[1]);
				break;
			case 2: // Subtract
				result = CalcFunc(1, operand[0], operand[1]);
				break;
			case 3: // Multiply
				result = CalcFunc(2, operand[0], operand[1]);
				break;
			case 4: // Divide
				// guard divide-by-zero
				if (operand[1] == 0) {
					result = 0;
				} else {
					result = CalcFunc(3, operand[0], operand[1]);
				}
				break;
			case 11: // back to menu
				current_case = APPMENU;
				break;
			default:
				break;
			}
			break;
		case SHOW:
			DisplayPrint(CALC, 0, "Result:");
			DisplayPrint(CALC, 1, "%u", result);
			state = WAIT;
			break;
		case WAIT:
			// Press any pad to return to the menu
			if (TouchInput(CALC) != NONE) {
				current_case = APPMENU;
				state = MENU;
			}
			break;
		}
		break;
	}

		// GCD
	case GCD: {

		switch (state) {
		case MENU:
			value1 = 0;
			value2 = 0;
			count = 0;
			result = 0;

			for (int i = 0; i < NMAX; i++)
				operand[i] = 0;
			DisplayPrint(CALC, 0, "GCD");
			DisplayPrint(CALC, 1, "Press 1 to advance");

			Press_t p = TouchInput(CALC);
			if ((int) p == 11) {
				current_case = APPMENU;
				break;
			}
			if ((int) p == 1) {
				state = PROMPT;
			}

			break;

		case PROMPT:
			if (count == 0) {
				DisplayPrint(CALC, 0, "Enter 1st number:");
				DisplayPrint(CALC, 1, "");
			} else {
				DisplayPrint(CALC, 0, "Enter 2nd number:");
				DisplayPrint(CALC, 1, "");
			}
			state = ENTRY;
			break;

		case ENTRY:
			bool done = TouchEntry(CALC, &operand[count]);
			DisplayPrint(CALC, 1, "%u", operand[count]);

			if (done) {
				count++;

				if (count == 1) {
					value1 = operand[0];
					state = PROMPT;
				} else if (count == 2) {
					value2 = operand[1];
					state = RUN;
				}
			}
			break;

		case RUN:
			DisplayPrint(CALC, 0, "Calculating...");
			DisplayPrint(CALC, 1, "");
			result = CalcGCD(value1, value2);
			state = SHOW;
			break;

		case SHOW:
			DisplayPrint(CALC, 0, "The GCD is:");
			DisplayPrint(CALC, 1, "%u", result);
			state = WAIT;
			break;
		case WAIT:
			if (TouchInput(CALC) != NONE) {
				current_case = APPMENU;
				state = MENU;
			}
			break;
		}
		break;
	}

		// Factorial
	case FACTORIAL: {
		switch (state) {
		case MENU:
			value1 = 0;
			result = 0;
			for (int i = 0; i < NMAX; i++)
				operand[i] = 0;
			DisplayPrint(CALC, 0, "Factorial");
			DisplayPrint(CALC, 1, "Press 1 to advance");
			Press_t p = TouchInput(CALC);

			if ((int) p == 11) {
				current_case = APPMENU;
				break;
			}
			if ((int) p == 1) {
				state = PROMPT;
			}
			break;

		case PROMPT:
			DisplayPrint(CALC, 0, "Enter a number:");
			DisplayPrint(CALC, 1, "");
			state = ENTRY;
			break;

		case ENTRY:
			bool done = TouchEntry(CALC, &operand[0]);
			DisplayPrint(CALC, 1, "%u", operand[0]);
			if (done) {
				value1 = operand[0];
				state = RUN;
			}
			break;
		case RUN:
			DisplayPrint(CALC, 0, "Calculating...");
			DisplayPrint(CALC, 1, "");
			result = CalcFactorial(value1);
			state = SHOW;
			break;
		case SHOW:
			DisplayPrint(CALC, 0, "The factorial is:");
			DisplayPrint(CALC, 1, "%u", result);
			state = WAIT;
			break;
		case WAIT:
			if (TouchInput(CALC) != NONE) {
				current_case = APPMENU;
				state = MENU;
			}
			break;
		}

		break;
	}

		// Fibonacci
	case FIBONACCI: {
		switch (state) {
		case MENU:
			value1 = 0;
			result = 0;
			for (int i = 0; i < NMAX; i++)
				operand[i] = 0;
			DisplayPrint(CALC, 0, "Fibonacci");
			DisplayPrint(CALC, 1, "Press 1 to advance");
			Press_t p = TouchInput(CALC);

			if ((int) p == 11) {
				current_case = APPMENU;
				break;
			}
			if ((int) p == 1) {
				state = PROMPT;
			}

			break;

		case PROMPT:
			DisplayPrint(CALC, 0, "Enter a number:");
			DisplayPrint(CALC, 1, "");
			state = ENTRY;
			break;

		case ENTRY:
			bool done = TouchEntry(CALC, &operand[0]);
			DisplayPrint(CALC, 1, "%u", operand[0]);
			if (done) {
				value1 = operand[0];
				state = RUN;
			}
			break;
		case RUN:
			DisplayPrint(CALC, 0, "Calculating...");
			DisplayPrint(CALC, 1, "");
			result = fib(value1);
			state = SHOW;
			break;
		case SHOW:
			DisplayPrint(CALC, 0, "The n term is:");
			DisplayPrint(CALC, 1, "%u", result);
			state = WAIT;
			break;
		case WAIT:
			if (TouchInput(CALC) != NONE) {
				current_case = APPMENU;
				state = MENU;
			}
			break;
		}

		break;
	}

		// Sort
	case SORT: {
		switch (state) {
		case MENU:
			value1 = 0;
			size = 0;
			count = 0;
			result = 0;
			DisplayPrint(CALC, 0, "SORT");
			DisplayPrint(CALC, 1, "Press 1 to advance");
			Press_t p = TouchInput(CALC);
			if ((int) p == 1) {
				state = PROMPT;
			}

			if ((int) p == 11){
				current_case = APPMENU;
			}

			break;

		case PROMPT:

			value1 = 0;

			if (count == 0 && size == 0) {
				DisplayPrint(CALC, 0, "Enter size");
				DisplayPrint(CALC, 1, "");
			} else {

				DisplayPrint(CALC, 0, "Enter number %d", count + 1);
				DisplayPrint(CALC, 1, "");
			}

			state = ENTRY;
			break;

		case ENTRY:

			if (count == 0 && size == 0) {
				Press_t p = TouchInput(CALC);

				if ((int) p == 11) {
					current_case = APPMENU;
					break;
				}

				if (p == NONE)
					break;

				size = (int) p;

				if (size < 1 || size > NMAX) {
					DisplayPrint(CALC, 1, "Invalid size!");
					size = 0;
					break;
				}

				for (int i = 0; i < size; i++)
					operand[i] = 0;

				state = PROMPT;
				break;
			}

			if (count < size) {
				bool done = TouchEntry(CALC, &value1);

				DisplayPrint(CALC, 1, " ");

				DisplayPrint(CALC, 1, "%u", value1);

				if (!done)
					break;

				operand[count] = value1;
				count++;

				state = PROMPT;
				break;
			}

			state = RUN;
			break;

		case RUN:
			DisplayPrint(CALC, 0, "Calculating...");
			DisplayPrint(CALC, 1, "");
			CalcSort(operand, size);
			delay = TimeNow();
			state = SHOW;
			index_for_sort =0;


			break;

		case SHOW:

			if (index_for_sort < size) {
				DisplayPrint(CALC, 0, "Sorted:");
				DisplayPrint(CALC, 1, "%u", operand[index_for_sort]);

				if ( TimePassed(delay) >= DELAY_FOR_SORT ) {
					delay = TimeNow();
					index_for_sort++;

				}
			} else {
				state = WAIT;
			}
			break;

		case WAIT:
			if (TouchInput(CALC) != NONE) {
				current_case = APPMENU;
				state = MENU;
			}
			break;
		}
		break;
	}

		// Average

	case AVERAGE: {
		switch (state) {
		case MENU:
			value1 = 0;
			size = 0;
			count = 0;
			result = 0;
			DisplayPrint(CALC, 0, "AVERAGE");
			DisplayPrint(CALC, 1, "Press 1 to advance");
			Press_t p = TouchInput(CALC);
			if ((int) p == 1) {
				state = PROMPT;
			}
			if ((int) p == 11) {
				current_case = APPMENU;
			}

			break;

		case PROMPT:

			value1 = 0;

			if (count == 0 && size == 0) {
				DisplayPrint(CALC, 0, "Enter size");
				DisplayPrint(CALC, 1, "");
			} else {

				DisplayPrint(CALC, 0, "Enter number %d", count + 1);
				DisplayPrint(CALC, 1, "");
			}

			state = ENTRY;
			break;

		case ENTRY:

			if (count == 0 && size == 0) {
				Press_t p = TouchInput(CALC);

				if ((int) p == 11) {
					state = APPMENU;
					break;
				}

				if (p == NONE)
					break;

				size = (int) p;

				if (size < 1 || size > NMAX) {
					DisplayPrint(CALC, 1, "Invalid size!");
					size = 0;
					break;
				}

				for (int i = 0; i < size; i++)
					operand[i] = 0;

				state = PROMPT;
				break;
			}

			if (count < size) {
				bool done = TouchEntry(CALC, &value1);

				DisplayPrint(CALC, 1, " ");

				DisplayPrint(CALC, 1, "%u", value1);

				if (!done)
					break;

				operand[count] = value1;
				count++;

				state = PROMPT;
				break;
			}

			state = RUN;
			break;

		case RUN:
			DisplayPrint(CALC, 0, "Calculating...");
			DisplayPrint(CALC, 1, "");
			result = CalcAverage(operand, size);

			state = SHOW;
			break;

		case SHOW:
			DisplayPrint(CALC, 0, "Result:");
			DisplayPrint(CALC, 1, "%u.%u", result/10,result%10);
			state = WAIT;
			break;

		case WAIT:
			if (TouchInput(CALC) != NONE) {
				current_case = APPMENU;
				state = MENU;
			}
			break;
		}
		break;
	}

		break;
	}
}
