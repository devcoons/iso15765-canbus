/*!
@file   lib_iqueue.c
@brief  <brief description here>
@t.odo	-
---------------------------------------------------------------------------

GNU Affero General Public License v3.0  

Copyright (c) 2024 Ioannis D. (devcoons)  

This program is free software: you can redistribute it and/or modify it 
under the terms of the GNU Affero General Public License as published by 
the Free Software Foundation, either version 3 of the License.  

This program is distributed in the hope that it will be useful,  
but WITHOUT ANY WARRANTY; without even the implied warranty of  
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the  
GNU Affero General Public License for more details.  

You should have received a copy of the GNU Affero General Public License  
along with this program. If not, see <https://www.gnu.org/licenses/>.  

For commercial use, including proprietary or for-profit applications, 
a separate license is required. Contact:  

- GitHub: [https://github.com/devcoons](https://github.com/devcoons)  
- Email: i_-_-_s@outlook.com 
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

i_status iqueue_init(iqueue_t* _queue, uint32_t _max_elements, size_t _element_size, void* _storage)
{
    if ((_queue != NULL) && (_storage != NULL))
    {
        (void)memset(_storage, 0, _element_size * _max_elements);
		_queue->element_size = _element_size;
		_queue->max_elements = _max_elements;
		_queue->first = (uintptr_t)NULL;
		_queue->next = (uintptr_t)_storage;
		_queue->storage = _storage;
        return I_OK;
    }
    else
    {
        return I_ERROR;
    }
}

void* iqueue_get_next_enqueue(iqueue_t* _queue)
{
	return (_queue != NULL) ? (void *)(_queue->next) : NULL;
}

i_status iqueue_advance_next(iqueue_t* _queue)
{
	if (_queue->first != _queue->next)
	{
		uintptr_t storage_end = (uintptr_t)(_queue->storage) + (_queue->element_size * _queue->max_elements);
		_queue->first = (_queue->first == 0U) ? _queue->next : _queue->first;
		_queue->next = ((_queue->next + _queue->element_size) == storage_end) ? (uintptr_t)_queue->storage : (_queue->next + _queue->element_size);
		return I_OK;
	}
	else
	{
		return I_FULL;
	}
}

i_status iqueue_enqueue(iqueue_t* _queue, void* _element)
{
	if (_queue->first != _queue->next)
	{
		(void)memmove((void*)_queue->next, (void*)_element, _queue->element_size);
		return iqueue_advance_next(_queue);
	}
	return I_FULL;
}

i_status iqueue_dequeue(iqueue_t* _queue, void* _element)
{
	void* x = iqueue_dequeue_fast(_queue);
	if (x != NULL)
	{
		(void)memmove((void*)_element, x, _queue->element_size);
		return I_OK;
	}
	else
	{
		return I_EMPTY;
	}
}

void* iqueue_dequeue_fast(iqueue_t* _queue)
{
	if (_queue == NULL) {
		return NULL;
	}

	if (_queue->first != 0U) {
		void* ret_ptr = (void*)_queue->first;

		uintptr_t storage_end = (uintptr_t)_queue->storage + (_queue->element_size * _queue->max_elements);
		uintptr_t next_first = _queue->first + _queue->element_size;

		_queue->first = (next_first == storage_end) ? (uintptr_t)_queue->storage : next_first;
		_queue->first = (_queue->first == _queue->next) ? 0U : _queue->first;

		return ret_ptr;
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