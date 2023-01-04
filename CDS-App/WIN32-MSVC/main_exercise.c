/*
 * FreeRTOS Kernel V10.1.1
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, software written by omkar, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/******************************************************************************
 * NOTE: Windows will not be running the FreeRTOS demo threads continuously, so
 * do not expect to get real time behaviour from the FreeRTOS Windows port, or
 * this demo application.  Also, the timing information in the FreeRTOS+Trace
 * logs have no meaningful units.  See the documentation page for the Windows
 * port for further information:
 * http://www.freertos.org/FreeRTOS-Windows-Simulator-Emulator-for-Visual-Studio-and-Eclipse-MingW.html
 * 
 ******************************************************************************
 *
 * NOTE:  Console input and output relies on Windows system calls, which can
 * interfere with the execution of the FreeRTOS Windows port.  This demo only
 * uses Windows system call occasionally.  Heavier use of Windows system calls
 * can crash the port.
 */

/* Standard includes. */
#include <stdio.h>
#include <conio.h>
#include <stdint.h>
#include <stdbool.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

#define JOB_COUNT 19

#if CONFIG_FREERTOS_UNICORE
	static const BaseType_t app_cpu = 0;
#else
	static const BaseType_t app_cpu = 1;
#endif

typedef struct CDS_job_set {
	uint32_t t_rel_start;
	uint32_t t_exec;

	void (*task_routine)(void);
} CDS_job_set;

void t1_routine(void* taskParameters);
void t2_routine(void* taskParameters);
void t3_routine(void* taskParameters);
void t4_routine(void* taskParameters);

void cyclic_executer(void* taskParameters);

/* Static schedule decided beforehand */
const CDS_job_set job_set[JOB_COUNT] =
{
	/* START TIME, EXECUTION TIME, ROUTINE */
	{0U,		   10U,			   &t1_routine},
	{10U,		   12U,			   &t4_routine},
	{22U,		   15U,			   &t3_routine},
	{37U,		   11U,			   &t2_routine},
	{100U,		   10U,			   &t1_routine},
	{110U,		   12U,			   &t4_routine},
	{150U,		   15U,			   &t3_routine},
	{200U,		   10U,			   &t1_routine},
	{210U,		   12U,			   &t4_routine},
	{222U,		   11U,			   &t2_routine},
	{300U,		   10U,			   &t1_routine},
	{310U,		   12U,			   &t4_routine},
	{322U,		   15U,			   &t3_routine},
	{400U,		   10U,			   &t1_routine},
	{410U,		   12U,			   &t4_routine},
	{422U,		   11U,			   &t2_routine},
	{450U,		   15U,			   &t3_routine},
	{500U,		   10U,			   &t1_routine},
	{510U,		   12U,			   &t4_routine},
};

void main_exercise(void) {

	/* We create solely one task which handles our pre-defined schedule. */
	xTaskCreate(&cyclic_executer, "Main Task",
				configMINIMAL_STACK_SIZE,
				NULL, 1	, NULL);

	/* Summon the In-built FreeRTOS Task Scheduler */
	vTaskStartScheduler();
}

/* Main function responsible for running the static schedule */
void cyclic_executer(void* taskParameters) {
	static uint16_t iter = 0, hyperperiod = 600;
	
	while (true) {
		for (uint16_t i = 0; i < JOB_COUNT; i++) {
			uint64_t curr_job_end = (iter * hyperperiod) +
									(job_set[i].t_rel_start +
									 job_set[i].t_exec);
			uint64_t next_job_start = 0;
			
			/* As soon as we reach the last job of one hyperperiod,
			 * we increase the iteration by one.
			 */
			if (i == JOB_COUNT - 1)
				iter++;

			next_job_start = (iter * hyperperiod) +
							  job_set[(i + 1) % 20].t_rel_start;

			job_set[i].task_routine();

			if (curr_job_end < next_job_start) {
				uint16_t delay = next_job_start - curr_job_end;
				/*
				 * Cyclic executer sleeps until it is time to execute another
				 * task. It sleeps for the time difference between the end of
				 * current job and the start of the next.
				 * 
				 * Additionally, there is a delay of 1 tick at the start of
				 * the schedule, most likely accounting for the setup of the
				 * run-through of the schedule.
				 */
				vTaskDelay(delay);

				printf("\n========================");
				printf("\nSLEEP for %d Ticks\n", delay);
				printf("========================\n\n");
			}
		}

		printf("XXXXXXXXXXXXXXXXXXXXXX\n");
		printf("END OF CYCLE %d\n", iter);
		printf("XXXXXXXXXXXXXXXXXXXXXX\n\n");
	}
}


/* Routines for each task */
void t1_routine(void* taskParameters) {
	static uint64_t t1_counter;
	const TickType_t t1_exec = 10;
	static TickType_t t1_lastWakeUpTime;

	while (true) {
		++t1_counter;
		t1_lastWakeUpTime = xTaskGetTickCount();

		printf("[T1] Current Tick %d\n", t1_lastWakeUpTime);

		/*
		 * Since the actual task Ti isn't itself executing like in the case
		 * of the RMA schedule in Task 1, the call 'vTaskDelayUntil()' in
		 * fact delays the 'cyclic_executor()' function for the execution
		 * time of the task Ti in question.
		 * 
		 * Though, one thing to keep in mind is that the for any task, the
		 * routine 'ti_routine()' is invoked in accordance with the schedule
		 * 'job_set'.
		 */
		vTaskDelayUntil(&t1_lastWakeUpTime, t1_exec);
		break;
	}
}

void t2_routine(void* taskParameters) {
	static uint64_t t2_counter;
	const TickType_t t2_exec = 11;
	static TickType_t t2_lastWakeUpTime;

	while (true) {
		++t2_counter;
		t2_lastWakeUpTime = xTaskGetTickCount();

		printf("[T2] Current Tick %d\n", t2_lastWakeUpTime);

		vTaskDelayUntil(&t2_lastWakeUpTime, t2_exec);
		break;
	}
}

void t3_routine(void* taskParameters) {
	static uint64_t t3_counter;
	static TickType_t t3_lastWakeUpTime;
	const TickType_t t3_exec = 15;

	while (true) {
		++t3_counter;
		t3_lastWakeUpTime = xTaskGetTickCount();

		printf("[T3] Current Tick %d\n", t3_lastWakeUpTime);

		vTaskDelayUntil(&t3_lastWakeUpTime, t3_exec);
		break;
	}
}

void t4_routine(void* taskParameters) {
	static uint64_t t4_counter;
	static TickType_t t4_lastWakeUpTime;
	const TickType_t t4_exec = 12;

	while (true) {
		++t4_counter;
		t4_lastWakeUpTime = xTaskGetTickCount();

		printf("[T4] Current Tick %d\n", t4_lastWakeUpTime);

		vTaskDelayUntil(&t4_lastWakeUpTime, t4_exec);
		break;
	}
}
