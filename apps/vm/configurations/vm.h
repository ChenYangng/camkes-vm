/*
 * Copyright 2014, NICTA
 *
 * This software may be distributed and modified according to the terms of
 * the GNU General Public License version 2. Note that NO WARRANTY is provided.
 * See "LICENSE_GPLv2.txt" for details.
 *
 * @TAG(NICTA_GPL)
 */

#include <autoconf.h>
#include <boost/preprocessor/arithmetic.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/list/for_each.hpp>
#include <boost/preprocessor/comparison.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/expand.hpp>
#include <boost/preprocessor/tuple.hpp>
#include <boost/preprocessor/control.hpp>

/* Include every possible configuration and assume they guard themselves
 * by insepcting the actual build configuration */
#define c162_threevm_testing 1
#include "c162_threevm_testing.h"
#define nohardware_onevm 2
#include "nohardware_onevm.h"

/* Now define a bunch of general definitions for constructing the VM */

#define CAT BOOST_PP_CAT

/* The timer layout is
 * timer 0 timer for serial server
 * blocks of 8 timers for each VM
 */
#define VTIMER_FIRST 1
#define VTIMER_NUM   8
#define VTIMER(t, n) VTIMER_I(t, n)
#define VTIMER_I(t, n) CAT(timer,BOOST_PP_ADD(VTIMER_FIRST,BOOST_PP_ADD(t, BOOST_PP_MUL(VTIMER_NUM, n))))
#define VM_NUM_TIMERS BOOST_PP_ADD(VTIMER_FIRST, BOOST_PP_MUL(VTIMER_NUM, VM_NUM_GUESTS))

/* VM and per VM componenents */
#define VM_COMP_DEF(num) \
    component Init vm##num; \
    component PICEmulator IntMan##num; \
    component PITEmulator PIT##num; \
    component RTCEmulator RTCEmul##num; \
    component SerialEmulator SerialEmul##num; \
    /**/

#define VM_CONNECT_DEF(num) \
    /* Connect all the components to the serial server */ \
    connection seL4RPCCall serial_vm##num(from vm##num.putchar, to serial.vm##num); \
    connection seL4RPCCall serial_intman##num(from IntMan##num.putchar, to serial.vm##num); \
    connection seL4RPCCall serial_pit##num(from PIT##num.putchar, to serial.vm##num); \
    connection seL4RPCCall serial_rtcemul##num(from RTCEmul##num.putchar, to serial.vm##num); \
    connection seL4RPCCall serial_serialemul##num(from SerialEmul##num.putchar, to serial.guest##num); \
    /* Connect the emulated serial input to the serial server */ \
    connection seL4RPCCall serial_input##num(from SerialEmul##num.getchar, to serial.CAT(guest##num,_input)); \
    connection seL4Asynch serial_input_ready##num(from serial.CAT(guest##num,_input_signal), to SerialEmul##num.getchar_signal); \
    /* Temporarily connect the VM directly to the RTC */ \
    connection seL4RPCCall rtctest##num(from vm##num.rtc, to rtc.rtc); \
    /* Connect the emulated serial to the VM */ \
    connection seL4RPCCall serial##num(from vm##num.serial, to SerialEmul##num.serialport); \
    /* Connect the emulated PIT to the timer server */ \
    connection seL4RPCCall CAT(pit##num,_timer)(from PIT##num.timer, to time_server.VTIMER(0, num)); \
    connection seL4Asynch CAT(pit##num,_timer_interrupt)(from time_server.CAT(VTIMER(0, num),_complete), to PIT##num.timer_interrupt); \
    /* Connect the emulated RTC to the timer server */ \
    connection seL4RPCCall CAT(rtc##num,_timer0)(from RTCEmul##num.periodic_timer, to time_server.VTIMER(1, num)); \
    connection seL4Asynch CAT(rtc##num,_timer0_interrupt)(from time_server.CAT(VTIMER(1, num),_complete), to RTCEmul##num.periodic_timer_interrupt); \
    connection seL4RPCCall CAT(rtc##num,_timer1)(from RTCEmul##num.coalesced_timer, to time_server.VTIMER(2, num)); \
    connection seL4Asynch CAT(rtc##num,_timer1_interrupt)(from time_server.CAT(VTIMER(2, num),_complete), to RTCEmul##num.coalesced_timer_interrupt); \
    connection seL4RPCCall CAT(rtc##num,_timer2)(from RTCEmul##num.second_timer, to time_server.VTIMER(3, num)); \
    connection seL4Asynch CAT(rtc##num,_timer2_interrupt)(from time_server.CAT(VTIMER(3, num),_complete), to RTCEmul##num.second_timer_interrupt); \
    connection seL4RPCCall CAT(rtc##num,_timer3)(from RTCEmul##num.second_timer2, to time_server.VTIMER(4, num)); \
    connection seL4Asynch CAT(rtc##num,_timer3_interrupt)(from time_server.CAT(VTIMER(4, num),_complete), to RTCEmul##num.second_timer2_interrupt); \
    /* Connect the emulated serial to the timer server */ \
    connection seL4RPCCall CAT(serial##num,_timer0)(from SerialEmul##num.fifo_timeout, to time_server.VTIMER(5, num)); \
    connection seL4Asynch CAT(serial##num,_timer0_interrupt)(from time_server.CAT(VTIMER(5,num),_complete), to SerialEmul##num.fifo_timeout_interrupt); \
    connection seL4RPCCall CAT(serial##num,_timer1)(from SerialEmul##num.transmit_timer, to time_server.VTIMER(6, num)); \
    connection seL4Asynch CAT(serial##num,_timer1_interrupt)(from time_server.CAT(VTIMER(6,num),_complete), to SerialEmul##num.transmit_timer_interrupt); \
    connection seL4RPCCall CAT(serial##num,_timer2)(from SerialEmul##num.modem_status_timer, to time_server.VTIMER(7, num)); \
    connection seL4Asynch CAT(serial##num,_timer2_interrupt)(from time_server.CAT(VTIMER(7,num),_complete), to SerialEmul##num.modem_status_timer_interrupt); \
    /* Connect the emulated RTC to the RTC component */ \
    connection seL4RPCCall cmosrtc_system##num(from RTCEmul##num.system_rtc, to rtc.rtc); \
    /* Connect the emulated RTC to the VM */ \
    connection seL4RPCCall cmosrtc##num(from vm##num.cmos, to RTCEmul##num.cmosport); \
    /* Connect the emulated PIT to the VM */ \
    connection seL4RPCCall i8254_##num(from vm##num.i8254, to PIT##num.i8254port); \
    /* Connect config space to main VM */ \
    connection seL4RPCCall pciconfig##num(from vm##num.pci_config, to pci_config.pci_config); \
    /* Connect the PIC emulator to the main VM */ \
    connection seL4RPCCall i8259port##num(from vm##num.i8259, to IntMan##num.i8259port); \
    connection seL4RPCCall intmanager##num(from vm##num.IntManager, to IntMan##num.i8259int); \
    connection seL4AsynchBind haveint##num(from IntMan##num.haveint, to vm##num.intready); \
    /* Connect the emulated pit to the PIC emulator */ \
    connection seL4RPCCall irq0_level_##num(from PIT##num.pit_irq, to IntMan##num.irq0_level); \
    connection seL4Asynch irq0_edge_##num(from PIT##num.pit_edge_irq, to IntMan##num.irq0); \
    /* Connect the emulated rtc to the PIC emulator */ \
    connection seL4RPCCall irq8_level_##num(from RTCEmul##num.rtc_irq, to IntMan##num.irq8_level); \
    /* Connect the emulated serial to the PIC emulator */ \
    connection seL4RPCCall irq4_level_##num(from SerialEmul##num.serial_irq, to IntMan##num.irq4_level); \
    connection seL4Asynch irq4_edge_##num(from SerialEmul##num.serial_edge_irq, to IntMan##num.irq4); \
    /**/

#ifdef CONFIG_APP_CAMKES_VM_GUEST_DMA_ONE_TO_ONE
#define VM_MAYBE_ZONE_DMA(num) vm##num.mmio = "0x8000:0x97000";
#else
#define VM_MAYBE_ZONE_DMA(num)
#endif

/* If the platform configuration defined extra ram that we
 * generate the specifics of that generation for them */
#ifdef VM_CONFIGURATION_EXTRA_RAM
#define EXTRA_RAM_OUTPUT(a,b) BOOST_PP_STRINGIZE(a:b)
#define VM_MAYBE_EXTRA_RAM(num) vm##num.mmio = BOOST_PP_EXPAND(EXTRA_RAM_OUTPUT CAT(VM_CONFIGURATION_EXTRA_RAM_,num)()) ;
#else
#define VM_MAYBE_EXTRA_RAM(num)
#endif

/* Generate IOSpace capabilities if using the IOMMU */
#ifdef CONFIG_APP_CAMKES_VM_GUEST_DMA_IOMMU
#define IOSPACE_OUTPUT(r, data, elem) vm##data.iospace = elem;
#define VM_MAYBE_IOSPACE(num) BOOST_PP_LIST_FOR_EACH(IOSPACE_OUTPUT,num,BOOST_PP_TUPLE_TO_LIST(CAT(VM_CONFIGURATION_IOSPACES_,num)()))
#else
#define VM_MAYBE_IOSPACE(num)
#endif

#define MMIO_OUTPUT(r, data, elem) vm##data.mmio = elem;
#define VM_MMIO(num) BOOST_PP_LIST_FOR_EACH(MMIO_OUTPUT, num, BOOST_PP_TUPLE_TO_LIST(CAT(VM_CONFIGURATION_MMIO_, num)()))

#define IOPORT_OUTPUT(r, data, elem) vm##data.ioport = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0,elem):BOOST_PP_TUPLE_ELEM(1,elem));
#define VM_IOPORT(num) BOOST_PP_LIST_FOR_EACH(IOPORT_OUTPUT, num, BOOST_PP_TUPLE_TO_LIST(CAT(VM_CONFIGURATION_IOPORT_, num)()))

#define VM_CONFIG_DEF(num) \
    vm##num.cnode_size_bits = 21; \
    vm##num.simple = true; \
    VM_MAYBE_ZONE_DMA(num) \
    VM_MAYBE_EXTRA_RAM(num) \
    VM_MAYBE_IOSPACE(num) \
    VM_MMIO(num) \
    VM_IOPORT(num) \
    /**/
