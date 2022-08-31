//**************************************************************************************************
// @Module        Memory types
// @Filename      memory_types.h
//--------------------------------------------------------------------------------------------------
// @Description   Interface of the Memory types.
//--------------------------------------------------------------------------------------------------
// @Version       
//--------------------------------------------------------------------------------------------------
// @Date          
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment

//**************************************************************************************************

#ifndef MEMORY_TYPES_H
#define MEMORY_TYPES_H



//**************************************************************************************************
// Project Includes
//**************************************************************************************************

// None.



//**************************************************************************************************
// Declarations of global (public) data types
//**************************************************************************************************

// End of memory job callback
typedef void (*MEM_END_OF_JOB_CALLBACK)(const U8 nEventID,
                                        const U8 nJobResult);



//**************************************************************************************************
// Definitions of global (public) constants
//**************************************************************************************************

// Memory job possible results
#define MEM_JOB_RESULT_OK         (0U)
#define MEM_JOB_RESULT_NOT_OK     (1U)
#define MEM_JOB_RESULT_PENDING    (2U)

// Memory job callbacks
#define MEM_JOB_CALLBACK_READ     (0U)
#define MEM_JOB_CALLBACK_LOAD     (0U)
#define MEM_JOB_CALLBACK_WRITE    (1U)
#define MEM_JOB_CALLBACK_STORE    (1U)
#define MEM_JOB_CALLBACK_ERASE    (2U)

// Memory statuses
#define MEM_STATUS_READ_ERR_MASK          (ON << 0U)
#define MEM_STATUS_WRITE_ERR_MASK         (ON << 1U)
#define MEM_STATUS_ERASE_ERR_MASK         (ON << 2U)
#define MEM_STATUS_RECOVER_WARN_MASK      (ON << 3U)
#define MEM_STATUS_NON_RECOVER_ERR_MASK   (ON << 4U)
#define MEM_STATUS_WEAR_THRS_1_WARN_MASK  (ON << 5U)
#define MEM_STATUS_WEAR_THRS_2_ERR_MASK   (ON << 6U)



//**************************************************************************************************
// Declarations of global (public) variables
//**************************************************************************************************

// None.



//**************************************************************************************************
// Declarations of global (public) functions
//**************************************************************************************************

// None.



#endif // #ifndef MEMORY_TYPES_H

//****************************************** end of file *******************************************
