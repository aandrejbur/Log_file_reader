#include "list_t.h"

/* Create new node_t from string */
node_t *node_init(char *string)
{
    if (string!=NULL)
    {
        node_t *pnNode = malloc(sizeof(node_t));
        pnNode->pnNext = pnNode->pnPrev = NULL;
        pnNode->szLine = malloc(strlen(string)+1);
        strcpy(pnNode->szLine, string);
        return pnNode;
    }
    else
    {
        return NULL;
    }
}
/* Destroyng the node */
int destroy_node(node_t *pnNode)
{
    if (pnNode == NULL)
    {
        return ERROR;
    }
    pnNode->pnPrev = pnNode->pnNext = NULL;
    /* Free the line with text */
    free(pnNode->szLine);
    pnNode->szLine = NULL;
    free(pnNode);
    pnNode = NULL;
    return SUCCESS;
}

/* List_t initialisation*/
list_t* list_init()
{
    list_t *plTemp = malloc(sizeof(list_t));
    plTemp->pnTail = plTemp->pnHead = NULL;
    return plTemp;
}
/* Destroy list_t*/
int list_destroy(list_t *plList)
{
    node_t *pnTemp = NULL;
    if (plList == NULL)
    {
        return ERROR;
    }
    while (plList->pnHead!=NULL) {
        pnTemp = plList->pnHead->pnNext;
        destroy_node(plList->pnHead);
        plList->pnHead = pnTemp;
    }
    free(plList);
    plList=NULL;
    return SUCCESS;
}
/* Adding node to the end of the list_t */
int list_tail_add(list_t *plList, node_t *pnNode)
{
    if ( (plList==NULL) || (pnNode==NULL) ) {
        return ERROR;
    }
    if (plList->iNodes == 0)
    {
        plList->pnHead = plList->pnTail = pnNode;
        plList->iNodes++;
    }
    else
    {
        plList->pnTail->pnNext = pnNode;
        pnNode->pnPrev = plList->pnTail;
        plList->pnTail = pnNode;
        plList->iNodes++;
    }
    return SUCCESS;
}

/* Adding new node_t to the top of the list_t */
int list_top_add(list_t *plList, node_t *pnNode)
{
    if ( (plList==NULL) || (pnNode==NULL) ) {
        return ERROR;
    }
    if (plList->iNodes == 0)
    {
        plList->pnHead = plList->pnTail = pnNode;
        plList->iNodes++;
    }
    else
    {
        plList->pnHead->pnPrev = pnNode;
        pnNode->pnNext = plList->pnHead;
        plList->pnHead = pnNode;
        plList->iNodes++;
    }
    return SUCCESS;
}

node_t* get_node_top(list_t *pstlist)
{
    node_t *pnTemp = NULL;
    if (pstlist->iNodes == 0)
    {
        return NULL;
    }
    else if (pstlist->iNodes ==1)
    {
        pnTemp = pstlist->pnHead;
        pstlist->pnHead = pstlist->pnTail = NULL;
        pstlist->iNodes--;
        return pnTemp;
    }
    else
    {
        pnTemp = pstlist->pnHead;
        
        pnTemp->pnNext->pnPrev = NULL;
        pstlist->pnHead = pnTemp->pnNext;
        pnTemp->pnNext = NULL;
        pstlist->iNodes--;
        return pnTemp;
    }
}
