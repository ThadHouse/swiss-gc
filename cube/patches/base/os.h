#ifndef OS_H
#define OS_H

#include "common.h"

#define OS_BASE_CACHED   0x80000000
#define OS_BASE_UNCACHED 0xC0000000
#define OS_BASE_MIRRORED 0xC2000000

#define OSPhysicalToCached(paddr)    ((void *)(paddr) + OS_BASE_CACHED)
#define OSPhysicalToUncached(paddr)  ((void *)(paddr) + OS_BASE_UNCACHED)
#define OSCachedToPhysical(caddr)    ((u32)(caddr) - OS_BASE_CACHED)
#define OSUncachedToPhysical(ucaddr) ((u32)(ucaddr) - OS_BASE_UNCACHED)
#define OSCachedToUncached(caddr)    ((void *)(caddr) + (OS_BASE_UNCACHED - OS_BASE_CACHED))
#define OSUncachedToCached(ucaddr)   ((void *)(ucaddr) - (OS_BASE_UNCACHED - OS_BASE_CACHED))
#define OSCachedToMirrored(caddr)    ((void *)(caddr) + (OS_BASE_MIRRORED - OS_BASE_CACHED))

typedef struct {
	u32 gpr[32];
	u32 cr, lr, ctr, xer;
	f64 fpr[32];
	u64 fpscr;
	u32 srr0, srr1;
	u16 mode, state;
	u32 gqr[8];
	f64 psf[32];
} OSContext;

extern u32 OSExceptionContext;
extern OSContext *OSCurrentContext;
extern OSContext *OSFPUContext;

void OSSetCurrentContext(OSContext *context);
void OSClearContext(OSContext *context);

typedef enum {
	OS_EXCEPTION_SYSTEM_RESET = 0,
	OS_EXCEPTION_MACHINE_CHECK,
	OS_EXCEPTION_DSI,
	OS_EXCEPTION_ISI,
	OS_EXCEPTION_EXTERNAL_INTERRUPT,
	OS_EXCEPTION_ALIGNMENT,
	OS_EXCEPTION_PROGRAM,
	OS_EXCEPTION_FLOATING_POINT,
	OS_EXCEPTION_DECREMENTER,
	OS_EXCEPTION_SYSTEM_CALL,
	OS_EXCEPTION_TRACE,
	OS_EXCEPTION_PERFORMANCE_MONITOR,
	OS_EXCEPTION_BREAKPOINT,
	OS_EXCEPTION_SYSTEM_INTERRUPT,
	OS_EXCEPTION_THERMAL_INTERRUPT,
	OS_EXCEPTION_MAX
} OSException;

typedef void (*OSExceptionHandler)(OSException exception, OSContext *context, ...);

extern OSExceptionHandler OSExceptionHandlerTable[16];

static OSExceptionHandler OSSetExceptionHandler(OSException exception, OSExceptionHandler handler)
{
	OSExceptionHandler oldHandler = OSExceptionHandlerTable[exception];
	OSExceptionHandlerTable[exception] = handler;
	return oldHandler;
}

static OSExceptionHandler OSGetExceptionHandler(OSException exception)
{
	return OSExceptionHandlerTable[exception];
}

typedef enum {
	OS_INTERRUPT_MEM_0 = 0,
	OS_INTERRUPT_MEM_1,
	OS_INTERRUPT_MEM_2,
	OS_INTERRUPT_MEM_3,
	OS_INTERRUPT_MEM_ADDRESS,
	OS_INTERRUPT_DSP_AI,
	OS_INTERRUPT_DSP_ARAM,
	OS_INTERRUPT_DSP_DSP,
	OS_INTERRUPT_AI_AI,
	OS_INTERRUPT_EXI_0_EXI,
	OS_INTERRUPT_EXI_0_TC,
	OS_INTERRUPT_EXI_0_EXT,
	OS_INTERRUPT_EXI_1_EXI,
	OS_INTERRUPT_EXI_1_TC,
	OS_INTERRUPT_EXI_1_EXT,
	OS_INTERRUPT_EXI_2_EXI,
	OS_INTERRUPT_EXI_2_TC,
	OS_INTERRUPT_PI_CP,
	OS_INTERRUPT_PI_PE_TOKEN,
	OS_INTERRUPT_PI_PE_FINISH,
	OS_INTERRUPT_PI_SI,
	OS_INTERRUPT_PI_DI,
	OS_INTERRUPT_PI_RSW,
	OS_INTERRUPT_PI_ERROR,
	OS_INTERRUPT_PI_VI,
	OS_INTERRUPT_PI_DEBUG,
	OS_INTERRUPT_PI_HSP,
	OS_INTERRUPT_EMU_DI,
	OS_INTERRUPT_MAX
} OSInterrupt;

typedef void (*OSInterruptHandler)(OSInterrupt interrupt, OSContext *context);

typedef u32 OSInterruptMask;

#define OS_INTERRUPTMASK(interrupt) (0x80000000U >> (interrupt))

#define OS_INTERRUPTMASK_MEM_0       OS_INTERRUPTMASK(OS_INTERRUPT_MEM_0)
#define OS_INTERRUPTMASK_MEM_1       OS_INTERRUPTMASK(OS_INTERRUPT_MEM_1)
#define OS_INTERRUPTMASK_MEM_2       OS_INTERRUPTMASK(OS_INTERRUPT_MEM_2)
#define OS_INTERRUPTMASK_MEM_3       OS_INTERRUPTMASK(OS_INTERRUPT_MEM_3)
#define OS_INTERRUPTMASK_MEM_ADDRESS OS_INTERRUPTMASK(OS_INTERRUPT_MEM_ADDRESS)

#define OS_INTERRUPTMASK_MEM (OS_INTERRUPTMASK_MEM_0 | \
                              OS_INTERRUPTMASK_MEM_1 | \
                              OS_INTERRUPTMASK_MEM_2 | \
                              OS_INTERRUPTMASK_MEM_3 | \
                              OS_INTERRUPTMASK_MEM_ADDRESS)

#define OS_INTERRUPTMASK_DSP_AI   OS_INTERRUPTMASK(OS_INTERRUPT_DSP_AI)
#define OS_INTERRUPTMASK_DSP_ARAM OS_INTERRUPTMASK(OS_INTERRUPT_DSP_ARAM)
#define OS_INTERRUPTMASK_DSP_DSP  OS_INTERRUPTMASK(OS_INTERRUPT_DSP_DSP)

#define OS_INTERRUPTMASK_DSP (OS_INTERRUPTMASK_DSP_AI   | \
                              OS_INTERRUPTMASK_DSP_ARAM | \
                              OS_INTERRUPTMASK_DSP_DSP)

#define OS_INTERRUPTMASK_AI_AI OS_INTERRUPTMASK(OS_INTERRUPT_AI_AI)

#define OS_INTERRUPTMASK_AI (OS_INTERRUPTMASK_AI_AI)

#define OS_INTERRUPTMASK_EXI_0_EXI OS_INTERRUPTMASK(OS_INTERRUPT_EXI_0_EXI)
#define OS_INTERRUPTMASK_EXI_0_TC  OS_INTERRUPTMASK(OS_INTERRUPT_EXI_0_TC)
#define OS_INTERRUPTMASK_EXI_0_EXT OS_INTERRUPTMASK(OS_INTERRUPT_EXI_0_EXT)

#define OS_INTERRUPTMASK_EXI_0 (OS_INTERRUPTMASK_EXI_0_EXI | \
                                OS_INTERRUPTMASK_EXI_0_TC  | \
                                OS_INTERRUPTMASK_EXI_0_EXT)

#define OS_INTERRUPTMASK_EXI_1_EXI OS_INTERRUPTMASK(OS_INTERRUPT_EXI_1_EXI)
#define OS_INTERRUPTMASK_EXI_1_TC  OS_INTERRUPTMASK(OS_INTERRUPT_EXI_1_TC)
#define OS_INTERRUPTMASK_EXI_1_EXT OS_INTERRUPTMASK(OS_INTERRUPT_EXI_1_EXT)

#define OS_INTERRUPTMASK_EXI_1 (OS_INTERRUPTMASK_EXI_1_EXI | \
                                OS_INTERRUPTMASK_EXI_1_TC  | \
                                OS_INTERRUPTMASK_EXI_1_EXT)

#define OS_INTERRUPTMASK_EXI_2_EXI OS_INTERRUPTMASK(OS_INTERRUPT_EXI_2_EXI)
#define OS_INTERRUPTMASK_EXI_2_TC  OS_INTERRUPTMASK(OS_INTERRUPT_EXI_2_TC)

#define OS_INTERRUPTMASK_EXI_2 (OS_INTERRUPTMASK_EXI_2_EXI | \
                                OS_INTERRUPTMASK_EXI_2_TC)

#define OS_INTERRUPTMASK_EXI (OS_INTERRUPTMASK_EXI_0_EXI | \
                              OS_INTERRUPTMASK_EXI_0_TC  | \
                              OS_INTERRUPTMASK_EXI_0_EXT | \
                              OS_INTERRUPTMASK_EXI_1_EXI | \
                              OS_INTERRUPTMASK_EXI_1_TC  | \
                              OS_INTERRUPTMASK_EXI_1_EXT | \
                              OS_INTERRUPTMASK_EXI_2_EXI | \
                              OS_INTERRUPTMASK_EXI_2_TC)

#define OS_INTERRUPTMASK_PI_PE_TOKEN  OS_INTERRUPTMASK(OS_INTERRUPT_PI_PE_TOKEN)
#define OS_INTERRUPTMASK_PI_PE_FINISH OS_INTERRUPTMASK(OS_INTERRUPT_PI_PE_FINISH)

#define OS_INTERRUPTMASK_PI_PE (OS_INTERRUPTMASK_PI_PE_TOKEN | \
                                OS_INTERRUPTMASK_PI_PE_FINISH)

#define OS_INTERRUPTMASK_PI_CP    OS_INTERRUPTMASK(OS_INTERRUPT_PI_CP)
#define OS_INTERRUPTMASK_PI_SI    OS_INTERRUPTMASK(OS_INTERRUPT_PI_SI)
#define OS_INTERRUPTMASK_PI_DI    OS_INTERRUPTMASK(OS_INTERRUPT_PI_DI)
#define OS_INTERRUPTMASK_PI_RSW   OS_INTERRUPTMASK(OS_INTERRUPT_PI_RSW)
#define OS_INTERRUPTMASK_PI_ERROR OS_INTERRUPTMASK(OS_INTERRUPT_PI_ERROR)
#define OS_INTERRUPTMASK_PI_VI    OS_INTERRUPTMASK(OS_INTERRUPT_PI_VI)
#define OS_INTERRUPTMASK_PI_DEBUG OS_INTERRUPTMASK(OS_INTERRUPT_PI_DEBUG)
#define OS_INTERRUPTMASK_PI_HSP   OS_INTERRUPTMASK(OS_INTERRUPT_PI_HSP)

#define OS_INTERRUPTMASK_PI (OS_INTERRUPTMASK_PI_CP        | \
                             OS_INTERRUPTMASK_PI_SI        | \
                             OS_INTERRUPTMASK_PI_DI        | \
                             OS_INTERRUPTMASK_PI_RSW       | \
                             OS_INTERRUPTMASK_PI_ERROR     | \
                             OS_INTERRUPTMASK_PI_VI        | \
                             OS_INTERRUPTMASK_PI_PE_TOKEN  | \
                             OS_INTERRUPTMASK_PI_PE_FINISH | \
                             OS_INTERRUPTMASK_PI_DEBUG     | \
                             OS_INTERRUPTMASK_PI_HSP)

#define OS_INTERRUPTMASK_EMU_DI OS_INTERRUPTMASK(OS_INTERRUPT_EMU_DI)

#define OS_INTERRUPTMASK_EMU (OS_INTERRUPTMASK_EMU_DI)

extern OSInterruptHandler OSInterruptHandlerTable[32];

static OSInterruptHandler OSSetInterruptHandler(OSInterrupt interrupt, OSInterruptHandler handler)
{
	OSInterruptHandler oldHandler = OSInterruptHandlerTable[interrupt];
	OSInterruptHandlerTable[interrupt] = handler;
	return oldHandler;
}

static OSInterruptHandler OSGetInterruptHandler(OSInterrupt interrupt)
{
	return OSInterruptHandlerTable[interrupt];
}

#endif /* OS_H */
