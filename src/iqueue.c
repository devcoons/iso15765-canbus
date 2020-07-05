/*
 * iqueue.c
 *
 *  Created on: 31 Jul 2018
 *      Author: ioadel
 */


 /******************************************************************************
 * SOF - Source | By: Io.D
 ******************************************************************************/

 /******************************************************************************
 * Includes
 ******************************************************************************/

#include "iqueue.h"

 /******************************************************************************
 * Declarations
 ******************************************************************************/

 /******************************************************************************
 * Public functions
 ******************************************************************************/

I_Status_Queue iqueue_init(iqueue_t* _queue, int _max_elements, size_t _element_size, void* _storage)
{
	if (_queue == NULL)
		return I_ERROR;

	memset(_storage, 0x00, _element_size * _max_elements);

	_queue->element_size = _element_size;
	_queue->max_elements = _max_elements;

	_queue->first = (void*)0;
	_queue->next = _queue->storage = _storage;

	return I_OK;
}


volatile void* iqueue_get_next_enqueue(iqueue_t* _queue)
{
	return _queue->next;
}

I_Status_Queue iqueue_advance_next(iqueue_t* _queue)
{
	if (_queue->first == _queue->next)
		return I_FULL;

	_queue->first = _queue->first == (void*)0 ? _queue->next : _queue->first;
	_queue->next = (uint8_t*)_queue->next + _queue->element_size == (uint8_t*)_queue->storage + (_queue->element_size * _queue->max_elements)
		? _queue->storage : (uint8_t*)_queue->next + _queue->element_size;


	return I_OK;
}

I_Status_Queue iqueue_enqueue(iqueue_t* _queue, void* _element)
{
	if (_queue->first == _queue->next)
		return I_FULL;

	memmove((void*)_queue->next, (void*)_element, _queue->element_size);

	_queue->first = _queue->first == (void*)0 ? _queue->next : _queue->first;
	_queue->next = (uint8_t*)_queue->next + _queue->element_size == (uint8_t*)_queue->storage + (_queue->element_size * _queue->max_elements)
		? _queue->storage : (uint8_t*)_queue->next + _queue->element_size;

	return I_OK;
}

I_Status_Queue iqueue_dequeue(iqueue_t* _queue, void* _element)
{
	if (_queue->first == (void*)0)
		return I_EMPTY;

	memmove((void*)_element, (void*)_queue->first, _queue->element_size);

	_queue->first = (uint8_t*)_queue->first + _queue->element_size == (uint8_t*)_queue->storage + (_queue->element_size * _queue->max_elements)
		? _queue->storage : (uint8_t*)_queue->first + _queue->element_size;
	_queue->first = _queue->first == _queue->next ? (void*)0 : _queue->first;

	return I_OK;
}

volatile void* iqueue_dequeue_fast(iqueue_t* _queue)
{
	if (_queue->first == (void*)0)
		return NULL;

	void* x = (void*)_queue->first;

	_queue->first = (uint8_t*)_queue->first + _queue->element_size == (uint8_t*)_queue->storage + (_queue->element_size * _queue->max_elements)
		? _queue->storage : (uint8_t*)_queue->first + _queue->element_size;
	_queue->first = _queue->first == _queue->next ? (void*)0 : _queue->first;

	return x;
}


I_Status_Queue iqueue_size(iqueue_t* _queue, uint32_t* _size)
{
	*_size = _queue->first == (void*)0
		? 0
		: _queue->first < _queue->next
		? ((uintptr_t)_queue->next - (uintptr_t)_queue->first) / _queue->element_size
		: _queue->max_elements - (((uintptr_t)_queue->first - (uintptr_t)_queue->next) / _queue->element_size);

	return I_OK;
}

/******************************************************************************
* Static functions
******************************************************************************/

/******************************************************************************
* EOF - Source
******************************************************************************/