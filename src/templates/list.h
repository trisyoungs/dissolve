/*
	*** Linked List Class
	*** src/templates/list.h
	Copyright T. Youngs 2012-2016

	This file is part of dUQ.

	dUQ is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	dUQ is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with dUQ.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DUQ_LIST_H
#define DUQ_LIST_H

#include <stdlib.h>
#include <stdio.h>

// Forward Declarations
template <class T> class List;

/*
 * MPIListItem Class
 * Basic class providing linked list pointers. Any class which is required to be contained in a linked list, and be 
 * broadcastable with the general broadcast routines must subclass MPIListItem.
 */
template <class T> class MPIListItem
{
	public:
	// Constructor
	MPIListItem<T>()
	{
		prev = NULL;
		next = NULL;
	}
	// Destructor (virtual)
	virtual ~MPIListItem<T>()
	{
	}
	// List pointers
	T* prev, *next;

	public:
	// Broadcast data from Master to all Slaves
	virtual bool broadcast() = 0;
};

/*
 * ListItem Class
 * Basic class providing linked list pointers. Any class which is required to be contained in a linked list must
 * subclass ListItem or MPIListItem
 */
template <class T> class ListItem
{
	public:
	// Constructor
	ListItem<T>()
	{
		prev = NULL;
		next = NULL;
	}
	// List pointers
	T *prev, *next;
};

/*
 * List Class
 * Linked list for user-defined classes. Any class which is required to be contained in a List must subclass ListItem.
*/
template <class T> class List
{
	public:
	// Constructor
	List<T>()
	{
		listHead_ = NULL;
		listTail_ = NULL;
		nItems_ = 0;
		regenerate_ = true;
		items_ = NULL;
	}
	// Destructor
	~List()
	{
		clear();
	}
	// Copy Constructor
	List<T>(const List<T>& source)
	{
		(*this) = source;
	}
	// Assignment operator
	List<T>& operator=(const List<T>& source)
	{
		// Clear any current data in the list...
		clear();
		T *newitem, *olditem;
		for (olditem = source.first(); olditem != NULL; olditem = olditem->next)
		{
			// To ensure that we don't mess around with the pointers of the old list, copy the object and then own it
			newitem = new T;
			*newitem = *olditem;
			newitem->prev = NULL;
			newitem->next = NULL;
			own(newitem);
		}

		// Don't deep-copy the static list, just flag that it must be regenerated if required.
		regenerate_ = true;

		return *this;
	}
	// Element access operator
	T* operator[](int index)
	{
		if ((index < 0) || (index >= nItems_))
		{
			printf("LIST_OPERATOR[] - Array index (%i) out of bounds (%i items in List) >>>>\n", index, nItems_);
			return NULL;
		}
		return array()[index];
	}


	/*
	 * Basic Data
	 */
	protected:
	// Pointers to head and tail of list
	T* listHead_, *listTail_;
	// Number of items in list
	int nItems_;
	// Static array of items
	T** items_;
	// Array regeneration flag
	bool regenerate_;

	public:
	// Clear the list
	void clear()
	{
		// Grab the item array, to make sure that it is up to date.
		array();

		// We will go through this array backwards, deleting the items here rather than using the remove() method.
		// In this way, any calls to find the item's index in destructors will succeed.
		for (int n=nItems_-1; n >= 0; --n) delete items_[n];

		// Delete static items array and reset all quantities
		if (items_) delete[] items_;
		items_ = NULL;
		listHead_ = NULL;
		listTail_ = NULL;
		nItems_ = 0;
		regenerate_ = true;
	}
	// Create empty list of size N
	void createEmpty(int size)
	{
		clear();
		for (int n=0; n<size; ++n) add();
		regenerate_ = true;
	}
	// Returns the number of items in the list
	int nItems() const
	{
		return nItems_;
	}
	// Returns the list head
	T* first() const
	{
		return listHead_;
	}
	// Returns the second item in the list
	T* second() const
	{
		return (listHead_ == NULL ? NULL : listHead_->next);
	}
	// Returns the list tail
	T* last() const
	{
		return listTail_;
	}


	/*
	 * Item Addition
	 */
	public:
	// Append an item to the list
	T* add()
	{
		T* newItem = new T;
		// Add the pointer to the list
		listHead_ == NULL ? listHead_ = newItem : listTail_->next = newItem;
		newItem->prev = listTail_;
		listTail_ = newItem;
		++nItems_;
		regenerate_ = true;
		return newItem;
	}
	// Prepend an item to the list
	T* prepend()
	{
		T* newItem = new T;
		
		// Add the pointer to the start of the list
		newItem->next = listHead_;
		listHead_ == NULL ? listTail_ = newItem : listHead_->prev = newItem;
		listHead_ = newItem;
		++nItems_;
		regenerate_ = true;
		return newItem;
	}
	// Add an item into the list at the position specified
	T* addAt(int position)
	{
		// If position is -1, or the list is empty, just add at end of the list
		if ((position == -1) || (nItems_ == 0)) return add();

		// If position is zero, add at start of list
		if (position == 0) return insertBefore(listHead_);

		// Get item at (position-1), and add a new item after it
		T* prevItem = array()[position-1];
		return insertAfter(prevItem);
	}
	// Insert an item into the list (after supplied item)
	T* insertAfter(T* item)
	{
		if (item == NULL) return prepend();

		T* newItem = new T;
		// Get pointer to next item after the supplied item (our newnext)
		T* newNext = item->next;
		// Re-point newprev and the new item
		item->next = newItem;
		newItem->prev = item;
		// Re-point newnext and the new item
		if (newNext != NULL) newNext->prev = newItem;
		else listTail_ = newItem;
		newItem->next = newNext;
		++nItems_;
		regenerate_ = true;
		return newItem;
	}
	// Insert an item into the list (before supplied item)
	T* insertBefore(T* item)
	{
		if (item == NULL)
		{
			printf("No item supplied to List<T>::insertBefore().\n");
			return NULL;
		}
		T* newItem = new T;
		// Get pointer to next item after the supplied item (our newprev)
		T* newPrev = item->prev;
		// Re-point newnext and the new item
		item->prev = newItem;
		newItem->next = item;
		// Re-point newprev and the new item
		if (newPrev != NULL) newPrev->next = newItem;
		else listHead_ = newItem;
		newItem->prev = newPrev;
		++nItems_;
		regenerate_ = true;
		return newItem;
	}
	// Add an existing item into this list
	void own(T* oldItem)
	{
		if (oldItem == NULL)
		{
			printf("Internal Error: NULL pointer passed to List<T>::own().\n");
			return;
		}
		// In the interests of 'pointer etiquette', refuse to own the item if its pointers are not NULL
		if ((oldItem->next != NULL) || (oldItem->prev != NULL))
		{
			printf("list::own <<<< List refused to own an item that still had ties >>>>\n");
			return;
		}
		listHead_ == NULL ? listHead_ = oldItem : listTail_->next = oldItem;
		oldItem->prev = listTail_;
		oldItem->next = NULL;
		listTail_ = oldItem;
		++nItems_;
		regenerate_ = true;
	}


	/*
	 * Item Removal
	 */
	public:
	// Disown the item, but do not delete it
	void disown(T* item)
	{
		if (item == NULL)
		{
			printf("Internal Error: NULL pointer passed to List<T>::disown().\n");
			return;
		}
		item->prev == NULL ? listHead_ = item->next : item->prev->next = item->next;
		item->next == NULL ? listTail_ = item->prev : item->next->prev = item->prev;
		item->next = NULL;
		item->prev = NULL;

		--nItems_;
		regenerate_ = true;
	}
	// Take the first item out of the list
	T* takeFirst()
	{
		if (listHead_ == NULL) return NULL;

		T* item = listHead_;
		disown(item);

		return item;
	}
	// Take the last item out of the list
	T* takeLast()
	{
		if (listTail_ == NULL) return NULL;

		T* item = listTail_;
		disown(item);

		return item;
	}
	// Remove an item from the list
	void remove(T* item)
	{
		if (item == NULL)
		{
			printf("Internal Error: NULL pointer passed to List<T>::remove().\n");
			return;
		}
		// Delete a specific item from the list
		item->prev == NULL ? listHead_ = item->next : item->prev->next = item->next;
		item->next == NULL ? listTail_ = item->prev : item->next->prev = item->prev;
		delete item;
		--nItems_;
		regenerate_ = true;
	}
	// Remove first item from the list
	void removeFirst()
	{
		if (listHead_ == NULL)
		{
			printf("Internal Error: No first item to delete in list.\n");
			return;
		}
		
		// Delete a first item from the list
		T *xitem = listHead_;
		xitem->next == NULL ? listTail_ = xitem->prev : xitem->next->prev = xitem->prev;
		listHead_ = xitem->next;
		delete xitem;
		--nItems_;
		regenerate_ = true;
	}
	// Remove last item from the list
	void removeLast()
	{
		if (listTail_ == NULL)
		{
			printf("Internal Error: No last item to delete in list.\n");
			return;
		}
		// Delete the last item from the list
		T *xitem = listTail_;
		xitem->prev == NULL ? listHead_ = xitem->next : xitem->prev->next = xitem->next;
		listTail_ = xitem->prev;
		delete xitem;
		--nItems_;
		regenerate_ = true;
	}
	// Remove an item from the list, and return the next in the list
	T* removeAndGetNext(T* item)
	{
		if (item == NULL)
		{
			printf("Internal Error: NULL pointer passed to List<T>::removeAndGetNext().\n");
			return NULL;
		}
		// Delete a specific item from the list, and return the next in the list
		T* result = item->next;
		item->prev == NULL ? listHead_ = item->next : item->prev->next = item->next;
		item->next == NULL ? listTail_ = item->prev : item->next->prev = item->prev;
		delete item;
		--nItems_;
		regenerate_ = true;
		return result;
	}
	// Cut item from list, bridging over specified item
	void cut(T* item)
	{
		if (item == NULL)
		{
			printf("Internal Error: NULL pointer passed to List<T>::cut().\n");
			return;
		}
		T *prev, *next;
		prev = item->prev;
		next = item->next;
		if (prev == NULL) listHead_ = next;
		else prev->next = next;
		if (next == NULL) listTail_ = prev;
		else next->prev = prev;
		item->next = NULL;
		item->prev = NULL;
		regenerate_ = true;
	}


	/*
	 * Array / Indexing
	 */
	public:
	// Find list index of supplied item
	int indexOf(T* item) const
	{
		int result = 0;
		for (T* i = listHead_; i != NULL; i = i->next)
		{
			if (item == i) return result;
			++result;
		}
		printf("Internal Error: List::indexOf() could not find supplied item.\n");
		return -1;
	}
	// Return nth item in List
	T* item(int n) const
	{
		if ((n < 0) || (n >= nItems_))
		{
			printf("Internal Error: List array index %i is out of bounds in List<T>::item().\n", n);
			return NULL;
		}
		int count = -1;
		for (T* item = listHead_; item != NULL; item = item->next) if (++count == n) return item;
		return NULL;
	}
	// Fills the supplied array with pointer values to the list items
	void fillArray(int nItems, T** itemArray)
	{
		int count = 0;
		T *i = listHead_;
		while (i != NULL)
		{
			itemArray[count] = i->item;
			++count;
			if (count == nItems) break;
			i = i->next;
			if (i == NULL) printf("List::fillArray <<<< Not enough items in list - requested %i, had %i >>>>\n", nItems ,nItems_);
		}
	}
	// Generate (if necessary) and return item array
	T** array()
	{
		if (!regenerate_) return items_;
		
		// Delete old atom list (if there is one)
		if (items_ != NULL) delete[] items_;
		items_ = NULL;

		if (nItems_ == 0) return items_;

		// Create new list
		items_ = new T*[nItems_];
		
		// Fill in pointers
		int count = 0;
		for (T* i = listHead_; i != NULL; i = i->next) items_[count++] = i;
		regenerate_ = false;
		return items_;
	}


	/*
	 * Search
	 */
	public:
	// Return whether the item is owned by the list
	bool contains(T* searchItem) const
	{
		T* item;
		for (item = listHead_; item != NULL; item = item->next) if (searchItem == item) break;
		return (item != NULL);
	}


	/*
	 * Item Moves
	 */
	private:
	// Swap two items in list (by pointer)
	void swap(T* item1, T* item2)
	{
		if ((item1 == NULL) || (item2 == NULL))
		{
			printf("Internal Error: NULL pointer(s) passed to List<T>::swap(%p,%p).\n", item1, item2);
			return;
		}
		// If the items are adjacent, swap the pointers 'outside' the pair and swap the next/prev between them
		T *n1, *n2, *p1, *p2;
		if ((item1->next == item2) || (item2->next == item1))
		{
			// Order the pointers so that item1->next == item2
			if (item2->next == item1)
			{
				n1 = item2;
				item2 = item1;
				item1 = n1;
			}
			p1 = item1->prev; 
			n2 = item2->next;
			item2->prev = p1;
			item2->next = item1;
			if (p1 != NULL) p1->next = item2;
			else listHead_ = item2;
			item1->prev = item2;
			item1->next = n2;
			if (n2 != NULL) n2->prev = item1;
			else listTail_ = item1;
		}
		else
		{
			// Store the list pointers of the two items
			//printf("Item 1 %p next %p prev %p\n",item1,item1->next,item1->prev);
			//printf("Item 2 %p next %p prev %p\n",item2,item2->next,item2->prev);
			//printf("Item 1 nextprev %p prevnext %p\n",item1->next->prev,item1->prev->next);
			//printf("Item 2 nextprev %p prevnext %p\n",item2->next->prev,item2->prev->next);
			n1 = item1->next;
			p1 = item1->prev;
			n2 = item2->next;
			p2 = item2->prev;
			// Set new values of swapped items
			item1->next = n2;
			item1->prev = p2;
			item2->next = n1;
			item2->prev = p1;
			//printf("Item 1 next %p prev %p\n",item1->next,item1->prev);
			//printf("Item 2 next %p prev %p\n",item2->next,item2->prev);
			// Set new values of items around swapped items
			if (item1->next != NULL) item1->next->prev = item1;
			else listTail_ = item1;
			if (item1->prev != NULL) item1->prev->next = item1;
			else listHead_ = item1;
			if (item2->next != NULL) item2->next->prev = item2;
			else listTail_ = item2;
			if (item2->prev != NULL) item2->prev->next = item2;
			else listHead_ = item2;
			//printf("Item 1 nextprev %p prevnext %p\n",item1->next->prev,item1->prev->next);
			//printf("Item 2 nextprev %p prevnext %p\n",item2->next->prev,item2->prev->next);
		}
		regenerate_ = true;
	}

	public:
	// Shift item up (towards head)
	void shiftUp(T* item)
	{
		if (item == NULL)
		{
			printf("Internal Error: NULL pointer passed to List<T>::shiftUp().\n");
			return;
		}
		// If the item is already at the head of the list, return NULL.
		if (listHead_ == item) return;
		swap(item->prev,item);
		regenerate_ = true;
	}
	// Shift item down (towards tail)
	void shiftDown(T* item)
	{
		if (item == NULL)
		{
			printf("Internal Error: NULL pointer passed to List<T>::shiftDown().\n");
			return;
		}

		// If the item is already at the tail of the list, return.
		if (listTail_ == item) return;
		swap(item->next,item);
		regenerate_ = true;
	}
	// Move item at position 'old' by 'delta' positions (+/-)
	void move(int target, int delta)
	{
		// Check positions
		if ((target < 0) || (target >= nItems_))
		{
			printf("Internal Error: Old position (%i) is out of range (0 - %i) in List<T>::move\n", target, nItems_-1);
			return;
		}
		int newpos = target + delta;
		if ((newpos < 0) || (newpos >= nItems_))
		{
			printf("Internal Error: New position (%i) is out of range (0 - %i) in List<T>::move\n", newpos, nItems_-1);
			return;
		}
		// Get pointer to item that we're moving and shift it
		T *olditem = array()[target];
		for (int n=0; n<abs(delta); n++) (delta < 0 ? shiftUp(olditem) : shiftDown(olditem));
		// Flag for regeneration
		regenerate_ = true;
	}
	// Move item to end of list
	void moveToEnd(T* item)
	{
		if (item == NULL)
		{
			printf("Internal Error: NULL pointer passed to List<T>::moveToEnd().\n");
			return;
		}
		// If the item is already at the tail, exit
		if (listTail_ == item) return;
		cut(item);
		item->prev = listTail_;
		item->next = NULL;
		if (listTail_ != NULL) listTail_->next = item;
		listTail_ = item;
		regenerate_ = true;
	}
	// Move item to start of list
	void moveToStart(T* item)
	{
		if (item == NULL)
		{
			printf("Internal Error: NULL pointer passed to List<T>::moveToStart().\n");
			return;
		}
		// If the item is already at the head, exit
		if (listHead_ == item) return;
		cut(item);
		item->prev = NULL;
		item->next = listHead_;
		if (listHead_ != NULL) listHead_->prev = item;
		listHead_ = item;
		regenerate_ = true;
	}
	// Move item so it is after specified item
	void moveAfter(T* item, T* reference)
	{
		// If 'reference' is NULL, then move to the start of the list
		if (item == NULL)
		{
			printf("Internal Error: NULL pointer passed to List<T>::moveAfter().\n");
			return;
		}

		// Cut item out of list...
		T *prev, *next;
		prev = item->prev;
		next = item->next;
		if (prev == NULL) listHead_ = next;
		else prev->next = next;
		if (next == NULL) listTail_ = prev;
		else next->prev = prev;

		// ...and then re-insert it
		// Get pointer to next item after newprev (our newnext)
		T *newnext = (reference == NULL ? listHead_ : reference->next);
		// Re-point reference and the new item
		if (reference != NULL) reference->next = item;
		else listHead_ = item;
		item->prev = reference;
		// Re-point newnext and the new item
		if (newnext != NULL) newnext->prev = item;
		else listTail_ = item;
		item->next = newnext;
		regenerate_ = true;
	}
        // Swap two items in list
        void swapByIndex(int id1, int id2)
	{
		// Check positions
		if ((id1 < 0) || (id1 >= nItems_))
		{
			printf("Internal Error: First index (%i) is out of range (0 - %i) in List<T>::swapByIndex\n", id1, nItems_-1);
			return;
		}
		if ((id2 < 0) || (id2 >= nItems_))
		{
			printf("Internal Error: Second index (%i) is out of range (0 - %i) in List<T>::swapByIndex\n", id2, nItems_-1);
			return;
		}

		// Get pointers to item that we're swapping, and swap them
		swap(array()[id1], array()[id2]);

		// Flag for regeneration
		regenerate_ = true;
	}
};

#endif
