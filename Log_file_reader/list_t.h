#ifndef list_t_h
#define list_t_h

#include <stdio.h>
#include <stdint.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>

/* Exit codes */
#define ERROR       -1
#define SUCCESS     0

/* a node struct */
typedef struct node_t
{
    /* String of information */
    char* szLine;
    /* Pointers to next and previous node_t element */
    struct node_t *pnNext, *pnPrev;
} node_t;

/* linked list */
typedef struct list
{
    /* Nodes counter for fast check */
    int iNodes;
    /* pointers to Hed and tail of the list */
    struct node_t *pnHead, *pnTail;
} list_t;

/* Create new node_t from string */
node_t* node_init(char *string);

/* Destroyng the node */
int destroy_node(node_t *pnNode);

/* List_t initialisation*/
list_t* list_init();

/* Destroy list_t*/
int list_destroy(list_t *plList);

/* Adding new node_t to the end of the list_t */
int list_tail_add(list_t *plList, node_t *pnNode);

/* Adding new node_t to the top of the list_t */
int list_top_add(list_t *plList, node_t *pnNode);

/* Getting node from tail */
node_t* get_node_top(list_t *plList);

#endif /* list_t_h */
