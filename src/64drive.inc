; SaveState Routines for the 64drive and Expansion Pak
; #####################################

#define FULL_STATE_RAMADRESS 0x80600000
#define SRAM_STATE_RAMADRESS 0x80700000

#define CI_HARDWARE_MAGIC    0xB80002EC
#define CI_COMMAND_REGISTER  0xB8000208

#define PI_BASE_REG          0xA4600000

; #####################################

SaveStateExMem
  _tpa(sw,ra,statera)
  jal BackupNESregs
  nop
; copy blocks to ExMem_FULL_STATE
; uncached
  la  t0, ((nops-firstvar)/8)-1
  la  t1, firstvar
  la  t2, FULL_STATE_RAMADRESS
  jal memcopy
  nop
; cached + NES registers
  la    t0, ((lastvar&$8fffffff-mem)/8)-1
  la    t1, mem
  jal memcopy
  nop
  jal BackupPPUdata
  nop
  _tpa(lw,ra,statera)
  jr ra
  nop

; #####################################

LoadStateExMem
  _tpa(sw,ra,statera)
; copy blocks from ExMem_FULL_STATE
; uncached
  la  t0, ((nops-firstvar)/8)-1
  la  t1, FULL_STATE_RAMADRESS
  la  t2, firstvar
  jal memcopy
  nop
; cached + NES registers
  la  t0, ((lastvar&$8fffffff-mem)/8)-1
  la  t2, mem
  jal memcopy
  nop
  jal RestorePPUdata
  nop
  jal ResumeRSP
  nop
  jal RestoreNESregs
  nop
  _tpa(lw,ra,statera)
  jr ra
  nop

; #####################################

SAVE_NES_SRAM_TO_N64_SRAM
  _tpa(sw,ra,statera)

; write NES-SRAM header to ExMem
  la t2, SRAM_STATE_RAMADRESS + 0x8000 + 0x8000 ;(NES-SRAM stored after SRAM SaveState)
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
  la v0,PI_BASE_REG

; IO_WRITE(PI_STATUS_REG, 0x02);
  li v1,0x2
  sw v1,0x10(v0) ;PI_STATUS_REG

; IO_WRITE(PI_DRAM_ADDR_REG, src);
  la v1,(SRAM_STATE_RAMADRESS + 0x8000 + 0x8000) & 0x1FFFFFFF
  sw v1,0x00(v0) ;PI_DRAM_ADDR_REG

; IO_WRITE(PI_CART_ADDR_REG, (0xA8000000 + offset) & 0x1FFFFFFF);
  la v1,0x08000000
  sw v1,0x04(v0) ;PI_CART_ADDR_REG

  jal PI_DMAWait
  nop
  jal data_cache_invalidate_all
  nop

; IO_WRITE(PI_RD_LEN_REG, (size - 1));
  la v0,PI_BASE_REG
  la v1,0x2010 ; Partial Chip1
  sw v1,0x08(v0) ;PI_RD_LEN_REG

  jal PI_DMAWait
  nop

  jal enable_interrupts
  nop

  _tpa(lw,ra,statera)
  jr ra
  nop

; #####################################

LOAD_NES_SRAM_FROM_N64_SRAM
  _tpa(sw,ra,statera)

; clear ExMem area to avoid memory garbage
  la t2, SRAM_STATE_RAMADRESS + 0x8000 + 0x8000 ;(NES-SRAM stored after SRAM SaveState)
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
  la v0,PI_BASE_REG

; IO_WRITE(PI_STATUS_REG, 0x03);
  li v1,0x3
  sw v1,0x10(v0) ;PI_STATUS_REG

; IO_WRITE(PI_DRAM_ADDR_REG, src);
  la v1,(SRAM_STATE_RAMADRESS + 0x8000 + 0x8000) & 0x1FFFFFFF
  sw v1,0x00(v0) ;PI_DRAM_ADDR_REG

; IO_WRITE(PI_CART_ADDR_REG, (0xA8000000 + offset) & 0x1FFFFFFF);
  la v1,0x08000000
  sw v1,0x04(v0) ;PI_CART_ADDR_REG

  jal PI_DMAWait
  nop
  jal data_cache_invalidate_all
  nop

; IO_WRITE(PI_WR_LEN_REG, (size - 1));
  la v0,PI_BASE_REG
  la v1,0x2010 ; Partial Chip1
  sw v1,0x0C(v0) ;PI_WR_LEN_REG

  jal PI_DMAWait
  nop

  jal enable_interrupts
  nop

; check header and footer
  la  t2, SRAM_STATE_RAMADRESS + 0x8000 + 0x8000
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
  la  t1, SRAM_STATE_RAMADRESS + 0x8000 + 0x8000 + 0x8
  la  t2, sram
  jal memcopy
  nop

N64loadSRAMend
  _tpa(lw,ra,statera)
  jr ra
  nop

; #####################################

SaveStateSRAM
  _tpa(sw,ra,statera)

  jal BackupNESregs
  nop

; gcpages handling (Part A)
; (3. case): ROM without CHR with SRAM
; Inject first 0x2080 byte into sramheader + sramm structure (only used while loading SRAM from mempak)
  _tpa(lbu,t0,chrromsize)
  bnez    t0, gcpages_save_handling_A_end ; 1. case (no injection needed)
  nop
  _tpa(lbu,t0,romc1)
  andi    t0,2
  beqz    t0, gcpages_save_handling_A_end ; 2. case (no injection needed)
  nop
  la t0, ((realpal-sramheader)/8)-1 ; 3. case (injection needed)
  la t1, pattblc
  la t2, sramheader
  jal memcopy
  nop
gcpages_save_handling_A_end

; copy blocks to ExMem_SRAM_STATE
; write NES-SRAM header to ExMem
  la  t2, SRAM_STATE_RAMADRESS
  _tpa(ld, t0, SRAM_HEADER)
  sd t0,(t2)
  addi t2,8
; store bgbuffer - bgline
  la  t0, ((pattblc-bgbuffer)/8)-1
  la  t1, bgbuffer
  jal memcopy
  nop
; store _SPRRAM - _bgpal
  la  t0, ((nops-_SPRRAM)/8)-1
  la  t1, _SPRRAM
  jal memcopy
  nop
; cached + NES registers
  la  t0, ((lastvar&$8fffffff-mem)/8)-1
  la  t1, mem
  jal memcopy
  nop

  jal BackupPPUdata
  nop

; write NES-SRAM footer to ExMem
  addi t2,0x1000
  _tpa(ld, t0, SRAM_FOOTER)
  sd t0,(t2)

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
; Used for NES-SRAM saving (and 3. case gcpages handling)

; ***SRAM CHIP2 offset = 0x40000
  la v0,PI_BASE_REG

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
  la v0,PI_BASE_REG
  la v1,0x7FFF ; Full Chip2
  sw v1,0x08(v0) ;PI_RD_LEN_REG

  jal PI_DMAWait
  nop

; ***SRAM CHIP3 offset = 0x80000
  la v0,PI_BASE_REG

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
  la v0,PI_BASE_REG
  la v1,0x7E5F+8+8 ; Partial Chip3 (Remaining State Area)
  sw v1,0x08(v0) ;PI_RD_LEN_REG

  jal PI_DMAWait
  nop

; gcpages handling (not enough space in N64 SRAM for full pattblc buffer)
;  1. case CHR game -> nothing to do, can simply be recompiled after loading state
;  2. case game without CHR and without SRAM -> no cache used, store gcpage in N64 SRAM CHIP1 (not utilisesd otherwise since no NES SRAM)
;  3. case game without CHR and with SRAM -> split gcpage in CHIP1 and sramheader and sramm structure
  _tpa(lbu,t0,chrromsize)
  beqz    t0, NO_CHR_SAVE
  nop
  j gcpages_save_end ; 1. case
  nop
NO_CHR_SAVE
  _tpa(lbu,t0,romc1)
  andi    t0,2
  bnez    t0, HAS_SRAM_AND_NO_CHR_SAVE ; 3. case (Part B)
  nop
  j       NO_SRAM_AND_NO_CHR_SAVE ; 2. case
  nop
gcpages_save_end

  jal enable_interrupts
  nop

  _tpa(lw,ra,statera)
  jr ra
  nop

; #####################################

LoadStateSRAM
  _tpa(sw,ra,statera)

; clear Header and Footer in ExMem area to avoid memory garbage
  la t2, SRAM_STATE_RAMADRESS
  sd r0,(t2)
  la t2, SRAM_STATE_RAMADRESS+0x8000+0x7E60+8 ; CHIP2 + CHIP3 + HEADER
  sd r0,(t2)

; load from SRAM
  jal disable_interrupts
  nop
  jal PI_DMAWait
  nop
  jal PI_Init_SRAM
  nop
  jal PI_DMAWait
  nop

; ***SRAM CHIP1 offset = 0x00000
; Used for NES-SRAM savingg (and 3. case gcpages handling)

; ***SRAM CHIP2 offset = 0x40000
  la v0,PI_BASE_REG

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
  la v0,PI_BASE_REG
  la v1,0x7FFF ; Full Chip2
  sw v1,0x0C(v0) ;PI_WR_LEN_REG

  jal PI_DMAWait
  nop

; ***SRAM CHIP3 offset = 0x80000
  la v0,PI_BASE_REG

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
  la v0,PI_BASE_REG
  la v1,0x7E5F+8+8 ; Partial Chip3 (Remaining State Area)
  sw v1,0x0C(v0) ;PI_WR_LEN_REG

  jal PI_DMAWait
  nop

  jal enable_interrupts
  nop

; check header and footer
  la  t2, SRAM_STATE_RAMADRESS
  _tpa(ld, t3, SRAM_HEADER)
  ld  t1,(t2)
  bne t3,t1,N64loadStateSRAMend
  nop
  la  t2, SRAM_STATE_RAMADRESS+0x8000+0x7E60+8 ; CHIP2 + CHIP3 + HEADER
  _tpa(ld, t3, SRAM_FOOTER)
  ld t1,(t2)
  bne t3,t1,N64loadStateSRAMend
  nop

; copy blocks from ExMem_SRAM_STATE
; load bgbuffer - bgline
  la  t1, SRAM_STATE_RAMADRESS+8
  la  t0, ((pattblc-bgbuffer)/8)-1
  la  t2, bgbuffer
  jal memcopy
  nop
; load _SPRRAM - _bgpal
  la  t0, ((nops-_SPRRAM)/8)-1
  la  t2, _SPRRAM
  jal memcopy
  nop
; cached + NES registers
  la  t0, ((lastvar&$8fffffff-mem)/8)-1
  la  t2, mem
  jal memcopy
  nop

  jal RestorePPUdata
  nop

; gcpages handling (see save function description for case details)
  _tpa(lbu,t0,chrromsize)
  beqz    t0, NO_CHR_LOAD
  nop
  j recompile_pattern_table ; 1. case
  nop
NO_CHR_LOAD
  _tpa(lbu,t0,romc1)
  andi    t0,2
  bnez    t0, HAS_SRAM_AND_NO_CHR_LOAD ; 3. case
  nop
  j       NO_SRAM_AND_NO_CHR_LOAD ; 2. case
  nop
gcpages_load_end

  jal ResumeRSP
  nop

  jal RestoreNESregs
  nop

N64loadStateSRAMend

  _tpa(lw,ra,statera)
  jr ra
  nop

  ; #####################################

recompile_pattern_table ; (1. case)
  jal data_cache_invalidate_all
  nop

; recompile full pattern table cache
  lui  v0, 0
  la   v1, metacache
  li   a0, cachepages-1
compile_gcpages
  lw   t5, 4(v1)           ; skip age value and load NES address
  beqz t5, skipCacheEntry  ; skip empty cachepages
  nop
  move t6, v0
  jal  cpage
  nop
skipCacheEntry
  addi v0, cachepagesize
  addi v1, 8
  bnez a0, compile_gcpages
  addi a0,-1

  j gcpages_load_end
  nop

; #####################################

NO_SRAM_AND_NO_CHR_SAVE ; (2. case)

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
  la v0,PI_BASE_REG

  ; IO_WRITE(PI_STATUS_REG, 0x02);
  li v1,0x2
  sw v1,0x10(v0) ;PI_STATUS_REG

  ; IO_WRITE(PI_DRAM_ADDR_REG, src);
  la v1,pattblc & 0x1FFFFFFF
  sw v1,0x00(v0) ;PI_DRAM_ADDR_REG

  ; IO_WRITE(PI_CART_ADDR_REG, (0xA8000000 + offset) & 0x1FFFFFFF);
  la v1,0x08000000
  sw v1,0x04(v0) ;PI_CART_ADDR_REG

  jal PI_DMAWait
  nop
  jal data_cache_invalidate_all
  nop

  ; IO_WRITE(PI_RD_LEN_REG, (size - 1));
  la v0,PI_BASE_REG
  la v1,0x7FFF ; Full Chip1
  sw v1,0x08(v0) ;PI_RD_LEN_REG

  jal PI_DMAWait
  nop

  j gcpages_save_end
  nop

; #####################################

NO_SRAM_AND_NO_CHR_LOAD ; (2. case)

; load from SRAM
  jal disable_interrupts
  nop
  jal PI_DMAWait
  nop
  jal PI_Init_SRAM
  nop
  jal PI_DMAWait
  nop

; ***SRAM CHIP1 offset = 0x00000
  la v0,PI_BASE_REG

; IO_WRITE(PI_STATUS_REG, 0x03);
  li v1,0x3
  sw v1,0x10(v0) ;PI_STATUS_REG

; IO_WRITE(PI_DRAM_ADDR_REG, src);
  la v1,pattblc & 0x1FFFFFFF
  sw v1,0x00(v0) ;PI_DRAM_ADDR_REG

; IO_WRITE(PI_CART_ADDR_REG, (0xA8000000 + offset) & 0x1FFFFFFF);
  la v1,0x08000000
  sw v1,0x04(v0) ;PI_CART_ADDR_REG

  jal PI_DMAWait
  nop
  jal data_cache_invalidate_all
  nop

; IO_WRITE(PI_WR_LEN_REG, (size - 1));
  la v0,PI_BASE_REG
  la v1,0x7FFF ; Full Chip1
  sw v1,0x0C(v0) ;PI_WR_LEN_REG

  jal PI_DMAWait
  nop

  jal enable_interrupts
  nop

  j gcpages_load_end
  nop

; #####################################

HAS_SRAM_AND_NO_CHR_SAVE ; (3. case) (Part B)
; save the remaing gcpages bytes into N64 SRAM CHIP1 after NES SRAM

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
  la v0,PI_BASE_REG

  ; IO_WRITE(PI_STATUS_REG, 0x02);
  li v1,0x2
  sw v1,0x10(v0) ;PI_STATUS_REG

  ; IO_WRITE(PI_DRAM_ADDR_REG, src);
  la v1,(pattblc + 0x2080) & 0x1FFFFFFF ; Part A size
  sw v1,0x00(v0) ;PI_DRAM_ADDR_REG

  ; IO_WRITE(PI_CART_ADDR_REG, (0xA8000000 + offset) & 0x1FFFFFFF);
  la v1,0x08002010 ; After NES SRAM
  sw v1,0x04(v0) ;PI_CART_ADDR_REG

  jal PI_DMAWait
  nop
  jal data_cache_invalidate_all
  nop

  ; IO_WRITE(PI_RD_LEN_REG, (size - 1));
  la v0,PI_BASE_REG
  la v1,0x8000 - 0x2080 - 1 ; Partial Chip1
  sw v1,0x08(v0) ;PI_RD_LEN_REG

  jal PI_DMAWait
  nop

  j gcpages_save_end
  nop

; #####################################

HAS_SRAM_AND_NO_CHR_LOAD ; (3. case)

  ; Restore Part A
  la t0, ((realpal-sramheader)/8)-1 ; 3. case (injection needed)
  la t1, sramheader
  la t2, pattblc
  jal memcopy
  nop

; Restore Part B (load from SRAM)
  jal disable_interrupts
  nop
  jal PI_DMAWait
  nop
  jal PI_Init_SRAM
  nop
  jal PI_DMAWait
  nop

; ***SRAM CHIP1 offset = 0x00000
  la v0,PI_BASE_REG

; IO_WRITE(PI_STATUS_REG, 0x03);
  li v1,0x3
  sw v1,0x10(v0) ;PI_STATUS_REG

; IO_WRITE(PI_DRAM_ADDR_REG, src);
  la v1,(pattblc + 0x2080) & 0x1FFFFFFF ; Part A size
  sw v1,0x00(v0) ;PI_DRAM_ADDR_REG

; IO_WRITE(PI_CART_ADDR_REG, (0xA8000000 + offset) & 0x1FFFFFFF);
  la v1,0x08002010 ; After NES SRAM
  sw v1,0x04(v0) ;PI_CART_ADDR_REG

  jal PI_DMAWait
  nop
  jal data_cache_invalidate_all
  nop

; IO_WRITE(PI_WR_LEN_REG, (size - 1));
  la v0,PI_BASE_REG
  la v1,0x8000 - 0x2080 - 1 ; Partial Chip1
  sw v1,0x0C(v0) ;PI_WR_LEN_REG

  jal PI_DMAWait
  nop

  jal enable_interrupts
  nop

  j gcpages_load_end
  nop

; #####################################

screen_capture_64drive
  _tpa(sw,ra,statera)

; detect 64drive
  _tpa(lw, t0, CI_HARDWARE_MAGIC)
  la t1, 0x55444556 ; U, D, E, V
  beq t0, t1, Found64drive
  nop

; nag if other backup device
  jal cls
  nop
  _text_string(20,50,no64drive)
  jal     pause
  nop

  _tpa(lw,ra,statera)
  jr ra
  nop

Found64drive
  jal ciWait
  nop
  li t0, 0xf0 ; enable cartrom writes
  _tpa(sw, t0, CI_COMMAND_REGISTER)
  jal ciWait
  nop

; copy framebuffer to cart rom
  jal PI_DMAWait
  nop
  la      t3, PI_BASE_REG
  la      t0, screenbuffer&$1fffffff
  sw      t0, $0000(t3)
  la      t2, $10300000   ; 64drive SDRAM location
  sw      t2, $0004(t3)
  la      t1, (screenbuffer2 - screenbuffer) - 1 ; framebuffer size
  sw      t1, $0008(t3)
  jal PI_DMAWait
  nop

  jal ciWait
  nop
  li t0, 0xf1 ; disable cartrom writes
  _tpa(sw, t0, CI_COMMAND_REGISTER)
  jal ciWait
  nop

  _tpa(lw,ra,statera)
  jr ra
  nop

; #####################################

memcopy ; t0 = count, t1 = src, t2 = dest
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
  la v1,PI_BASE_REG
DMAwaitloop
  lw v0,16(v1)    ; PI_STATUS_REG
  andi v0,v0,0x3  ; PI_STATUS_IO_BUSY | PI_STATUS_DMA_BUSY
  bne v0,r0,DMAwaitloop
  nop
  jr ra
  nop

; #####################################

PI_Init_SRAM
  la v0,PI_BASE_REG ; PI_STATUS_REG
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

ResumeRSP
  li      t0,$1000
  _tpa(sw,t0,$a4080000)   ; set PC
  li      t0,%100101101   ; clear all, set int on break
  _tpa(sw,t0,0xa4040010)  ; set status
  jr ra
  nop

; #####################################

PauseRSP
  li      t0,2                           ; halt the RSP
  _tpa(sw,t0,0xa4040010)
  li      t0,1|4|$10|$40|$80|$100|$200   ; halt the RDP
  _tpa(sw,t0,0xa410000C)
  jr ra
  nop

; #####################################

Wait_SP_DMA_FULL_REG
  _tpa(lw,t0,$a4040014)
   bnez    t0,Wait_SP_DMA_FULL_REG
   nop
   jr ra
   nop

; #####################################

BackupNESregs
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
  jr ra
  nop

; #####################################

RestoreNESregs
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
  jr ra
  nop

; #####################################

; t2 = RDRAM address
BackupPPUdata
  _tpa(sw,ra,ppura)
  jal PauseRSP
  nop
  jal Wait_SP_DMA_FULL_REG
  nop

; DMA from DMEM
  la      t3,0x0
  move    t4, t2
  li      t5,0x1000-8
  lui     t0,$a404
  sw      t3,0(t0)        ; t3=rspdest
  sw      t4,4(t0)        ; t4=dram
  sw      t5,0x0C(t0)     ; t5=size

  jal Wait_SP_DMA_FULL_REG
  nop
  jal ResumeRSP
  nop

  _tpa(lw,ra,ppura)
  jr ra
  nop

; #####################################

; t2 = RDRAM address
RestorePPUdata
  _tpa(sw,ra,ppura)
  jal PauseRSP
  nop
  jal Wait_SP_DMA_FULL_REG
  nop

; DMA to DMEM
  la      t3,0x0
  move    t4, t1
  li      t5,0x1000-8
  lui     t0,$a404
  sw      t3,0(t0)        ; t3=rspdest
  sw      t4,4(t0)        ; t4=dram
  sw      t5,8(t0)        ; t5=size

  jal Wait_SP_DMA_FULL_REG
  nop

; enforce palette change
  lui     t0,sp_set_palchg>16
  _tpa(sw,t0,sp_base_reg+sp_status_reg)

  _tpa(lw,ra,ppura)
  jr ra
  nop

; #####################################

ciWait
  _tpa(lwu, t0, 0xB8000200)
  andi t0, t0, 0xF000
  bne t0, r0, ciWait
  nop

  la t1, PI_BASE_REG
DMAwaitloopCI
  lw t0,16(t1)    ; PI_STATUS_REG
  andi t0,t0,0x3  ; PI_STATUS_IO_BUSY | PI_STATUS_DMA_BUSY
  bne t0,r0,DMAwaitloopCI
  nop

  jr ra
  nop

; #####################################

 _align(8)
SRAM_HEADER     dw 0x79783B4A, 0x985626E0
SRAM_FOOTER     dw 0x0BDFD303, 0x4579BC39
statera         dw 0
ppura           dw 0
SaveBeforeLoad  db 0
no64drive       db "64drive not detected!",0
