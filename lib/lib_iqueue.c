/*!
	@file   lib_iqueue.c
	@brief  <brief description here>
	@t.odo	-
	---------------------------------------------------------------------------

	MIT License
	Copyright (c) 2018 Io. D (Devcoons)

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/
/******************************************************************************
* Preprocessor Definitions & Macros
******************************************************************************/

/******************************************************************************
* Includes
******************************************************************************/

#include "lib_iqueue.h"

/******************************************************************************
* Enumerations, structures & Variables
******************************************************************************/

/******************************************************************************
* Declaration | Static Functions
******************************************************************************/

/******************************************************************************
* Definition  | Static Functions
******************************************************************************/

/******************************************************************************
* Definition  | Public Functions
******************************************************************************/

i_status iqueue_init(iqueue_t* _queue, int _max_elements, size_t _element_size, void* _storage)
{
	if (_queue != NULL)
	{
		memset(_storage, 0x00, _element_size * _max_elements);
		_queue->element_size = _element_size;
		_queue->max_elements = _max_elements;
		_queue->first = 0;
		_queue->next = (uintptr_t)_storage;
		_queue->storage = _storage;
		return I_OK;
	}
	return I_ERROR;
}

volatile void* iqueue_get_next_enqueue(iqueue_t* _queue)
{
	return (void*)_queue->next;
}

i_status iqueue_advance_next(iqueue_t* _queue)
{
	if (_queue->first != _queue->next)
	{
		_queue->first = _queue->first == 0 ? _queue->next : _queue->first;
		_queue->next = _queue->next + _queue->element_size == (uintptr_t)_queue->storage + (_queue->element_size * _queue->max_elements)
			? (uintptr_t)_queue->storage : _queue->next + _queue->element_size;
		return I_OK;
	}
	return I_FULL;
}

i_status iqueue_enqueue(iqueue_t* _queue, void* _element)
{
	if (_queue->first != _queue->next)
	{
		memmove((void*)_queue->next, (void*)_element, _queue->element_size);
		_queue->first = _queue->first == 0 ? _queue->next : _queue->first;
		_queue->next = _queue->next + _queue->element_size == (uintptr_t)_queue->storage + (_queue->element_size * _queue->max_elements)
			? (uintptr_t)_queue->storage : _queue->next + _queue->element_size;
		return I_OK;
	}
	return I_FULL;
}

i_status iqueue_dequeue(iqueue_t* _queue, void* _element)
{
	if (_queue->first != 0)
	{
		memmove((void*)_element, (void*)_queue->first, _queue->element_size);
		_queue->first = _queue->first + _queue->element_size == (uintptr_t)_queue->storage + (_queue->element_size * _queue->max_elements)
			? (uintptr_t)_queue->storage : _queue->first + _queue->element_size;
		_queue->first = _queue->first == _queue->next ? 0 : _queue->first;

		return I_OK;
	}
	return I_EMPTY;
}

void* iqueue_dequeue_fast(iqueue_t* _queue)
{
	if (_queue->first != 0)
	{
		uintptr_t x = _queue->first;
		_queue->first = _queue->first + _queue->element_size == (uintptr_t)_queue->storage + (_queue->element_size * _queue->max_elements)
			? (uintptr_t)_queue->storage : _queue->first + _queue->element_size;
		_queue->first = _queue->first == _queue->next ? 0 : _queue->first;
		return (void*)x;
	}
	return NULL;
}


i_status iqueue_size(iqueue_t* _queue, size_t* _size)
{
	*_size = _queue->first == 0
		? 0
		: _queue->first < _queue->next
		? (_queue->next - _queue->first) / _queue->element_size
		: _queue->max_elements - ((_queue->first - _queue->next) / _queue->element_size);

	return I_OK;
}

/******************************************************************************
* EOF - NO CODE AFTER THIS LINE
******************************************************************************/
