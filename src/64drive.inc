; SaveState Routines for the 64drive and Expansion Pak
; #####################################

#define FULL_STATE_RAMADRESS 0x80600000
#define SRAM_STATE_RAMADRESS 0x80700000

; #####################################

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

; copy blocks to ExMem_FULL_STATE

; uncached
  la  t0, (nops-firstvar-1)/8
  la  t1, firstvar
  la  t2, FULL_STATE_RAMADRESS
  jal memcopy
  nop
; cached + NES registers
  la    t0, (lastvar&$8fffffff-mem-1)/8
  la    t1, mem
  jal memcopy
  nop

  _tpa(lw,ra,statera)
  jr ra
  nop

; #####################################

LoadStateExMem
  _tpa(sw,ra,statera)

; copy blocks from ExMem_FULL_STATE

; uncached
  la  t0, (nops-firstvar-1)/8
  la  t1, FULL_STATE_RAMADRESS
  la  t2, firstvar
  jal memcopy
  nop
; cached + NES registers
  la  t0, (lastvar&$8fffffff-mem-1)/8
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

; #####################################

SAVE_NES_SRAM_TO_N64_SRAM
  _tpa(sw,ra,sram64drivera)

; write NES-SRAM header to ExMem
  la t2, SRAM_STATE_RAMADRESS + 0x8000 + 0x7000
  _tpa(ld, t0, SRAM_HEADER)
  sd t0,(t2)
  addi t2,8

; write NES-SRAM to ExMem
  la  t0, (0x2000/8) -1
  la  t1, sram
  jal memcopy
  nop

; write NES-SRAM footer to ExMem
  _tpa(ld, t0, SRAM_FOOTER)
  sd t0,(t2)

; write ExMem to N64-SRAM
  jal disable_interrupts
  nop
  jal PI_DMAWait
  nop
  jal PI_Init_SRAM
  nop
  jal PI_DMAWait
  nop

; ***SRAM CHIP1 offset = 0x00000
  la v0,0xA4600000 ;PI_BASE_REG

; IO_WRITE(PI_STATUS_REG, 0x02);
  li v1,0x2
  sw v1,0x10(v0) ;PI_STATUS_REG

; IO_WRITE(PI_DRAM_ADDR_REG, src);
  la v1,(SRAM_STATE_RAMADRESS + 0x8000 + 0x7000) & 0x1FFFFFFF
  sw v1,0x00(v0) ;PI_DRAM_ADDR_REG

; IO_WRITE(PI_CART_ADDR_REG, (0xA8000000 + offset) & 0x1FFFFFFF);
  la v1,0x08000000
  sw v1,0x04(v0) ;PI_CART_ADDR_REG

  jal PI_DMAWait
  nop
  jal data_cache_invalidate_all
  nop

; IO_WRITE(PI_RD_LEN_REG, (size - 1));
  la v0,0xA4600000 ;PI_BASE_REG
  la v1,0x2010 ; Partial Chip1
  sw v1,0x08(v0) ;PI_RD_LEN_REG

  jal PI_DMAWait
  nop

  jal enable_interrupts
  nop

  _tpa(lw,ra,sram64drivera)
  jr ra
  nop

; #####################################

LOAD_NES_SRAM_FROM_N64_SRAM
  _tpa(sw,ra,sram64drivera)

; clear ExMem area to avoid memory garbage
  la t2, SRAM_STATE_RAMADRESS + 0x8000 + 0x7000
  la t0, (0x2000/8)+2 -1
ExMemClear
  sd  r0,(t2)
  addi  t2,8
  bnez  t0, ExMemClear
  addi  t0,-1

; load N64 SRAM to ExMem
  jal disable_interrupts
  nop
  jal PI_DMAWait
  nop
  jal PI_Init_SRAM
  nop
  jal PI_DMAWait
  nop

; ***SRAM CHIP1 offset = 0x800000
  la v0,0xA4600000 ;PI_BASE_REG

; IO_WRITE(PI_STATUS_REG, 0x03);
  li v1,0x3
  sw v1,0x10(v0) ;PI_STATUS_REG

; IO_WRITE(PI_DRAM_ADDR_REG, src);
  la v1,(SRAM_STATE_RAMADRESS + 0x8000 + 0x7000) & 0x1FFFFFFF
  sw v1,0x00(v0) ;PI_DRAM_ADDR_REG

; IO_WRITE(PI_CART_ADDR_REG, (0xA8000000 + offset) & 0x1FFFFFFF);
  la v1,0x08000000
  sw v1,0x04(v0) ;PI_CART_ADDR_REG

  jal PI_DMAWait
  nop
  jal data_cache_invalidate_all
  nop

; IO_WRITE(PI_WR_LEN_REG, (size - 1));
  la v0,0xA4600000 ;PI_BASE_REG
  la v1,0x2010 ; Partial Chip1
  sw v1,0x0C(v0) ;PI_WR_LEN_REG

  jal PI_DMAWait
  nop

  jal enable_interrupts
  nop

; check header and footer
  la  t2, SRAM_STATE_RAMADRESS + 0x8000 + 0x7000
  _tpa(ld, t3, SRAM_HEADER)
  ld  t1,(t2)
  bne t3,t1,N64loadSRAMend
  nop
  addi t2,0x2000+8
  _tpa(ld, t3, SRAM_FOOTER)
  ld t1,(t2)
  bne t3,t1,N64loadSRAMend
  nop

; load ExMem SRAM to NES-SRAM
  la  t0, (0x2000/8) -1
  la  t1, SRAM_STATE_RAMADRESS + 0x8000 + 0x7000 + 0x8
  la  t2, sram
  jal memcopy
  nop

N64loadSRAMend
  _tpa(lw,ra,sram64drivera)
  jr ra
  nop

; #####################################

SaveStateSRAM
  _tpa(sw,ra,sram64drivera)

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

; copy blocks to ExMem_SRAM_STATE

; store bgbuffer - bgline
  la  t0, (pattblc-bgbuffer-1)/8
  la  t1, bgbuffer
  la  t2, SRAM_STATE_RAMADRESS
  jal memcopy
  nop
; store _SPRRAM - _bgpal
  la  t0, (nops-_SPRRAM-1)/8
  la  t1, _SPRRAM
  jal memcopy
  nop
; cached + NES registers
  la  t0, ((lastvar&$8fffffff-mem-1)/8) ;+10
  la  t1, mem
  jal memcopy
  nop

; copy to SRAM
  jal disable_interrupts
  nop
  jal PI_DMAWait
  nop
  jal PI_Init_SRAM
  nop
  jal PI_DMAWait
  nop

; ***SRAM CHIP1 offset = 0x00000
; Used for NES-SRAM saving

; ***SRAM CHIP2 offset = 0x40000
  la v0,0xA4600000 ;PI_BASE_REG

; IO_WRITE(PI_STATUS_REG, 0x02);
  li v1,0x2
  sw v1,0x10(v0) ;PI_STATUS_REG

; IO_WRITE(PI_DRAM_ADDR_REG, src);
  la v1,SRAM_STATE_RAMADRESS & 0x1FFFFFFF
  sw v1,0x00(v0) ;PI_DRAM_ADDR_REG

; IO_WRITE(PI_CART_ADDR_REG, (0xA8000000 + offset) & 0x1FFFFFFF);
  la v1,0x08040000
  sw v1,0x04(v0) ;PI_CART_ADDR_REG

  jal PI_DMAWait
  nop
  jal data_cache_invalidate_all
  nop

; IO_WRITE(PI_RD_LEN_REG, (size - 1));
  la v0,0xA4600000 ;PI_BASE_REG
  la v1,0x7FFF ; Full Chip2
  sw v1,0x08(v0) ;PI_RD_LEN_REG

  jal PI_DMAWait
  nop

; ***SRAM CHIP3 offset = 0x80000
  la v0,0xA4600000 ;PI_BASE_REG

; IO_WRITE(PI_STATUS_REG, 0x02);
  li v1,0x2
  sw v1,0x10(v0) ;PI_STATUS_REG

; IO_WRITE(PI_DRAM_ADDR_REG, src);
  la v1,(SRAM_STATE_RAMADRESS + 0x8000) & 0x1FFFFFFF
  sw v1,0x00(v0) ;PI_DRAM_ADDR_REG

; IO_WRITE(PI_CART_ADDR_REG, (0xA8000000 + offset) & 0x1FFFFFFF);
  la v1,0x08080000
  sw v1,0x04(v0) ;PI_CART_ADDR_REG

  jal PI_DMAWait
  nop
  jal data_cache_invalidate_all
  nop

; IO_WRITE(PI_RD_LEN_REG, (size - 1));
  la v0,0xA4600000 ;PI_BASE_REG
  la v1,0x6E5E ; Partial Chip3 (Remaining State Area)
  sw v1,0x08(v0) ;PI_RD_LEN_REG

  jal PI_DMAWait
  nop

  jal enable_interrupts
  nop

; restore v0+v1
  _tpa(ld, A6502_regval, A6502_regval_BAK)
  _tpa(ld, A6502_instrcnt, A6502_instrcnt_BAK)

  _tpa(lw,ra,sram64drivera)
  jr ra
  nop

; #####################################

LoadStateSRAM
  _tpa(sw,ra,sram64drivera)

; copy to SRAM
  jal disable_interrupts
  nop
  jal PI_DMAWait
  nop
  jal PI_Init_SRAM
  nop
  jal PI_DMAWait
  nop

; ***SRAM CHIP1 offset = 0x00000
; Used for NES-SRAM saving

; ***SRAM CHIP2 offset = 0x40000
  la v0,0xA4600000 ;PI_BASE_REG

; IO_WRITE(PI_STATUS_REG, 0x03);
  li v1,0x3
  sw v1,0x10(v0) ;PI_STATUS_REG

; IO_WRITE(PI_DRAM_ADDR_REG, src);
  la v1,SRAM_STATE_RAMADRESS & 0x1FFFFFFF
  sw v1,0x00(v0) ;PI_DRAM_ADDR_REG

; IO_WRITE(PI_CART_ADDR_REG, (0xA8000000 + offset) & 0x1FFFFFFF);
  la v1,0x08040000
  sw v1,0x04(v0) ;PI_CART_ADDR_REG

  jal PI_DMAWait
  nop
  jal data_cache_invalidate_all
  nop

; IO_WRITE(PI_WR_LEN_REG, (size - 1));
  la v0,0xA4600000 ;PI_BASE_REG
  la v1,0x7FFF ; Full Chip2
  sw v1,0x0C(v0) ;PI_WR_LEN_REG

  jal PI_DMAWait
  nop

; ***SRAM CHIP3 offset = 0x80000
  la v0,0xA4600000 ;PI_BASE_REG

; IO_WRITE(PI_STATUS_REG, 0x03);
  li v1,0x3
  sw v1,0x10(v0) ;PI_STATUS_REG

; IO_WRITE(PI_DRAM_ADDR_REG, src);
  la v1,(SRAM_STATE_RAMADRESS + 0x8000) & 0x1FFFFFFF
  sw v1,0x00(v0) ;PI_DRAM_ADDR_REG

; IO_WRITE(PI_CART_ADDR_REG, (0xA8000000 + offset) & 0x1FFFFFFF);
  la v1,0x08080000
  sw v1,0x04(v0) ;PI_CART_ADDR_REG

  jal PI_DMAWait
  nop
  jal data_cache_invalidate_all
  nop

; IO_WRITE(PI_WR_LEN_REG, (size - 1));
  la v0,0xA4600000 ;PI_BASE_REG
  la v1,0x6E5E ; Partial Chip3 (Remaining State Area)
  sw v1,0x0C(v0) ;PI_WR_LEN_REG

  jal PI_DMAWait
  nop

  jal enable_interrupts
  nop

; copy blocks from ExMem_SRAM_STATE
; load bgbuffer - bgline
  la  t0, (pattblc-bgbuffer-1)/8
  la  t1, SRAM_STATE_RAMADRESS
  la  t2, bgbuffer
  jal memcopy
  nop
; load _SPRRAM - _bgpal
  la  t0, (nops-_SPRRAM-1)/8
  la  t2, _SPRRAM
  jal memcopy
  nop
; cached + NES registers
  la  t0, ((lastvar&$8fffffff-mem-1)/8) ; + 10
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

  _tpa(lw,ra,sram64drivera)
  jr ra
  nop

; #####################################

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

; #####################################

disable_interrupts
  mfc0 t0, status
  nop
  li t1, ~1
  and t0, t1
  mtc0 t0, status
  nop
  jr ra
  nop

; #####################################

enable_interrupts
  mfc0 t0, status
  nop
  ori t0, 1
  mtc0 t0, status
  nop
  jr ra
  nop

; #####################################

data_cache_invalidate_all
  la t0,0x80000000
  la t1,0x80000000
  addiu t1,t1,0x1FF0
cacheloop
  cache 1,0(t0)
  cache 1,16(t0)
  cache 1,32(t0)
  cache 1,48(t0)
  cache 1,64(t0)
  cache 1,80(t0)
  cache 1,96(t0)
  addiu t0,t0,112
  bne t0,t1,cacheloop
  cache 1,0(t0)
  jr ra
  nop

; #####################################

PI_DMAWait
  la v1, 0xa4600000 ;PI_STATUS_REG
DMAwaitloop
  lw v0,16(v1)
  andi v0,v0,0x3 ;PI_STATUS_IO_BUSY | PI_STATUS_DMA_BUSY
  bne v0,r0,DMAwaitloop
  nop
  jr ra
  nop

; #####################################

PI_Init_SRAM
  la v0,0xa4600000 ;PI_STATUS_REG
  li v1,0x5
  sw v1,36(v0) ;PI_BSD_DOM2_LAT_REG
  li v1,0xc
  sw v1,40(v0) ;PI_BSD_DOM2_PWD_REG
  li v1,0xd
  sw v1,44(v0) ;PI_BSD_DOM2_PGS_REG
  li v1,0x2
  sw v1,48(v0) ;PI_BSD_DOM2_RLS_REG
  jr ra
  nop

; #####################################

 _align(8)
statera         dw  0
sram64drivera   dw  0
SRAM_HEADER     dw 0x79783B4A, 0x985626E0
SRAM_FOOTER     dw 0x0BDFD303, 0x4579BC39
SaveBeforeLoad  db 0
