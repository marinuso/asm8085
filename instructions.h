/* asm8085 (C) 2019-20 Marinus Oosters 
 *
 * instructions.h
 * This file lists all the instructions (opcodes and directives) in one place
 */
 
 
/* Empty definitions for undefined macros */
#ifndef _OP
#define _OP(x,y,z)
#endif

#ifndef _DIR
#define _DIR(x)
#endif

/* Directives */
_DIR(include)
_DIR(incbin)
_DIR(org)
_DIR(db)
_DIR(dw)
_DIR(ds)
_DIR(equ)
_DIR(macro)
_DIR(endm)
_DIR(if)
_DIR(ifdef)
_DIR(ifndef)
_DIR(endif)
_DIR(pushd)
_DIR(popd)
_DIR(align)
_DIR(assert)
_DIR(pushorg)
_DIR(poporg)
_DIR(repeat)
_DIR(endr)
_DIR(end)
_DIR(cpu)

/* Opcodes 
   
   First argument: name of instruction
   Second argument: allowed in 8080 mode
   Fourth argument: specification, which is:
        1. ARG_NONE(byte):    <byte>
        2. ARG_IMM(n, byte):  <byte> followed by N-byte immediate
        3. ARG_3CONST(expr):  byte given by expr, in which `v' is the constant
        4. ARG_R(expr):       byte given by expr, in which `r' is the register
        5. ARG_RP(expr):      byte given by expr, in which `rp` is the register pair
        6. ARG_R8(expr):      register + 8-bit immediate
        7. ARG_RP16(expr):    register pair + 16-bit immediate
        8. ARG_DS(expr):      two registers, `d' and `s'
*/

_OP(mov,  TRUE,  ARG_DS  (  0x40 | d<<3 | s ))
_OP(mvi,  TRUE,  ARG_R8  (  0x06 | r<<3     ))
_OP(lxi,  TRUE,  ARG_RP16(  0x01 | rp<<4    ))
_OP(lda,  TRUE,  ARG_IMM(2, 0x3a            ))
_OP(sta,  TRUE,  ARG_IMM(2, 0x32            )) 
_OP(lhld, TRUE,  ARG_IMM(2, 0x2a            ))
_OP(shld, TRUE,  ARG_IMM(2, 0x22            ))
_OP(ldax, TRUE,  ARG_RP  (  0x0a | rp<<4    ))
_OP(stax, TRUE,  ARG_RP  (  0x02 | rp<<4    ))
_OP(xchg, TRUE,  ARG_NONE(  0xeb            ))
_OP(add,  TRUE,  ARG_R   (  0x80 | r        ))
_OP(adi,  TRUE,  ARG_IMM(1, 0xc6            ))
_OP(adc,  TRUE,  ARG_R   (  0x88 | r        ))
_OP(aci,  TRUE,  ARG_IMM(1, 0xce            ))
_OP(sub,  TRUE,  ARG_R   (  0x90 | r        ))
_OP(sui,  TRUE,  ARG_IMM(1, 0xd6            ))
_OP(sbb,  TRUE,  ARG_R   (  0x98 | r        ))
_OP(sbi,  TRUE,  ARG_IMM(1, 0xde            ))
_OP(inr,  TRUE,  ARG_R   (  0x04 | r<<3     ))
_OP(dcr,  TRUE,  ARG_R   (  0x05 | r<<3     ))
_OP(inx,  TRUE,  ARG_RP  (  0x03 | rp<<4    ))
_OP(dcx,  TRUE,  ARG_RP  (  0x0b | rp<<4    ))
_OP(dad,  TRUE,  ARG_RP  (  0x09 | rp<<4    ))
_OP(daa,  TRUE,  ARG_NONE(  0x27            ))
_OP(ana,  TRUE,  ARG_R   (  0xa0 | r        ))
_OP(ani,  TRUE,  ARG_IMM(1, 0xe6            ))
_OP(ora,  TRUE,  ARG_R   (  0xb0 | r        ))
_OP(ori,  TRUE,  ARG_IMM(1, 0xf6            ))
_OP(xra,  TRUE,  ARG_R   (  0xa8 | r        ))
_OP(xri,  TRUE,  ARG_IMM(1, 0xee            ))
_OP(cmp,  TRUE,  ARG_R   (  0xb8 | r        ))
_OP(cpi,  TRUE,  ARG_IMM(1, 0xfe            ))
_OP(rlc,  TRUE,  ARG_NONE(  0x07            ))
_OP(rrc,  TRUE,  ARG_NONE(  0x0f            ))
_OP(ral,  TRUE,  ARG_NONE(  0x17            ))
_OP(rar,  TRUE,  ARG_NONE(  0x1f            ))
_OP(cma,  TRUE,  ARG_NONE(  0x2f            ))
_OP(cmc,  TRUE,  ARG_NONE(  0x3f            ))
_OP(stc,  TRUE,  ARG_NONE(  0x37            ))
_OP(jmp,  TRUE,  ARG_IMM(2, 0xc3            ))
_OP(jnz,  TRUE,  ARG_IMM(2, 0xc2            ))
_OP(jz,   TRUE,  ARG_IMM(2, 0xca            ))
_OP(jnc,  TRUE,  ARG_IMM(2, 0xd2            ))
_OP(jc,   TRUE,  ARG_IMM(2, 0xda            ))
_OP(jpo,  TRUE,  ARG_IMM(2, 0xe2            ))
_OP(jpe,  TRUE,  ARG_IMM(2, 0xea            ))
_OP(jp,   TRUE,  ARG_IMM(2, 0xf2            ))
_OP(jm,   TRUE,  ARG_IMM(2, 0xfa            ))
_OP(call, TRUE,  ARG_IMM(2, 0xcd            ))
_OP(cnz,  TRUE,  ARG_IMM(2, 0xc4            ))
_OP(cz,   TRUE,  ARG_IMM(2, 0xcc            ))
_OP(cnc,  TRUE,  ARG_IMM(2, 0xd4            ))
_OP(cc,   TRUE,  ARG_IMM(2, 0xdc            ))
_OP(cpo,  TRUE,  ARG_IMM(2, 0xe4            ))
_OP(cpe,  TRUE,  ARG_IMM(2, 0xec            ))
_OP(cp,   TRUE,  ARG_IMM(2, 0xf4            ))
_OP(cm,   TRUE,  ARG_IMM(2, 0xfc            ))
_OP(ret,  TRUE,  ARG_NONE(  0xc9            ))
_OP(rnz,  TRUE,  ARG_NONE(  0xc0            ))
_OP(rz,   TRUE,  ARG_NONE(  0xc8            ))
_OP(rnc,  TRUE,  ARG_NONE(  0xd0            ))
_OP(rc,   TRUE,  ARG_NONE(  0xd8            ))
_OP(rpo,  TRUE,  ARG_NONE(  0xe0            ))
_OP(rpe,  TRUE,  ARG_NONE(  0xe8            ))
_OP(rp,   TRUE,  ARG_NONE(  0xf0            ))
_OP(rm,   TRUE,  ARG_NONE(  0xf8            ))
_OP(rst,  TRUE,  ARG_3CONST(0xc7 | v<<3     ))
_OP(pchl, TRUE,  ARG_NONE(  0xe9            ))
_OP(push, TRUE,  ARG_RP  (  0xc5 | rp<<4    ))
_OP(pop,  TRUE,  ARG_RP  (  0xc1 | rp<<4    ))
_OP(xthl, TRUE,  ARG_NONE(  0xe3            ))
_OP(sphl, TRUE,  ARG_NONE(  0xf9            ))
_OP(in,   TRUE,  ARG_IMM(1, 0xdb            ))
_OP(out,  TRUE,  ARG_IMM(1, 0xd3            ))
_OP(ei,   TRUE,  ARG_NONE(  0xfb            ))
_OP(di,   TRUE,  ARG_NONE(  0xf3            ))
_OP(hlt,  TRUE,  ARG_NONE(  0x76            ))
_OP(nop,  TRUE,  ARG_NONE(  0x00            ))

/* 8085 specific opcodes */
_OP(dsub, FALSE, ARG_NONE(  0x08            ))
_OP(arhl, FALSE,  ARG_NONE(  0x10            ))
_OP(rdel, FALSE,  ARG_NONE(  0x18            ))
_OP(rim,  FALSE,  ARG_NONE(  0x20            ))
_OP(ldhi, FALSE,  ARG_IMM(1, 0x28            ))
_OP(sim,  FALSE,  ARG_NONE(  0x30            ))
_OP(ldsi, FALSE,  ARG_IMM(1, 0x38            ))
_OP(rstv, FALSE,  ARG_NONE(  0xcb            ))
_OP(shlx, FALSE,  ARG_NONE(  0xd9            ))
_OP(jnk,  FALSE,  ARG_IMM(2, 0xdd            ))
_OP(lhlx, FALSE,  ARG_NONE(  0xed            ))
_OP(jk,   FALSE,  ARG_IMM(2, 0xfd            ))


#undef _OP
#undef _DIR
