; MMC4 (nearly identical to MMC2 but PRGROM storage is different)
 
        andi    t4,A6502_regaddr,$f000
        li      t5,$a000
        bne     t5,t4,map9notprgrom
        li      t5,$b000
       
        andi    A6502_regval, 0x0F
        sll     A6502_regval,10+4   ; 16K
 
        la      t4,prgrom
        addu    A6502_regval,t4
        li      t4,0x8000
        subu    A6502_regval,t4
 
        li      t4,0x40-1
        la      t5,nespagetable+(0x80*4)
map10setPRGloop
        sw      A6502_regval,(t5)
        addi    t5,4
        bnez    t4,map10setPRGloop
        addi    t4,-1
        jr      A6502_return
        nop
        ; remaining stuff is done in mmc2.inc