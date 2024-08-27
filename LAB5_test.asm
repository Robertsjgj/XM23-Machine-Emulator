; Start of data area
;
    org     #800
Array   word    $1
        word    $2
        word    $-1
        word    $3     ; Add more elements to the array if needed
        word    $-2
        word    $4
        word    $-3
        word    $5
        word    $-4
        word    $6
        word    $-5
	word	$0
;
; Start of code area
;
    org     #1000
;
Start   movl    Array,R1   ; R1 = LSByte(Array)
        movh    Array,R1   ; R1 = MSByte(Array)
        movlz   $0,R0     ; Initialize R0 to 0 for comparison
        movlz   $2,R3     ; Initialize R3 to 0 for accumulation
	movlz	$0,R4
;
	SETCC 	VSNZC
	CLRCC	VSNZC
;
Loop    ld.w    R1+,R2     ; R2 = [R1]; R1 = R1 + 2
;
        cmp     $0,R2
        beq     Done    ; Branch to Done if R2 is 0
        ; Check if R2 < 0
        cmp     R0,R2
        cex     PL,$2,$1   ; Branch to Done if negative
;
        ; Add or subtract based on the condition
        add     R2,R3      ; Add R2 to R3
        add     $-1,R3      ; Subtract 1 from R2
;
        sub     R2,R2       
	bra     Loop       ; Repeat loop
;
; End of program
;
Done    bra     Done       ; Terminate the program

    end     Start   ; End of program - first executable address is "Start"
