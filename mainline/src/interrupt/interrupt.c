#include "interrupt.h"
#include "debug.h"

u8 idt_base_addr[6];
static hw_gate_t idt[MAX_INTERRUPT_COUNT];
volatile int k_reenter = -1;
static void (*vector_table[MAX_INTERRUPT_COUNT])(void) = {
        excp000, excp001, excp002, excp003,
        excp004, excp005, excp006, excp007,
        excp008, excp009, excp010, excp011,
        excp012, excp013, excp014, excp015,
        excp016, excp017, excp018, excp019,
        excp020, excp021, excp022, excp023,
        excp024, excp025, excp026, excp027,
        excp028, excp029, excp030, excp031,
        hwint00, hwint01, hwint02, hwint03,
        hwint04, hwint05, hwint06, hwint07,
        hwint08, hwint09, hwint10, hwint11,
        hwint12, hwint13, hwint14, hwint15,
};
void (*handler_table[MAX_INTERRUPT_COUNT])(void);


void idt_init(void)
{       
        sw_gate_t gate = {0}; 
        int i = 0;

        gate.selector     = 0x8;
        gate.present      = 0x1;
        gate.dpl          = 0x3;
        gate.segment_type = 0xe;
        gate.param_count  = 0x0;

        for (i = 0; i < MAX_INTERRUPT_COUNT; i++) {
                gate.handler_entry_offset = (u32)vector_table[i];
                write_gate(&idt[i], &gate);
        }


        gate.handler_entry_offset = (u32)syscall;
        write_gate(&idt[100], &gate);
      

        *(u16 *)idt_base_addr = sizeof(hw_gate_t) * MAX_INTERRUPT_COUNT;
        *((u32 *)&idt_base_addr[2]) = (u32)&idt;
}


void exception_handler(int vector_no)
{
        char *err_msg[] = {
                "#DE Divide Error",
                "#DB RESERVED",
                "#NI NMI Interrupt",
                "#BP Breakpoint",
                "#OF Overflow",
                "#BR BOUND Range Exceeded",
                "#UD Invalid Opcode (Undefined Opcode)",
                "#NM Device Not Available (No Math Coprocessor)",
                "#DF Double Fault",
                "#SO Coprocessor Segment Overrun (reserved)",
                "#TS Invalid TSS",
                "#NP Segment Not Present",
                "#SS Stack-Segment Fault",
                "#GP General Protection",
                "#PF Page Fault",
                "#IR(Intel reserved. Do not use.)",
                "#MF x87 FPU Floating-Point Error (Math Fault)",
                "#AC Alignment Check",
                "#MC Machine Check",
                "#XF SIMD Floating-Point Exception"
	};

	log_string(err_msg[vector_no]); 
}

static void out_byte(u16 port, u8 data)
{
        __asm__ __volatile__(
                "outb   %0,           %1   \n\t"
                :
                :"r"(data), "r"(port)
                :
        );
}

void chip8259a_init(void)
{
        out_byte(INT_M_CTL,     0x11);
        out_byte(INT_S_CTL,     0x11);
        out_byte(INT_M_CTLMASK, INT_VECTOR_IRQ0);
        out_byte(INT_S_CTLMASK, INT_VECTOR_IRQ8);
        out_byte(INT_M_CTLMASK, 0x04);
        out_byte(INT_S_CTLMASK, 0x02);
        out_byte(INT_M_CTLMASK, 0x01);
        out_byte(INT_S_CTLMASK, 0x01);
        out_byte(INT_M_CTLMASK, 0xff);
        out_byte(INT_S_CTLMASK, 0xff);
}

void register_interrupt_handler(u32 vector_no, void (*handler)())
{
        if (weak_assert(vector_no <= MAX_INTERRUPT_COUNT))
                return;       
 
        handler_table[vector_no] = handler;
}

void enable_irq_master(int irq)
{
        unsigned char mask = ~((unsigned char)(1 << irq));

        __asm__ __volatile__(
                "inb    $0x21,          %%al    \n\t"
                "movb   %0,             %%ah    \n\t"
                "andb   %%ah,           %%al    \n\t"
                "outb   %%al,           $0x21   \n\t"
                :
                :"g"(mask)
                :"ax"
        );
}

void disable_irq_master(int irq)
{
        unsigned char mask = (unsigned char)(1 << irq);
        
        __asm__ __volatile__(
                "inb    $0x21,          %%al    \n\t"
                "movb   %0,             %%ah    \n\t"
                "orb    %%ah,           %%al    \n\t"
                "outb   %%al,           $0x21   \n\t"
                :
                :"g"(mask)
                :"ax"
        );
}

void enable_irq_slave(int irq)
{
        unsigned char mask = ~((unsigned char)(1 << irq));
        
        __asm__ __volatile__(
                "inb    $0xa1,          %%al    \n\t"
                "movb   %0,             %%ah    \n\t"
                "andb   %%ah,           %%al    \n\t"
                "outb   %%al,           $0xa1   \n\t"
                :
                :"g"(mask)
                :"ax"
        );
}

void disable_irq_slave(int irq)
{
        unsigned char mask = (unsigned char)(1 << irq);
        
        __asm__ __volatile__(
                "inb    $0xa1,          %%al    \n\t"
                "movb   %0,             %%ah    \n\t"
                "orb    %%ah,           %%al    \n\t"
                "outb   %%al,           $0xa1   \n\t"
                :
                :"g"(mask)
                :"ax"
        );
}

