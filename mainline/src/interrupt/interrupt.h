#ifndef __INTERRUPT_H_
#define __INTERRUPT_H_

#include "common.h"
#include "debug.h"
#include "string.h"


#define INT_M_CTL               0x20
#define INT_M_CTLMASK           0x21
#define INT_S_CTL               0xA0
#define INT_S_CTLMASK           0xA1
#define	INT_VECTOR_IRQ0         0x20
#define	INT_VECTOR_IRQ8         0x28

#define MAX_INTERRUPT_COUNT     128
#define MAX_SYSCALL_COUNT       128

// Syscall List
#define SYSCALL_SYNC_IPC        1
#define SYSCALL_TTY_WRITE       2

void excp000(void);
void excp001(void);
void excp002(void);
void excp003(void);
void excp004(void);
void excp005(void);
void excp006(void);
void excp007(void);
void excp008(void);
void excp009(void);
void excp010(void);
void excp011(void);
void excp012(void);
void excp013(void);
void excp014(void);
void excp015(void);
void excp016(void);
void excp017(void);
void excp018(void);
void excp019(void);
void excp020(void);
void excp021(void);
void excp022(void);
void excp023(void);
void excp024(void);
void excp025(void);
void excp026(void);
void excp027(void);
void excp028(void);
void excp029(void);
void excp030(void);
void excp031(void);
void hwint00(void);
void hwint01(void);
void hwint02(void);
void hwint03(void);
void hwint04(void);
void hwint05(void);
void hwint06(void);
void hwint07(void);
void hwint08(void);
void hwint09(void);
void hwint10(void);
void hwint11(void);
void hwint12(void);
void hwint13(void);
void hwint14(void);
void hwint15(void);
void syscall(void);

void idt_init(void);
void chip8259a_init(void);
void register_interrupt_handler(u32 interrupt_no, void (*handler)());
void register_syscall_handler(u32 syscall_no, void (*handler)());
void enable_irq_master(int irq);
void disable_irq_master(int irq);
void enable_irq_slave(int irq);
void disable_irq_slave(int irq);
#endif
