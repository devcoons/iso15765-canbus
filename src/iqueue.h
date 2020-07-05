/******************************************************************************
* SOF - Header | By: Io.D
******************************************************************************/

#ifndef LIBRARIES_IQUEUE_H_

/******************************************************************************
* Definitions
******************************************************************************/

#define LIBRARIES_IQUEUE_H_

/******************************************************************************
* Includes
******************************************************************************/

#include <inttypes.h>
#include <string.h>

/******************************************************************************
* Enumerations, structures & External Variables
******************************************************************************/

typedef enum
{
	I_OK = 0x00,
	I_ERROR = 0x01,
	I_FULL = 0x60,
	I_EMPTY = 0x61
}I_Status_Queue;

typedef struct
{
	volatile void* storage;
	volatile void* first;
	volatile void* next;
	volatile size_t element_size;
	volatile uint32_t max_elements;
}
iqueue_t;

/******************************************************************************
* Public Functions (API)
******************************************************************************/

I_Status_Queue 	iqueue_init(iqueue_t* _queue, int _max_elements, size_t _element_size, void* _storage);
I_Status_Queue 	iqueue_enqueue(iqueue_t* _queue, void* _element);
I_Status_Queue 	iqueue_dequeue(iqueue_t* _queue, void* _element);
I_Status_Queue	iqueue_size(iqueue_t* _queue, uint32_t* _size);
volatile void* iqueue_get_next_enqueue(iqueue_t* _queue);
volatile void* iqueue_dequeue_fast(iqueue_t* _queue);
I_Status_Queue iqueue_advance_next(iqueue_t* _queue);
/******************************************************************************
* EOF - Header
******************************************************************************/

#endif