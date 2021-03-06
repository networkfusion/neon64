; The menu system
; All toggles are intentionaly in the code here, so that settings are
; preserved, along with the indicators, through resets.

menu    _tpa(sw,ra,menura)
        li      t0,1
        _tpa(sw,t0,debugflag)   ; so VI handler doesn't switch screen

        la      t1,bgline-abuf1-1
        la      t0,abuf1
clearaudioloop ; copy zero to whole buffer for silence
        sb      r0,(t0)
        addi    t0,1
        bnez    t1,clearaudioloop
        addi    t1,-1

        ; copy current screen to buffer 3
        _tpa(lw,t0,$a4400004)
        lui     t1,$a000
        or      t0,t1
        _tpa(sw,t0,menuscrtemp)
        la      t1,screenbuffer3
        la      t2,280*256*2-8
menucopy    ld      t3,(t0)
        addi    t0,8
        sd      t3,(t1)
        addi    t1,8
        bnez    t2,menucopy
        addi    t2,-8
        la      t0,screenbuffer3
        _tpa(sw,t0,$a4400004)

        _text_string(35,45,menuframe)
        li  t4,0    ; first menu item selected initially

menu_run_loop
        li  t1,0
        li  t3,53 ; Y
        li  t0,43 ; X
        _tpa(sw,t0,text_x)
        _tpa(sw,r0,text_type)
highlightloop
        sll t2,t1,2
        _vtpa(lw,t2,menutab,t2)
        _tpa(sw,t3,text_y)
        beqz    t2,hlend
        addi    t3,8
        _tpa(sw,t2,text_msg)
        bne     t1,t4,hlno
        li      t2,$ffff
        _tpa(sw,t2,text_bg)
        li      t2,$0001
        _tpa(sw,t2,text_color)
hlno    jal     text_blit
        nop
        li      t2,$0001
        _tpa(sw,t2,text_bg)
        li      t2,$ffff
        _tpa(sw,t2,text_color)
        j       highlightloop
hlend   addi    t1,1

        jal     Pause
        nop
        andi    t1,t0,$4030 ; B,L shoulder, R shoulder
        bnez    t1,menuend
        nop
        andi    t1,t0,$0800
        beqz    t1,menu_noup
        nop
        addi    t4,-1
        bgez    t4,menu_noup
        nop
        li      t4,menu_cnt-1
menu_noup   andi    t1,t0,$0400
        beqz    t1,menu_nodown
        li      t1,menu_cnt
        addi    t4,1
        bne     t4,t1,menu_nodown
        nop
        li      t4,0
menu_nodown andi    t1,t0,$8000
        beqz    t1,menu_run_loop
        nop

        ; menu items
        bnez    t4,menu0_no ; Reset NES (hard reset)
        addi    t4,-1
        li      t0,1
        _tpa(sb,t0,resetflag)
        j       loadnewrom
        nop
menu0_no
        bnez    t4,menu1_no ; load new ROM
        addi    t4,-1
        j       loadnewrom
        nop
menu1_no
        bnez    t4,menu2_no ; save SRAM to C-Pak
        addi    t4,-1

        _tpa(lbu,t0,romc1)
        andi    t0,2
        beqz    t0,noSRAMsave
        nop
        ; write SRAM to controller pack (will handle any fanciness)
        jal     saveSRAM
        nop
noSRAMsave  j       menuend
        nop
menu2_no
        bnez    t4,menu3_no ; save NES-SRAM to N64-SRAM
        addi    t4,-1

        _tpa(lbu,t0,romc1)
        andi    t0,2
        beqz    t0,noSRAMsave
        nop
        jal     SAVE_NES_SRAM_TO_N64_SRAM
        nop
        j       menuend
        nop
menu3_no
        bnez    t4,menu4_no ; PAL mode
        addi    t4,-1
        _tpa(lbu,t0,vidmode)
        xori    t0,1
        _tpa(sb,t0,vidmode)
        la      t4,paltog
        jal     togglewrite
        nop

        jal     setscreenlimits
        nop

        jal     cls ; PAL->NTSC switch leaves a line at the bottom
        nop

        j       menuend
        nop
menu4_no
        bnez    t4,menu5_no ; take screen shot v64jr
        addi    t4,-1

        _tpa(lw,t0,menuscrtemp)
        _tpa(sw,t0,$a4400004)

        li      t0,240
        la      t1, $10400000
        jal     screen_capture
        nop

        j       menuend
        nop
menu5_no
        bnez    t4,menu6_no ; take screen shot 64driveUSB
        addi    t4,-1
        jal     screen_capture_64drive
        nop
        j       menuend
        nop
menu6_no
        bnez    t4,menu7_no ; frame by frame
        addi    t4,-1
        _tpa(lbu,t0,fbfflag)
        xori    t0,1
        _tpa(sb,t0,fbfflag)
        la      t4,fbftog
        jal     togglewrite
        nop
        j       menuend
        nop
menu7_no
        bnez    t4,menu8_no ; FPS
        addi    t4,-1
        _tpa(lbu,t0,frametoggle)
        xori    t0,1
        _tpa(sb,t0,frametoggle)
        la      t4,fpstog
        jal     togglewrite
        nop
        j       menuend
        nop
menu8_no
        bnez    t4,menu9_no ; Arkanoid controller
        addi    t4,-1
        _tpa(lbu,t0,arkflag)
        xori    t0,1
        _tpa(sb,t0,arkflag)
        la      t4,arktog
        jal     togglewrite
        nop
        j       menuend
        nop
menu9_no

        bnez    t4,menua_no ; dmc enable
        addi    t4,-1
        _tpa(lbu,t0,dmcenable)
        xori    t0,1
        _tpa(sb,t0,dmcenable)
        la      t4,dmctog
        jal     togglewrite
        nop
menua_no

        bnez    t4,menub_no ; trap errors
        addi    t4,-1
        _tpa(lbu,t0,traperrors)
        xori    t0,1
        _tpa(sb,t0,traperrors)
        _tpa(sb,t0,traperrors2)
        la      t4,errtog
        jal     togglewrite
        nop
        j       menuend
        nop
menub_no
        bnez    t4,menuc_no ; debug info
        addi    t4,-1
        jal     cls
        nop
        jal     a6502_status
        nop
        jal     pause
        nop
        j       menuend
        nop
menuc_no
        bnez    t4,menud_no ; load state from N64-SRAM
        addi    t4,-1
        jal     LoadStateSRAM
        nop
        j       menuend
        nop
menud_no
        bnez    t4,menue_no ; save state to N64-SRAM
        addi    t4,-1
        jal     SaveStateSRAM
        nop
        j       menuend
        nop
menue_no
        bnez    t4,menuf_no ; load state from ExMem
        addi    t4,-1
        _tpa(lbu,t0,SaveBeforeLoad) ; Need to save once before loading
        blez    t0, noStateload
        nop
        jal     LoadStateExMem
        nop
noStateload        j       menuend
        nop
menuf_no
        bnez    t4,menug_no ; save state to ExMem
        addi    t4,-1
        li      t0,1
        _tpa(sb,t0,SaveBeforeLoad)
        jal     SaveStateExMem
        nop
        j       menuend
        nop
menug_no

menuend
mploop  jal     read_con ; wait until button released so it won't go
        nop              ; through to the game
        srl     t0,16
        bnez    t0,mploop
        nop

            _tpa(lw,t0,menuscrtemp)
            _tpa(sw,t0,$a4400004)

            _tpa(sw,r0,debugflag)
            _tpa(lw,ra,menura)
            jr      ra
            nop

; toggle value in t0
; address to write string in t4
togglewrite
            bnez    t0,togon
            nop
            la      t1,toggleoff
            j       togoff
            nop
togon       la      t1,toggleon
togoff      lb      t2,0(t1)
            lb      t3,1(t1)
            sb      t2,0(t4)
            lb      t2,2(t1)
            sb      t3,1(t4)
            sb      t2,2(t4)
            jr      ra
            nop

setscreenlimits
        _tpa(lbu,t0,vidmode)
        la      t1,ssntsc
        sll     t0,3
        addu    t1,t0
        lw      t0,(t1)
        _tpa(sw,t0,rspdata+scissor)
        lw      t0,4(t1)
        _tpa(sw,t0,rspdata+scissor+4)
        jr      ra
        nop

;196=�
;191=�
;218=�
;192=�
;179=�
;217=�
menuframe   db "������������������������Ŀ",$0a
            db "�                        �",$0a
            db "�                        �",$0a
            db "�                        �",$0a
            db "�                        �",$0a
            db "�                        �",$0a
            db "�                        �",$0a
            db "�                        �",$0a
            db "�                        �",$0a
            db "�                        �",$0a
            db "�                        �",$0a
            db "�                        �",$0a
            db "�                        �",$0a
            db "�                        �",$0a
            db "�                        �",$0a
            db "�                        �",$0a
            db "�                        �",$0a
            db "�                        �",$0a
            db "��������������������������",0

menu_cnt equ 17

menu0       db "*Reset NES",0
menu1       db "*Load new ROM - GS",0
menu2       db "*Save NES-SRAM - C-Pak",0
menu3       db "*Save NES-SRAM - 64-SRAM",0
menu4       db "*PAL Mode "
paltog      db "Off",0
menu5       db "*Screenshot - v64jr",0
menu6       db "*Screenshot - 64driveUSB",0
menu7       db "*Frame-by-frame "
fbftog      db "Off",0
menu8       db "*FPS "
fpstog      db "Off",0
menu9       db "*Arkanoid paddle "
arktog      db "Off",0
menua       db "*DMC "
dmctog      db "On ",0
menub       db "*Error trapping "
errtog      db "Off",0
menuc       db "*Debug info",0
toggleon    db "On",0
toggleoff   db "Off",0
menud       db "*Load State <- 64-SRAM",0
menue       db "*Save State -> 64-SRAM",0
menuf       db "*Load State <- ExMem",0
menug       db "*Save State -> ExMem",0

resetflag   db 0
arkflag     db 0
fbfflag     db 0
traperrors  db 0
vidmode     db 0    ; 0=NTSC, 1=PAL
frametoggle db 0
dmcenable   db 1
VIvidmode   db 0    ; 0=NTSC, 1=PAL (included here to persist past reset)
 _align(4)
menutab dw menu0,menu1,menu2,menu3,menu4,menu5,menu6,menu7,menu8,menu9,menua,menub,menuc,menud,menue,menuf,menug,0
menuscrtemp dw  0
ssntsc
        dw  $ED000000|(8<14)|(8<2)     ; SetScissor NTSC
        dw  ($118<14)|(232<2)
sspal
        dw  $ED000000|(8<14)|(0<2)     ; SetScissor PAL (show top and bottom
        dw  ($118<14)|(240<2)           ; 8 lines)

menura  dw  0
