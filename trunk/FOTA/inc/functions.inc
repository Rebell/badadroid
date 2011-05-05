;some functions built by Rebellos and b.kubica
;if adding new please make them model-independent
macro FUNCTIONS
{

;void dloadmode()
;prints info message to current debug interface and enters downloading mode
dloadmode:
	ldr	r0, [s_downloadmode_a]
	bl	debug_print

	mov	r1, 0
	mov	r0, 2
	bl	maxim_charging_control
	bl	DloadMain

;void debug_print(char* fmt, ...)
debug_print:
	STMFD	SP!, {LR\}
	LDR	R10, [output_switch]
	CMP	R10, 1
	BEQ	debug_print_fota
	CMP	R10, 2
	BEQ	debug_print_uart
	B	debug_print_ret
debug_print_fota:
	BL	disp_FOTA_Printf
	mov	r4, 0
	ldr	r3, [fota_x]
	ldr	r3, [r3]
	cmp	r3, 20
	BLE	debug_print_ret

	ldr	r0, [v_delay_800ms]  ;wait 0.8s so we we can read the text
	bl	PWM_Drv_Delay
	ldr	r3, [fota_x]
	str	r4, [r3]	    ;reset fota_printf_x position counter

	B	debug_print_ret
debug_print_uart:
	BL	EdbgOutputDebugString
	LDR	r0, [s_uart_newline_a]
	BL	EdbgOutputDebugString
	B	debug_print_ret
debug_print_ret:
	LDMFD	SP!, {PC\}

;void enable_fota_output()
enable_fota_output:
	STMFD	SP!, {R0-R1,LR\}
	ADR	r1, output_switch
	MOV	r0, 1
	STR	R0, [r1]
	BL	disp_FOTA_Init
	ldr	r0, [s_fotaoutput_a]
	bl	debug_print
	LDMFD	SP!, {R0-R1,PC\}

;void enable_uart_output()
enable_uart_output:
	STMFD	SP!, {R0-R1,LR\}
	ADR	r1, output_switch
	MOV	r0, 2
	STR	R0, [r1]
	ldr	r0, [s_uartoutput_a]
	bl	debug_print
	LDMFD	SP!, {R0-R1,PC\}

;void int_debugprint(int num)
int_debugprint:
	STMFD	SP!, {R1,LR\}
	mov	r1, r0
	ldr	r0, [s_debug_fmt_a]
	bl	debug_print
	LDMFD	SP!, {R1,PC\}

;int loadfile(wchar* filepath, ulong startaddr, ulong size)
;loads file to RAM under specific address
;returns bytes read or 0 if error
loadfile:
	STMFD	SP!,{R3-R6,LR\}
	mov	r4, r1
	mov	r5, r2

	mov	r1, 4 ;flag, probably READ or READWRITE
	mov	r0, r0	;path
	bl	tfs4_open
	mov	r6, r0
	cmp	r0, 0	;is it FILE index or -1?
	blt	loadfile_notfound

	mov	r0, 1

	mov	r0, r6

	mov	r2, r5	 ;len
	mov	r1, r4	 ;buffer
	mov	r0, r6	 ;FILE index
	bl	tfs4_read
	mov	r4, r0

	mov	r0, 2

	mov	r0, r4

	mov	r0, r6
	bl	tfs4_close
	mov	r0, r4
loadfile_ret:
	LDMFD	SP!, {R3-R6,PC\}
loadfile_notfound:
	mov	r0, 0
	b	loadfile_ret


;int fota_listdir(wchar* path)
;auto reset
;returns 0 and print FOTA message if error occured/dir not found, otherwise returns 1 (even if dir is empty)
listdir:
	STMFD	SP!, {R1-R5,LR\}
	mov	r1, 0xC
	mov	r0, r0
	bl	tfs4_opendir
	cmp	r0, 0
	beq	listdir_notfound  ;tfs4_opendir returns 0 if something went wrong
	mov	r5, r0	 ;store *DIR

	mov r4, 0
listdir_loop:
	mov	r0, r5
	bl	tfs4_readdir
	cmp	r0, 0
	beq	listdir_end ;EOF

	;lets convert wcstr to cstr
	mov	r1, 2
	mov	r2, 1

listdir_convertloop:
	ldrb	r3, [r0, r1]
	strb	r3, [r0, r2]

	add	r1, 2
	add	r2, 1

	ldrb	r3, [r0, r1]
	cmp	r3, 0x00

	bne	listdir_convertloop
	strb	r3, [r0, r2] ;make sure str is zero-ending


	mov	r1, r0
	ldr	r0, [s_fmt_a]
	bl	debug_print


	ldr	r0, [v_delay_100ms]  ;pause to make it easy readable
	bl	PWM_Drv_Delay

	b	listdir_loop
listdir_end:
	mov	r0, r5
	bl	tfs4_closedir
	mov	r0, 1
	b	listdir_ret
listdir_notfound:
	ldr	r0, [s_notfound_a]
	bl	debug_print
	mov	r0, 0
	b	listdir_ret
listdir_ret:
	LDMFD	SP!, {R1-R5,PC\}
}