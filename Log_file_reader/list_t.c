#include "list_t.h"
#include "includes.h"
/* Create new node_t from string */
node_t *node_init(char *string)
{
    if (string!=NULL)
    {
        node_t *pnNode = malloc(sizeof(node_t));
        pnNode->pnNext = pnNode->pnPrev = NULL;
        pnNode->szLine = malloc(strlen(string)+3);
        strlcpy_udev(pnNode->szLine, string,strlen(string)+1);
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
    plTemp->iNodes = 0;
    return plTemp;
}
/* Destroy list_t*/
int list_destroy(list_t *plList)
{
    if (plList == NULL)
    {
        return ERROR;
    }
    node_t *pnTemp = NULL;
    while (plList->pnHead!=NULL) {
        pnTemp = plList->pnHead->pnNext;
        destroy_node(plList->pnHead);
        plList->pnHead = pnTemp;
    }
    free(plList);
    plList =NULL;
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

node_t* get_node_top(list_t *plList)
{
    node_t *pnTemp = NULL;
    if (plList->iNodes == 0)
    {
        return NULL;
    }
    else if (plList->iNodes ==1)
    {
        pnTemp = plList->pnHead;
        plList->pnHead = plList->pnTail = NULL;
        plList->iNodes--;
        return pnTemp;
    }
    else
    {
        pnTemp = plList->pnHead;
        pnTemp->pnNext->pnPrev = NULL;
        plList->pnHead = pnTemp->pnNext;
        pnTemp->pnNext = NULL;
        plList->iNodes--;
        return pnTemp;
    }
}
