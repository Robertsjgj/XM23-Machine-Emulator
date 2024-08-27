;manually set the current priority to 7, so program starts with priority 7
	SETPRI	#3
	SETPRI	#5
	SETPRI	#0
	SETCC	S
	MOVLZ	#37,R0; set current priority of ISR vector to 1. Set all psw status except sleep
	MOVL	#1000,R3; set vector entry point for handler to address 0x1000
	MOVH	#1000,R3
	MOVL	#FFC4,R1; get address of vector 1 psw status
	MOVH	#FFC4,R1
	MOVL	#FFC6,R2; get address of vector 1 entry point to handler
	MOVH	#FFC6,R2
	ST	R0,R1; store psw status of handler in vector 1
	ST	R3,R2; store address of entry point to handler in vector 1 entry point to handler position in memory 
	SVC 	#1; trap to vector 1
	MOVLZ	#17,R0; set current priority of ISR vector to 1. Set all psw status except sleep
	MOVL	#1000,R3; set vector entry point for handler to address 0x1000
	MOVH	#1000,R3
	MOVL	#FFC4,R1; get address of vector 1 psw status
	MOVH	#FFC4,R1
	MOVL	#FFC6,R2; get address of vector 1 entry point to handler
	MOVH	#FFC6,R2
	ST	R0,R1; store psw status of handler in vector 1
	ST	R3,R2; store address of entry point to handler in vector 1 entry point to handler position in memory 
	SVC 	#1; trap to vector 1	
	BRA	DONE
DONE	BRA	DONE