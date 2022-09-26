//**************************************************************************************************
// @Module        Circular buffer
// @Filename      circ_buffer.c
//--------------------------------------------------------------------------------------------------
// @Platform      Independent
//--------------------------------------------------------------------------------------------------
// @Compatible    Independent
//--------------------------------------------------------------------------------------------------
// @Description   Implementation of the Circular buffer functionality.
//                Put and get operations are mutually independent.
//
//                Abbreviations:
//                  None
//
//                Global (public) functions:
//                  CIRCBUF_Init()
//                  CIRCBUF_PutData()
//                  CIRCBUF_GetData()
//                  CIRCBUF_Purge()
//                  CIRCBUF_GetNumberOfItems()
//                  CIRCBUF_GetFreeSize()
//
//                Local (private) functions:
//                  None
//
//--------------------------------------------------------------------------------------------------
// @Version       1.3.1
//--------------------------------------------------------------------------------------------------
// @Date          06.04.2016
//--------------------------------------------------------------------------------------------------
// @History       Version  Author    Comment
//                1.0.0    ASD       First release.
//                1.0.1    ASD       Adapted for a lot of MISRA-C 2004 rules.
// 16.04.2012     1.0.2    SRM       Circular buffer adapted to use it with any other modules
// 19.04.2012     1.0.3    SRM       An error has been eliminated in the data buffer
// 11.06.2012     1.0.4    SRM       Now the first call of the CIRCBUF_PutData() puts the first element 
//                                   into the [0] circular buffer element, not in [1], like before.
// 22.06.2012     1.1.0    SRM       A Serious problem with the signed variables has been eliminated:
//                                   (S32)(pBuf->head - pBuf->tail) -> (S32)((S32)pBuf->head - (S32)pBuf->tail)
// 20.04.2014     1.2.0    SRM       Error in CIRCBUF_GetFreeSize() has been removed: when there is
//                                   no free items in the buffer, CIRCBUF_GetFreeSize() returns "1".
// 08.02.2016     1.3.0    ASD       Bug with null pointers fixed. Formatting improved.
// 06.04.2016     1.3.1    SRM       Adopted to C89 object definition rules.
//**************************************************************************************************



//**************************************************************************************************
// Project Includes
//**************************************************************************************************

// Native header
#include "circ_buffer.h"



//**************************************************************************************************
// Verification of the imported configuration parameters
//**************************************************************************************************

// None



//**************************************************************************************************
// Definitions of global (public) variables
//**************************************************************************************************

// None



//**************************************************************************************************
// Declarations of local (private) data types
//**************************************************************************************************

// None



//**************************************************************************************************
// Definitions of local (private) constants
//**************************************************************************************************

// The minimum size of a circular buffer in items
#define CIRCBUF_SIZE_MIN  (2U)



//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

// None



//**************************************************************************************************
// Declarations of local (private) functions
//**************************************************************************************************

// None



//**************************************************************************************************
//==================================================================================================
// Definitions of global (public) functions
//==================================================================================================
//**************************************************************************************************



//**************************************************************************************************
// @Function      CIRCBUF_Init()
//--------------------------------------------------------------------------------------------------
// @Description   Initializes a circular buffer. Size of the buffer must be more than one item,
//                an upper limit of stored items is less by one than size of the buffer.
//--------------------------------------------------------------------------------------------------
// @Notes         This function is not reentrant (accesses to global data). That is why
//                only transactional callings are allowed.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   CIRCBUF_NOT_EXIST - buffer does not exist;
//                CIRCBUF_DATA_NOT_EXIST - data of the buffer do not exist;
//                CIRCBUF_SIZE_TOO_SMALL - size of the buffer is too small;
//                CIRCBUF_NO_ERR - no error occurred.
//--------------------------------------------------------------------------------------------------
// @Parameters    pBuf - pointer to a buffer to be initialized;
//                pData - pointer to data for the buffer;
//                size - size of the buffer in items.
//**************************************************************************************************
enCIRCBUF_ERR CIRCBUF_Init(stCIRCBUF* const pBuf,
                           const void* const pData,
                           const CIRCBUF_SZ size)
{
    // Check if the buffer is available
    if (0 == pBuf)
    {
        return CIRCBUF_NOT_EXIST;
    }

    // Check if data for the buffer is available
    if (0 == pData)
    {
        return CIRCBUF_DATA_NOT_EXIST;
    }

    // Check if the size of the buffer is valid
    if (size < CIRCBUF_SIZE_MIN)
    {
        return CIRCBUF_SIZE_TOO_SMALL;
    }

    // Initialize pointer to data for the buffer and set initial values for the
    // size, head and tail of the buffer
    pBuf->pData = pData;
    pBuf->size = size;
    pBuf->head = size - 1;
    pBuf->tail = size - 1;
	pBuf->itemSize = 1;
    return CIRCBUF_NO_ERR;
} // end of CIRCBUF_Init()



//**************************************************************************************************
// @Function      CIRCBUF_PutData()
//--------------------------------------------------------------------------------------------------
// @Description   Puts one item into a circular buffer. If the buffer is not full, the newest item
//                is written into the the head of the buffer. In order to provide mutual independence
//                of the operations CIRCBUF_PutData() and CIRCBUF_GetData(), all operations with the
//  head of the buffer (loopback and writing the next item) are carried out in the local variable
//  "headPlus". The next value of the head will be set only when data have been written into the buffer.
//--------------------------------------------------------------------------------------------------
// @Notes         This function is not reentrant (accesses to global data). That is why
//                only transactional callings are allowed.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   CIRCBUF_NOT_EXIST - buffer does not exist;
//                CIRCBUF_DATA_NOT_EXIST - data of the buffer do not exist;
//                CIRCBUF_OVERFLOW - buffer overflow;
//                CIRCBUF_NO_ERR - no error occurred.
//--------------------------------------------------------------------------------------------------
// @Parameters    pData - pointer to the item that will be put into the buffer;
//                pBuf - pointer to a buffer.
//**************************************************************************************************
enCIRCBUF_ERR CIRCBUF_PutData(const void* const pData,
                              stCIRCBUF* const pBuf)
{
    CIRCBUF_SZ headPlus = 0;
    uint8_t i = 0;

    // Check if the item is available
    if (0 == pData)
    {
        return CIRCBUF_DATA_NOT_EXIST;
    }

    // Check if the buffer is available
    if (0 == pBuf)
    {
        return CIRCBUF_NOT_EXIST;
    }

    headPlus = 1 + pBuf->head;

    // If the head is reached the size, loopback the buffer
    if (headPlus == pBuf->size)
    {
        headPlus = 0;
    }

    // Check if free space is present in the buffer
    if (headPlus == pBuf->tail)
    {
        return CIRCBUF_OVERFLOW;
    }

    // Put one item into the head of the buffer
    for (i = 0 ; i < pBuf->itemSize; i++)
    {
        *(((uint8_t*)pBuf->pData + (headPlus * pBuf->itemSize) + i)) = *((uint8_t*)pData + i);
    }

    // Set the next value of the head
    pBuf->head = headPlus;

    return CIRCBUF_NO_ERR;
} // end of CIRCBUF_PutData()



//**************************************************************************************************
// @Function      CIRCBUF_GetData()
//--------------------------------------------------------------------------------------------------
// @Description   Gets and removes one item from a circular buffer. If data is present in the buffer,
//                the oldest item is taken out from the tail of the buffer and written into output
//                variable. In order to provide mutual independence of the operations CIRCBUF_PutData()
//  and CIRCBUF_GetData(), all operation with the tail of the buffer (loopback and reading the next item)
//  are carried out in the local variable "tailPlus". The next value of the tail will be set only when
//  corresponding item of the buffer have been released.
//--------------------------------------------------------------------------------------------------
// @Notes         This function is not reentrant (accesses to global data). That is why
//                only transactional callings are allowed.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   CIRCBUF_NOT_EXIST - buffer does not exist;
//                CIRCBUF_DATA_NOT_EXIST - data of the buffer do not exist;
//                CIRCBUF_EMPTY - buffer is empty;
//                CIRCBUF_NO_ERR - no error occurred.
//--------------------------------------------------------------------------------------------------
// @Parameters    pData - pointer to the place where item that extracted from the buffer will be stored;
//                pBuf - pointer to a buffer.
//**************************************************************************************************
enCIRCBUF_ERR CIRCBUF_GetData(const void* const pData,
                              stCIRCBUF* const pBuf)
{
    CIRCBUF_SZ tailPlus = 0;
    uint8_t i = 0;

    // Check if the item is available
    if (0 == pData)
    {
        return CIRCBUF_DATA_NOT_EXIST;
    }

    // Check if the buffer is available
    if (0 == pBuf)
    {
        return CIRCBUF_NOT_EXIST;
    }

    tailPlus = 1 + pBuf->tail;

    // Check if data is present in the buffer
    if (pBuf->tail == pBuf->head)
    {
        return CIRCBUF_EMPTY;
    }

    // If the tail is reached the size, loopback the buffer
    if (tailPlus == pBuf->size)
    {
        tailPlus = 0;
    }

    // Get the next item from the tail of the buffer
    for (i = 0 ; i < pBuf->itemSize; i++)
    {
        *(((uint8_t*)pData + i)) = *(((uint8_t*)pBuf->pData + (tailPlus * pBuf->itemSize) + i));
    }
    
    // Set the next value of the tail
    pBuf->tail = tailPlus;

    return CIRCBUF_NO_ERR;
} // end of CIRCBUF_GetData()



//**************************************************************************************************
// @Function      CIRCBUF_Purge()
//--------------------------------------------------------------------------------------------------
// @Description   Cleans a circular buffer.
//--------------------------------------------------------------------------------------------------
// @Notes         This function is not reentrant (accesses to global data). That is why
//                only transactional callings are allowed.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   CIRCBUF_NOT_EXIST - buffer does not exist;
//                CIRCBUF_NO_ERR - no error occurred.
//--------------------------------------------------------------------------------------------------
// @Parameters    pBuf - pointer to a buffer.
//**************************************************************************************************
enCIRCBUF_ERR CIRCBUF_Purge(stCIRCBUF* const pBuf)
{
    // Check if the buffer is available
    if (0 == pBuf)
    {
        return CIRCBUF_NOT_EXIST;
    }

    // Clean the buffer
    pBuf->tail = pBuf->head;

    return CIRCBUF_NO_ERR;
} // end of CIRCBUF_Purge()



//**************************************************************************************************
// @Function      CIRCBUF_GetNumberOfItems()
//--------------------------------------------------------------------------------------------------
// @Description   Calculates the number of items in a circular buffer. In order to provide
//                signal-safety from the operations CIRCBUF_PutData() and CIRCBUF_GetData(), the head
//                and tail difference is assigned to the signed local variable "numBusyElements".
//--------------------------------------------------------------------------------------------------
// @Notes         This function is not reentrant (accesses to global data). That is why
//                only transactional callings are allowed.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   0 - buffer does not exist or empty;
//                [1..65534] - number of busy elements in a buffer.
//--------------------------------------------------------------------------------------------------
// @Parameters    pBuf - pointer to a buffer.
//**************************************************************************************************
CIRCBUF_SZ CIRCBUF_GetNumberOfItems(const stCIRCBUF* const pBuf)
{
    int numBusyElements = 0;

    // Check if the buffer is available
    if (0 == pBuf)
    {
        return 0;
    }

    numBusyElements = (int)((int)pBuf->head - (int)pBuf->tail);

    // If loopback occurred and the tail has not reached the end of the buffer yet, the head and tail
    // difference is negative, so the difference and the size are summed up to calculate the number of
    // busy elements correctly.
    if (numBusyElements < 0)
    {
        numBusyElements += pBuf->size;
    }

    return (CIRCBUF_SZ)numBusyElements;
} // end of CIRCBUF_GetNumberOfItems()



//**************************************************************************************************
// @Function      CIRCBUF_GetFreeSize()
//--------------------------------------------------------------------------------------------------
// @Description   Calculates the free number of elements in a circular buffer. In order to provide
//                signal-safety from the operations CIRCBUF_PutData() and CIRCBUF_GetData(), the head
//                and tail difference is assigned to the signed local variable "numBusyElements".
//--------------------------------------------------------------------------------------------------
// @Notes         This function is not reentrant (accesses to global data). That is why
//                only transactional callings are allowed.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   0 - buffer does not exist;
//                [1..65534] - number of free elements in a buffer.
//--------------------------------------------------------------------------------------------------
// @Parameters    pBuf - pointer to a circular buffer.
//**************************************************************************************************
CIRCBUF_SZ CIRCBUF_GetFreeSize(const stCIRCBUF* const pBuf)
{
    int numBusyElements = 0;

    // Check if the buffer is available
    if (0 == pBuf)
    {
        return 0;
    }

    numBusyElements = (int)((int)pBuf->head - (int)pBuf->tail);

    // If loopback occurred and the tail has not reached the end of the buffer yet, the head and tail
    // difference is negative, so the difference and the size are summed up to calculate the number of
    // busy elements correctly.
    if (numBusyElements < 0)
    {
        numBusyElements += pBuf->size;
    }

    return (CIRCBUF_SZ)((pBuf->size - (CIRCBUF_SZ)numBusyElements) - 1);
} // end of CIRCBUF_GetFreeSize()



//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************



// None



//****************************************** end of file *******************************************
