//**************************************************************************************************
// @Module        TERMINAL
// @Filename      terminal_cfg.h
//--------------------------------------------------------------------------------------------------
// @Description   Configuration of the required functionality of the MODULE module.
//                [Description...]
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          XX.XX.XXXX
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    XXX         First release.
//**************************************************************************************************

#ifndef TERMINAL_CFG_H
#define TERMINAL_CFG_H



//**************************************************************************************************
// Definitions of global (public) constants
//**************************************************************************************************

// Prm vTaskTerminal
#define TASK_TERMINAL_STACK_DEPTH          (512U)
#define TASK_TERMINAL_PARAMETERS           (NULL)
#define TASK_TERMINAL_PRIORITY             (1U)

// timeout
#define TASK_TERMINAL_TIMEOUT              (30U * 1000U)
#define TASK_TERMINAL_DELAY                (10U)

#endif // #ifndef TERMINAL_CFG_H

//****************************************** end of file *******************************************

