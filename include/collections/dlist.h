#ifndef _DLIST_H_
#define _DLIST_H_

#include <stdlib.h>
#include <stdbool.h>

typedef struct ListElement {

    struct ListElement *prev;
    void *data;
    struct ListElement *next;

} ListElement;

typedef struct List {

    size_t size;

    ListElement *start;
    ListElement *end;

    void (*destroy)(void *data);
    int (*compare)(void *one, void *two);

} DoubleList;

#define dlist_size(list) ((list)->size)

#define dlist_start(list) ((list)->start)
#define dlist_end(list) ((list)->end)

#define dlist_element_data(element) ((element)->data)
#define dlist_element_next(element) ((element)->next)

// sets a list compare function
// compare must return -1 if one < two, must return 0 if they are equal, and must return 1 if one > two
extern void dlist_set_compare (DoubleList *list, int (*compare)(void *one, void *two));

// sets list destroy function
extern void dlist_set_destroy (DoubleList *list, void (*destroy)(void *data));

// compare must return -1 if one < two, must return 0 if they are equal, and must return 1 if one > two
extern DoubleList *dlist_init (void (*destroy)(void *data),
    int (*compare)(void *one, void *two));
extern void dlist_reset (DoubleList *);
// only gets rid of the List elemenst, but the data is kept
extern void dlist_clean (DoubleList *);
extern void dlist_destroy (DoubleList *);

// Elements
extern bool dlist_insert_after (DoubleList *, ListElement *, void *data);
extern void *dlist_remove_element (DoubleList *, ListElement *);

// Searching
extern void *dlist_search (DoubleList *, void *data);
extern bool dlist_is_in_list (DoubleList *, void *data);
ListElement *dlist_get_element (DoubleList *, void *data);

// merge sort
// return 0 on succes 1 on error
extern int dlist_sort (DoubleList *list);

#endif