#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "cengine/collections/dlist.h"

static ListElement *list_element_new (void) {

    ListElement *le = (ListElement *) malloc (sizeof (ListElement));
    if (le) {
        le->next = le->prev = NULL;
        le->data = NULL;
    }

    return le;

}

static inline void list_element_delete (ListElement *le) { if (le) free (le); }

void dlist_set_compare (DoubleList *list, int (*compare)(void *one, void *two)) { if (list) list->compare = compare; }

void dlist_set_destroy (DoubleList *list, void (*destroy)(void *data)) { if (list) list->destroy = destroy; }

DoubleList *dlist_init (void (*destroy)(void *data), int (*compare)(void *one, void *two)) {

    DoubleList *list = (DoubleList *) malloc (sizeof (DoubleList));

    if (list) {
        memset (list, 0, sizeof (DoubleList));
        list->destroy = destroy;
        list->compare = compare; 
    }

    return list;

}

void dlist_reset (DoubleList *list) {

    if (list) {
        if (dlist_size (list) > 0) {
            void *data = NULL;
            while (dlist_size (list) > 0) {
                data = dlist_remove_element (list, NULL);
                if (data != NULL && list->destroy != NULL) list->destroy (data);
            }
        }

        list->start = NULL;
        list->end = NULL;
        list->size = 0;
    }

}

// only gets rid of the List elemenst, but the data is kept
// this is used if another list or structure points to the same data
void dlist_clean (DoubleList *list) {

    if (list) {
        void *data = NULL;
        while (dlist_size (list) > 0) 
            data = dlist_remove_element (list, NULL);

        free (list);
    }

}

void dlist_destroy (DoubleList *list) {

    if (list) {
        if (list->size > 0) {
            void *data = NULL;

            while (dlist_size (list) > 0) {
                data = dlist_remove_element (list, NULL);
                if (data) {
                    if (list->destroy) list->destroy (data);
                    else free (data);
                }
            }
        }

        free (list);
    }

}

bool dlist_insert_after (DoubleList *list, ListElement *element, void *data) {

    if (list && data) {
        ListElement *le = list_element_new ();
        if (!le) return false;

        le->data = (void *) data;

        if (element == NULL) {
            if (dlist_size (list) == 0) list->end = le;
            else list->start->prev = le;
        
            le->next = list->start;
            le->prev = NULL;
            list->start = le;
        }

        else {
            if (element->next == NULL) list->end = le;

            le->next = element->next;
            le->prev = element;
            element->next = le;
        }

        list->size++;

        return true;
    }

    return false;

}

// removes the list element from the list and returns the data
void *dlist_remove_element (DoubleList *list, ListElement *element) {

    if (list) {
        ListElement *old;
        void *data = NULL;

        if (dlist_size (list) == 0) return NULL;

        if (element == NULL) {
            data = list->start->data;
            old = list->start;
            list->start = list->start->next;
            if (list->start != NULL) list->start->prev = NULL;
        }

        else {
            data = element->data;
            old = element;

            ListElement *prevElement = element->prev;
            ListElement *nextElement = element->next;

            if (prevElement != NULL && nextElement != NULL) {
                prevElement->next = nextElement;
                nextElement->prev = prevElement;
            }

            else {
                // we are at the start of the list
                if (prevElement == NULL) {
                    if (nextElement != NULL) nextElement->prev = NULL;
                    list->start = nextElement;
                }

                // we are at the end of the list
                if (nextElement == NULL) {
                    if (prevElement != NULL) prevElement->next = NULL;
                    list->end = prevElement;
                }
            }
        }

        list_element_delete (old);
        list->size--;

        return data;
    }

    return NULL;

}

/*** Traversing --- Searching ***/

void *dlist_search (DoubleList *list, void *data) {

    if (list && data) {
        ListElement *ptr = dlist_start (list);
        while (ptr != NULL) {
            if (ptr->data == data) return ptr->data;
            ptr = ptr->next;
        }

        return NULL;    // not found
    }

    return NULL;    

}

bool dlist_is_in_list (DoubleList *list, void *data) {

    if (list && data) {
        ListElement *ptr = dlist_start (list);

        if (list->compare) {
            while (ptr != NULL) {
                if (!list->compare (ptr->data, data)) return true;
                ptr = ptr->next;
            }
        }

        else {
            while (ptr != NULL) {
                if (ptr->data == data) return true;
                ptr = ptr->next;
            }
        }

        return false;   // not found
    }

    return false;

}

// searches the list and returns the list element associated with the data
// can use a compartor set in the list
ListElement *dlist_get_element (DoubleList *list, void *data) {

    if (list && data) {
        ListElement *ptr = dlist_start (list);

        if (list->compare) {
            while (ptr != NULL) {
                if (!list->compare (ptr->data, data)) return ptr;
                ptr = ptr->next;
            }
        }

        else {  
            while (ptr != NULL) {
                if (ptr->data == data) return ptr;
                ptr = ptr->next;
            }
        }

        return NULL;    // not found
    }

    return NULL;    

}

/*** SORTING ***/

// Split a doubly linked list (DLL) into 2 DLLs of half sizes 
static ListElement *dllist_split (ListElement *head) { 

    ListElement *fast = head, *slow = head; 

    while (fast->next && fast->next->next) { 
        fast = fast->next->next; 
        slow = slow->next; 
    } 

    ListElement *temp = slow->next; 
    slow->next = NULL; 

    return temp; 

}  

// Function to merge two linked lists 
static ListElement *dllist_merge (int (*compare)(void *one, void *two), 
    ListElement *first, ListElement *second)  { 

    // If first linked list is empty 
    if (!first) return second; 
  
    // If second linked list is empty 
    if (!second) return first; 

    // Pick the smallest value 
    if (compare (first->data, second->data) <= 0) {
        first->next = dllist_merge (compare, first->next, second); 
        first->next->prev = first; 
        first->prev = NULL; 
        return first; 
    }

    else {
        second->next = dllist_merge (compare, first, second->next); 
        second->next->prev = second; 
        second->prev = NULL; 
        return second; 
    }

} 

// merge sort
static ListElement *dlist_merge_sort (ListElement *head, int (*compare)(void *one, void *two)) {

    if (!head || !head->next) return head;

    ListElement *second = dllist_split (head);

    // recursivly sort each half
    head = dlist_merge_sort (head, compare);
    second = dlist_merge_sort (second, compare);

    // merge the two sorted halves 
    return dllist_merge (compare, head, second);

}

int dlist_sort (DoubleList *list) {

    int retval = 1;

    if (list && list->compare) {
        list->start = dlist_merge_sort (list->start, list->compare);
        retval = 0;
    }

    return retval;

}