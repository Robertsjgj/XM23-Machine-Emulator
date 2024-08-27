#include "Emulator.h"

volatile sig_atomic_t ctrl_c_fnd;

/************ Debugger support function ************/
void sigint_hdlr()
{
    /*
    - Invoked when SIGINT (control-C) is detected
    - changes state of waiting_for_signal
    - signal must be reinitialized
    */
    ctrl_c_fnd = TRUE;
    signal(SIGINT, (_crt_signal_t)sigint_hdlr); /* Reinitialize SIGINT */
}

/************ Debugger startup software ************/

/* Call signal() - bind sigint_hdlr to SIGINT */
void initialize_signal_handler() {
    /* Call signal() - bind sigint_hdlr to SIGINT */
    ctrl_c_fnd = FALSE;
    signal(SIGINT, (_crt_signal_t)sigint_hdlr); /* Initialize SIGINT */
}


/************ Before calling the CPU emulator *************/

void run_xm()
{
    /* Run the CPU */
    ctrl_c_fnd = FALSE;
    cpu();
}





