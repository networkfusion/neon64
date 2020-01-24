; U64ASM v0.1p header file.
 assert 0x430
displaylist  equ 0x430
fillstart  equ 0x430
setcimg  equ 0x434
scissor  equ 0x438
bgcolor  equ 0x44c
fillrect  equ 0x450
fillend  equ 0x458
loadpalstart  equ 0x458
loadpalend  equ 0x488
bgsprstart  equ 0x488
primcolor  equ 0x49c
bg_spr_rect  equ 0x4d8
bgsprend  equ 0x4f8
bgstart  equ 0x4f8
bg_rect  equ 0x530
bgend  equ 0x550
fgsprstart  equ 0x550
fg_spr_rect  equ 0x588
fgsprend  equ 0x5a8
end_dl  equ 0x5a8
 incbin "staticdl.bin"
