; SaveState Routines for the 64drive and Expansion Pak
;#####################################

; ExMem Adress
#define RAMadress 0x80600000
; defs.inc byte up to mem / 8
#define copycount 0x16f1e
; mem to end / 8
#define memcount 0x749

;#####################################

SaveStateExMem

  _tpa(sw,ra,statera)

  ; backup NES registers
  _tpa(sd, A6502_flags, A6502_flags_BAK)
  _tpa(sd, A6502_PC, A6502_PC_BAK)
  _tpa(sd, A6502_X, A6502_X_BAK)
  _tpa(sd, A6502_Y, A6502_Y_BAK)
  _tpa(sd, A6502_A, A6502_A_BAK)
  _tpa(sd, A6502_S, A6502_S_BAK)
  _tpa(sd, A6502_count, A6502_count_BAK)
  _tpa(sd, A6502_nespage, A6502_nespage_BAK)
  _tpa(sd, A6502_mempage, A6502_mempage_BAK)
  _tpa(sd, A6502_readfcn, A6502_readfcn_BAK)
  _tpa(sd, A6502_writefcn, A6502_writefcn_BAK)
  _tpa(sd, A6502_return, A6502_return_BAK)
  _tpa(sd, A6502_regaddr, A6502_regaddr_BAK)
  _tpa(sd, A6502_regval, A6502_regval_BAK)
  _tpa(sd, A6502_instrcnt, A6502_instrcnt_BAK)

; copy blocks to ExMem

; uncached
  la  t0, copycount
  la  t1, firstvar
  la  t2, RAMadress
  jal memcopy
  nop
; cached + NES registers
  la    t0, memcount
  la    t1, mem
  jal memcopy
  nop

  _tpa(lw,ra,statera)
  jr ra
  nop

;#####################################

LoadStateExMem
  _tpa(sw,ra,statera)

; copy blocks from ExMem

; uncached
  la  t0, copycount
  la  t1, RAMadress
  la  t2, firstvar
  jal memcopy
  nop
; cached + NES registers
  la  t0, memcount
  la  t2, mem
  jal memcopy
  nop

; restore NES registers
  _tpa(ld, A6502_flags, A6502_flags_BAK)
  _tpa(ld, A6502_PC, A6502_PC_BAK)
  _tpa(ld, A6502_X, A6502_X_BAK)
  _tpa(ld, A6502_Y, A6502_Y_BAK)
  _tpa(ld, A6502_A, A6502_A_BAK)
  _tpa(ld, A6502_S, A6502_S_BAK)
  _tpa(ld, A6502_count, A6502_count_BAK)
  _tpa(ld, A6502_nespage, A6502_nespage_BAK)
  _tpa(ld, A6502_mempage, A6502_mempage_BAK)
  _tpa(ld, A6502_readfcn, A6502_readfcn_BAK)
  _tpa(ld, A6502_writefcn, A6502_writefcn_BAK)
  _tpa(ld, A6502_return, A6502_return_BAK)
  _tpa(ld, A6502_regaddr, A6502_regaddr_BAK)
  _tpa(ld, A6502_regval, A6502_regval_BAK)
  _tpa(ld, A6502_instrcnt, A6502_instrcnt_BAK)

  _tpa(lw,ra,statera)
  jr ra
  nop

;#####################################

; t0 = count
; t1 = src
; t2 = dest
memcopy
    ld  t3,(t1)
    sd  t3,(t2)

    addi    t1,8
    addi    t2,8

    bnez    t0, memcopy
    addi    t0,-1

    jr ra
    nop

;#####################################

statera  dw  0
