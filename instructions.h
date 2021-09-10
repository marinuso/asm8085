/* asm8085 (C) 2019-20 Marinus Oosters 
 *
 * instructions.h
 * This file lists all the instructions (opcodes and directives) in one place
 */
 
 
/* Empty definitions for undefined macros */
#ifndef _OP
#define _OP(x,y)
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
   Second argument: specification, which is:
        1. ARG_NONE(byte):    <byte>
        2. ARG_IMM(n, byte):  <byte> followed by N-byte immediate
        3. ARG_3CONST(expr):  byte given by expr, in which `v' is the constant
        4. ARG_R(expr):       byte given by expr, in which `r' is the register
        5. ARG_RP(expr):      byte given by expr, in which `rp` is the register pair
        6. ARG_R8(expr):      register + 8-bit immediate
        7. ARG_RP16(expr):    register pair + 16-bit immediate
        8. ARG_DS(expr):      two registers, `d' and `s'
*/

_OP(mov,    ARG_DS  (  0x40 | d<<3 | s ))
_OP(mvi,    ARG_R8  (  0x06 | r<<3     ))
_OP(lxi,    ARG_RP16(  0x01 | rp<<4    ))
_OP(lda,    ARG_IMM(2, 0x3a            ))
_OP(sta,    ARG_IMM(2, 0x32            )) 
_OP(lhld,   ARG_IMM(2, 0x2a            ))
_OP(shld,   ARG_IMM(2, 0x22            ))
_OP(ldax,   ARG_RP  (  0x0a | rp<<4    ))
_OP(stax,   ARG_RP  (  0x02 | rp<<4    ))
_OP(xchg,   ARG_NONE(  0xeb            ))
_OP(add,    ARG_R   (  0x80 | r        ))
_OP(adi,    ARG_IMM(1, 0xc6            ))
_OP(adc,    ARG_R   (  0x88 | r        ))
_OP(aci,    ARG_IMM(1, 0xce            ))
_OP(sub,    ARG_R   (  0x90 | r        ))
_OP(sui,    ARG_IMM(1, 0xd6            ))
_OP(sbb,    ARG_R   (  0x98 | r        ))
_OP(sbi,    ARG_IMM(1, 0xde            ))
_OP(inr,    ARG_R   (  0x04 | r<<3     ))
_OP(dcr,    ARG_R   (  0x05 | r<<3     ))
_OP(inx,    ARG_RP  (  0x03 | rp<<4    ))
_OP(dcx,    ARG_RP  (  0x0b | rp<<4    ))
_OP(dad,    ARG_RP  (  0x09 | rp<<4    ))
_OP(daa,    ARG_NONE(  0x27            ))
_OP(ana,    ARG_R   (  0xa0 | r        ))
_OP(ani,    ARG_IMM(1, 0xe6            ))
_OP(ora,    ARG_R   (  0xb0 | r        ))
_OP(ori,    ARG_IMM(1, 0xf6            ))
_OP(xra,    ARG_R   (  0xa8 | r        ))
_OP(xri,    ARG_IMM(1, 0xee            ))
_OP(cmp,    ARG_R   (  0xb8 | r        ))
_OP(cpi,    ARG_IMM(1, 0xfe            ))
_OP(rlc,    ARG_NONE(  0x07            ))
_OP(rrc,    ARG_NONE(  0x0f            ))
_OP(ral,    ARG_NONE(  0x17            ))
_OP(rar,    ARG_NONE(  0x1f            ))
_OP(cma,    ARG_NONE(  0x2f            ))
_OP(cmc,    ARG_NONE(  0x3f            ))
_OP(stc,    ARG_NONE(  0x37            ))
_OP(jmp,    ARG_IMM(2, 0xc3            ))
_OP(jnz,    ARG_IMM(2, 0xc2            ))
_OP(jz,     ARG_IMM(2, 0xca            ))
_OP(jnc,    ARG_IMM(2, 0xd2            ))
_OP(jc,     ARG_IMM(2, 0xda            ))
_OP(jpo,    ARG_IMM(2, 0xe2            ))
_OP(jpe,    ARG_IMM(2, 0xea            ))
_OP(jp,     ARG_IMM(2, 0xf2            ))
_OP(jm,     ARG_IMM(2, 0xfa            ))
_OP(call,   ARG_IMM(2, 0xcd            ))
_OP(cnz,    ARG_IMM(2, 0xc4            ))
_OP(cz,     ARG_IMM(2, 0xcc            ))
_OP(cnc,    ARG_IMM(2, 0xd4            ))
_OP(cc,     ARG_IMM(2, 0xdc            ))
_OP(cpo,    ARG_IMM(2, 0xe4            ))
_OP(cpe,    ARG_IMM(2, 0xec            ))
_OP(cp,     ARG_IMM(2, 0xf4            ))
_OP(cm,     ARG_IMM(2, 0xfc            ))
_OP(ret,    ARG_NONE(  0xc9            ))
_OP(rnz,    ARG_NONE(  0xc0            ))
_OP(rz,     ARG_NONE(  0xc8            ))
_OP(rnc,    ARG_NONE(  0xd0            ))
_OP(rc,     ARG_NONE(  0xd8            ))
_OP(rpo,    ARG_NONE(  0xe0            ))
_OP(rpe,    ARG_NONE(  0xe8            ))
_OP(rp,     ARG_NONE(  0xf0            ))
_OP(rm,     ARG_NONE(  0xf8            ))
_OP(rst,    ARG_3CONST(0xc7 | v<<3     ))
_OP(pchl,   ARG_NONE(  0xe9            ))
_OP(push,   ARG_RP  (  0xc5 | rp<<4    ))
_OP(pop,    ARG_RP  (  0xc1 | rp<<4    ))
_OP(xthl,   ARG_NONE(  0xe3            ))
_OP(sphl,   ARG_NONE(  0xf9            ))
_OP(in,     ARG_IMM(1, 0xdb            ))
_OP(out,    ARG_IMM(1, 0xd3            ))
_OP(ei,     ARG_NONE(  0xfb            ))
_OP(di,     ARG_NONE(  0xf3            ))
_OP(hlt,    ARG_NONE(  0x76            ))
_OP(nop,    ARG_NONE(  0x00            ))

/* 8085 specific opcodes */
_OP(dsub,   ARG_NONE(  0x08            ))
_OP(arhl,   ARG_NONE(  0x10            ))
_OP(rdel,   ARG_NONE(  0x18            ))
_OP(rim,    ARG_NONE(  0x20            ))
_OP(ldhi,   ARG_IMM(1, 0x28            ))
_OP(sim,    ARG_NONE(  0x30            ))
_OP(ldsi,   ARG_IMM(1, 0x38            ))
_OP(rstv,   ARG_NONE(  0xcb            ))
_OP(shlx,   ARG_NONE(  0xd9            ))
_OP(jnk,    ARG_IMM(2, 0xdd            ))
_OP(lhlx,   ARG_NONE(  0xed            ))
_OP(jk,     ARG_IMM(2, 0xfd            ))


#undef _OP
#undef _DIR
