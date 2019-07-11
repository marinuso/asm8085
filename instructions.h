/* asm8085 (C) 2019 Marinus Oosters 
 *
 * instructions.h
 * This file lists all the instructions (opcodes and directives) in one place
 */
 
 
/* Empty definitions for undefined macros */
#ifndef _OP
#define _OP(x)
#endif

#ifndef _DIR
#define _DIR(x)
#endif

/* Directives */
_DIR(org)
_DIR(db)
_DIR(dw)
_DIR(ds)
_DIR(equ)
//_DIR(macro)
//_DIR(end)

/* Opcodes */
_OP(mov)
_OP(mvi)
_OP(lxi)
_OP(lda)
_OP(sta)
_OP(lhld)
_OP(shld)
_OP(ldax)
_OP(stax)
_OP(xchg)
_OP(add)
_OP(adi)
_OP(adc)
_OP(aci)
_OP(sub)
_OP(sui)
_OP(sbb)
_OP(sbi)
_OP(inr)
_OP(dcr)
_OP(inx)
_OP(dcx)
_OP(dad)
_OP(daa)
_OP(ana)
_OP(ani)
_OP(ora)
_OP(ori)
_OP(xra)
_OP(xri)
_OP(cmp)
_OP(cpi)
_OP(rlc)
_OP(rrc)
_OP(ral)
_OP(rar)
_OP(cma)
_OP(cmc)
_OP(stc)
_OP(jmp)
_OP(jnz)
_OP(jz)
_OP(jnc)
_OP(jc)
_OP(jpo)
_OP(jpe)
_OP(jp)
_OP(jm)
_OP(call)
_OP(cnz)
_OP(cz)
_OP(cnc)
_OP(cc)
_OP(cpo)
_OP(cpe)
_OP(cp)
_OP(cm)
_OP(rst)
_OP(pchl)
_OP(push)
_OP(pop)
_OP(xthl)
_OP(sphl)
_OP(in)
_OP(out)
_OP(ei)
_OP(di)
_OP(hlt)
_OP(nop)
/* 8085 specific opcodes */
_OP(dsub)
_OP(arhl)
_OP(rdel)
_OP(rim)
_OP(ldhi)
_OP(sim)
_OP(ldsi)
_OP(rstv)
_OP(shlx)
_OP(jnk)
_OP(lhlx)
_OP(jk)


#undef _OP
#undef _DIR
