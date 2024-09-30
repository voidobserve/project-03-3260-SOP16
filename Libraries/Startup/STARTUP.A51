$NOMOD51
;------------------------------------------------------------------------------
;  This file is part of the C51 Compiler package
;  C51编译器包
;
;  Copyright (c) 1988-2005 Keil Elektronik GmbH and Keil Software, Inc.
;  Version 8.01
;
;  *** <<< Use Configuration Wizard in Context Menu >>> ***
;  在上下文菜单中使用配置向导
;------------------------------------------------------------------------------
;  STARTUP.A51:  This code is executed after processor reset.
;  这份文件的代码在处理器复位后被执行
;
;  To translate this file use A51 with the following invocation:
;  要转换该文件，请使用A51和以下调用
;
;     A51 STARTUP.A51
;
;  To link the modified STARTUP.OBJ file to your application use the following
;  Lx51 invocation:
;  链接修改后的文件到应用程序，使用以下Lx51调用
;
;     Lx51 your object file list, STARTUP.OBJ  controls
;
;------------------------------------------------------------------------------
;
;  User-defined <h> Power-On Initialization of Memory
;  用户自定义的上电初始化内存
;
;  With the following EQU statements the initialization of memory
;  at processor reset can be defined:
;  使用以下EQU语句，可以定义处理器复位时内存的初始化
;
; <o> IDATALEN: IDATA memory size <0x0-0x100>   IDATA内存大小
;     <i> Note: The absolute start-address of IDATA memory is always 0      注意：IDATA的绝对起始地址永远是0
;     <i>       The IDATA space overlaps physically the DATA and BIT areas. IDATA空间在物理上与DATA和BIT区域重叠
IDATALEN        EQU     80H
;
; <o> XDATASTART: XDATA memory start address <0x0-0xFFFF>   XDATA内存起始地址
;     <i> The absolute start address of XDATA memory        XDATA内存的绝对起始地址
XDATASTART      EQU     0x6000     
;
; <o> XDATALEN: XDATA memory size <0x0-0xFFFF>      XDATA内存的大小    
;     <i> The length of XDATA memory in bytes.      XDATA内存的长度以字节为单位
XDATALEN        EQU     0x0700      
;
; <o> PDATASTART: PDATA memory start address <0x0-0xFFFF>       PDATA内存起始地址
;     <i> The absolute start address of PDATA memory            PDATA内存绝对起始地址
PDATASTART      EQU     0H
;
; <o> PDATALEN: PDATA memory size <0x0-0xFF>        PDATA内存大小
;     <i> The length of PDATA memory in bytes.      PDATA内存的大小以字节为单位
PDATALEN        EQU     0H
;
;</h>
;------------------------------------------------------------------------------
;
;<h> Reentrant Stack Initialization     可重入堆栈初始化
;
;  The following EQU statements define the stack pointer for reentrant
;  functions and initialized it:
;  下面的EQU语句定义的可重入函数的堆栈指针并对其进行了初始化
;
; <h> Stack Space for reentrant functions in the SMALL model.           SMALL模型中可重入函数的堆栈空间
;  <q> IBPSTACK: Enable SMALL model reentrant stack                         IBPSTACK:使能SMALL模型可重入堆栈
;     <i> Stack space for reentrant functions in the SMALL model.               SMALL模型中看重入函数的堆栈空间
IBPSTACK        EQU     0       ; set to 1 if small reentrant is used.      如果使用small可重入，设置为1
;  <o> IBPSTACKTOP: End address of SMALL model stack <0x0-0xFF>     IBPSTACKTOP：SMALL模型栈的结束地址
;     <i> Set the top of the stack to the highest location.             将堆栈顶部设置为最高的位置（堆栈大小的上限）
IBPSTACKTOP     EQU     0xFF +1     ; default 0FFH+1  
; </h>
;
; <h> Stack Space for reentrant functions in the LARGE model.               LARGE模型中可重入函数的堆栈空间
;  <q> XBPSTACK: Enable LARGE model reentrant stack                             SBPSTACK:使能LARGE模型的可重入堆栈
;     <i> Stack space for reentrant functions in the LARGE model.                   LARGE模型中的可重入函数的堆栈空间
XBPSTACK        EQU     0       ; set to 1 if large reentrant is used.      如果在LARGE可重入，设置为1
;  <o> XBPSTACKTOP: End address of LARGE model stack <0x0-0xFFFF>       XBPSTACKTOP:LARGE模型栈的结束地址
;     <i> Set the top of the stack to the highest location.                  将堆栈顶部设置为最高的位置（堆栈大小的上限）
XBPSTACKTOP     EQU     0xFFFF +1   ; default 0FFFFH+1 
; </h>
;
; <h> Stack Space for reentrant functions in the COMPACT model.             COMPACT模型中的可重入函数的栈空间
;  <q> PBPSTACK: Enable COMPACT model reentrant stack                           PBPSTACK: 使能COMPACT模型的可重入栈
;     <i> Stack space for reentrant functions in the COMPACT model.                 COMPACT模型中的可重入函数的栈空间
PBPSTACK        EQU     0       ; set to 1 if compact reentrant is used.    如果在COMPACT可重入，设置为1
;
;   <o> PBPSTACKTOP: End address of COMPACT model stack <0x0-0xFFFF>        PBPSTACKTOP: COMPACT模栈的结束地址
;     <i> Set the top of the stack to the highest location.                     将堆栈顶部设置为最高的位置（堆栈大小的上限）
PBPSTACKTOP     EQU     0xFF +1     ; default 0FFH+1  
; </h>
;</h>
;------------------------------------------------------------------------------
;
;  Memory Page for Using the Compact Model with 64 KByte xdata RAM          使用具有64K字节xdata内存的COMPACT模型（紧凑模型）的内存页
;  <e>Compact Model Page Definition                                             COMPACT模型页的定义
;   
;  <i>Define the XDATA page used for PDATA variables.                           定义用于PDATA变量的XDATA页
;  <i>PPAGE must conform with the PPAGE set in the linker invocation.           PPAGE必须与链接器调用中设置的PPAGE一致
;
; Enable pdata memory page initalization    启动pdata内存页初始化
PPAGEENABLE     EQU     0       ; set to 1 if pdata object are used.    如果使用pdata对象，则设置为1
;
; <o> PPAGE number <0x0-0xFF>       PPAGE页码
; <i> uppermost 256-byte address of the page used for PDATA variables.  用于PDATA变量的页的最上端的256字节地址
PPAGE           EQU     0
;
; <o> SFR address which supplies uppermost address byte <0x0-0xFF>      提供最高地址字节的SFR地址
; <i> most 8051 variants use P2 as uppermost address byte               大多数8051变体使用P2作为最高地址字节
PPAGE_SFR       DATA    0A0H
;
; </e>
;------------------------------------------------------------------------------

; Standard SFR Symbols      标准SFR符号
ACC     DATA    0E0H
B       DATA    0F0H
SP      DATA    81H
DPL     DATA    82H
DPH     DATA    83H

                NAME    ?C_STARTUP


?C_C51STARTUP   SEGMENT   CODE
?STACK          SEGMENT   IDATA

                RSEG    ?STACK
                DS      1

                EXTRN CODE (?C_START)
                PUBLIC  ?C_STARTUP

                CSEG    AT      0000H
?C_STARTUP:     LJMP    STARTUP1

                RSEG    ?C_C51STARTUP

STARTUP1:

IF IDATALEN <> 0
                MOV     R0,#IDATALEN - 1
                CLR     A
IDATALOOP:      MOV     @R0,A
                DJNZ    R0,IDATALOOP
ENDIF

IF XDATALEN <> 0
                MOV     DPTR,#XDATASTART
                MOV     R7,#LOW (XDATALEN)
  IF (LOW (XDATALEN)) <> 0
                MOV     R6,#(HIGH (XDATALEN)) +1
  ELSE
                MOV     R6,#HIGH (XDATALEN)
  ENDIF
                CLR     A
XDATALOOP:      MOVX    @DPTR,A
                INC     DPTR
                DJNZ    R7,XDATALOOP
                DJNZ    R6,XDATALOOP
ENDIF

IF PPAGEENABLE <> 0
                MOV     PPAGE_SFR,#PPAGE
ENDIF

IF PDATALEN <> 0
                MOV     R0,#LOW (PDATASTART)
                MOV     R7,#LOW (PDATALEN)
                CLR     A
PDATALOOP:      MOVX    @R0,A
                INC     R0
                DJNZ    R7,PDATALOOP
ENDIF

IF IBPSTACK <> 0
EXTRN DATA (?C_IBP)

                MOV     ?C_IBP,#LOW IBPSTACKTOP
ENDIF

IF XBPSTACK <> 0
EXTRN DATA (?C_XBP)

                MOV     ?C_XBP,#HIGH XBPSTACKTOP
                MOV     ?C_XBP+1,#LOW XBPSTACKTOP
ENDIF

IF PBPSTACK <> 0
EXTRN DATA (?C_PBP)
                MOV     ?C_PBP,#LOW PBPSTACKTOP
ENDIF

                MOV     SP,#?STACK-1

; This code is required if you use L51_BANK.A51 with Banking Mode 4
;<h> Code Banking
; <q> Select Bank 0 for L51_BANK.A51 Mode 4
#if 0   
;     <i> Initialize bank mechanism to code bank 0 when using L51_BANK.A51 with Banking Mode 4.
EXTRN CODE (?B_SWITCH0)
                CALL    ?B_SWITCH0      ; init bank mechanism to code bank 0
#endif
;</h>
                LJMP    ?C_START
                
                CSEG    AT      0003H
                DW      00H
                DW      00H
                DW      00H
                DW      00H
                DW      00H
                DW      00H

                END
