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

#define TASK_COUNT 5

#if CONFIG_FREERTOS_UNICORE
	static const BaseType_t app_cpu = 0;
#else
	static const BaseType_t app_cpu = 1;
#endif

typedef struct {
	uint16_t t_id;

	uint32_t t_period;
	uint32_t t_deadline;
	uint32_t t_exec;

	uint16_t t_priority;

	TaskHandle_t t_taskHandle;

	void (*task_routine)(void);
} RMA_task;

static RMA_task task_set[TASK_COUNT];

void t1_routine(void* taskParameters);
void t2_routine(void* taskParameters);
void t3_routine(void* taskParameters);
void t4_routine(void* taskParameters);
void t5_routine(void* taskParameters);

void main_exercise(void) {

	/*
	 * Initialise the properties of each task. The periods, deadlines and
	 * execution times have been defined below solely for documentation
	 * and code clarity purposes. The actual definitions are in the
	 * respective task routines 'ti_routine' as parameters of
	 * 'vTaskDelayUntil()'.
	 */
	task_set[0].t_id = 1;
	task_set[0].task_routine = &t1_routine;
	task_set[0].t_period = 100U;
	task_set[0].t_deadline = 100U;
	task_set[0].t_exec = 10U;
	task_set[0].t_priority = 4;

	task_set[0].t_id = 2;
	task_set[1].task_routine = &t2_routine;
	task_set[1].t_period = 200U;
	task_set[1].t_deadline = 200U;
	task_set[1].t_exec = 30U;
	task_set[0].t_priority = 1;

	task_set[0].t_id = 3;
	task_set[2].task_routine = &t3_routine;
	task_set[2].t_period = 50U;
	task_set[2].t_deadline = 50U;
	task_set[2].t_exec = 10U;
	task_set[0].t_priority = 5;

	task_set[0].t_id = 4;
	task_set[3].task_routine = &t4_routine;
	task_set[3].t_period = 150U;
	task_set[3].t_deadline = 150U;
	task_set[3].t_exec = 15U;
	task_set[0].t_priority = 2;

	task_set[0].t_id = 5;
	task_set[4].task_routine = &t5_routine;
	task_set[4].t_period = 100U;
	task_set[4].t_deadline = 100U;
	task_set[4].t_exec = 12U;
	task_set[0].t_priority = 3;

	/* Create tasks for the FreeRTOS scheduler */
	for (int task = 0; task < TASK_COUNT; task++)
		xTaskCreate(task_set[task].task_routine, "Task " + (task + 1),
					configMINIMAL_STACK_SIZE,
					NULL, task_set[task].t_priority,
					task_set[task].t_taskHandle);

	/* Begin the In-built FreeRTOS Task Scheduler */
	vTaskStartScheduler();
}


/* Routines for each task */
void t1_routine(void* taskParameters) {
	static uint64_t t1_counter;
	const TickType_t t1_period = 100;
	static TickType_t t1_lastWakeUpTime;

	while (true) {
		++t1_counter;

		if (t1_counter == 1)
			printf("====Initialisation====\n");

		t1_lastWakeUpTime = xTaskGetTickCount();
		printf("[T1] Current cycle: %llu | T1 curent tick %d\n", t1_counter, t1_lastWakeUpTime);

		/* Next job of the task delayed until current time plus the period */
		vTaskDelayUntil(&t1_lastWakeUpTime, t1_period);
	}
}

void t2_routine(void* taskParameters) {
	static uint64_t t2_counter;
	const TickType_t t2_period = 200;
	static TickType_t t2_lastWakeUpTime;

	while (true) {
		++t2_counter;

		t2_lastWakeUpTime = xTaskGetTickCount();
		printf("[T2] Current cycle: %llu | T2 curent tick %d\n", t2_counter, t2_lastWakeUpTime);
		
		if (t2_counter != 1) {
			printf("===============================================\n");
			printf("All jobs from Cycle %llu finished\n\n", t2_counter);
		}
		vTaskDelayUntil(&t2_lastWakeUpTime, t2_period);
	}
}

void t3_routine(void* taskParameters) {
	static uint64_t t3_counter;
	static TickType_t t3_lastWakeUpTime;
	const TickType_t t3_period = 50;

	while (true) {
		++t3_counter;
		t3_lastWakeUpTime = xTaskGetTickCount();

		printf("[T3] Current cycle: %llu | T3 curent tick %d\n", t3_counter, t3_lastWakeUpTime);

		vTaskDelayUntil(&t3_lastWakeUpTime, t3_period);
	}
}

void t4_routine(void* taskParameters) {
	static uint64_t t4_counter;
	static TickType_t t4_lastWakeUpTime;
	const TickType_t t4_period = 150;

	while (true) {
		++t4_counter;

		t4_lastWakeUpTime = xTaskGetTickCount();
		printf("[T4] Current cycle: %llu | T4 curent tick %d\n", t4_counter, t4_lastWakeUpTime);

		vTaskDelayUntil(&t4_lastWakeUpTime, t4_period);
	}
}

void t5_routine(void* taskParameters) {
	static uint64_t t5_counter;
	const TickType_t t5_period = 100;
	static uint64_t t5_lastWakeUpTime;

	while (true) {
		++t5_counter;

		t5_lastWakeUpTime = xTaskGetTickCount();
		printf("[T5] Current cycle: %llu | T5 curent tick %d\n", t5_counter, t5_lastWakeUpTime);

		if (t5_counter == 1)
			printf("======================\n\n");

		vTaskDelayUntil(&t5_lastWakeUpTime, t5_period);
	}
}