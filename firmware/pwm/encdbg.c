// Diagnostic build: mirror the Pmod ENC inputs onto the Pmod 8LD bar.
//
//   LD0 = mprj_io[16]  ENC A
//   LD1 = mprj_io[17]  ENC B
//   LD2 = mprj_io[18]  ENC BTN
//   LD3 = mprj_io[19]  ENC SWT
//   LD7 = heartbeat, proves the CPU is running
//
// The servos are left alone; PWM is never enabled here.

#include <defs.h>
#include <stub.h>

void configure_io()
{
    reg_mprj_io_0 = GPIO_MODE_USER_STD_OUTPUT;

    reg_mprj_io_1 = GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_2 = GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_3 = GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_4 = GPIO_MODE_USER_STD_OUTPUT;

    reg_mprj_io_5 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;     // UART Rx
    reg_mprj_io_6 = GPIO_MODE_MGMT_STD_OUTPUT;           // UART Tx
    reg_mprj_io_7 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_8 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_9 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_10 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_11 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_12 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_13 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_14 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_15 = GPIO_MODE_MGMT_STD_OUTPUT;

    reg_mprj_io_16 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;    // Pmod ENC A
    reg_mprj_io_17 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;    // Pmod ENC B
    reg_mprj_io_18 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;    // Pmod ENC button
    reg_mprj_io_19 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;    // Pmod ENC switch

    reg_mprj_io_20 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_21 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_22 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_23 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_24 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_25 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_26 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_27 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_28 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_29 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_30 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_31 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_32 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_33 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_34 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_35 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_36 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_37 = GPIO_MODE_MGMT_STD_OUTPUT;

    reg_mprj_xfer = 1;
    while (reg_mprj_xfer == 1);
}

void led_bar(unsigned int bits)
{
    reg_mprj_datal = (reg_mprj_datal & 0x0fffffff) | ((bits & 0x0f) << 28);
    reg_mprj_datah = (reg_mprj_datah & 0xfffffff0) | ((bits >> 4) & 0x0f);
}

void delay(const int d)
{
	reg_timer0_config = 0;
	reg_timer0_data = d;
    reg_timer0_config = 1;

    reg_timer0_update = 1;
    while (reg_timer0_value > 0) {
           reg_timer0_update = 1;
    }
}

void main()
{
    unsigned int heartbeat = 0;
    unsigned int enc;

    reg_gpio_mode1 = 1;
    reg_gpio_mode0 = 0;
    reg_gpio_ien = 1;
    reg_gpio_oe = 1;

    reg_wb_enable  = 1;

    configure_io();

    reg_uart_enable = 1;

	reg_la0_oenb = reg_la0_iena = 0x00000000;
	reg_la1_oenb = reg_la1_iena = 0x00000000;
	reg_la2_oenb = reg_la2_iena = 0x00000000;
	reg_la3_oenb = reg_la3_iena = 0x00000000;

    while (1) {
        enc = (reg_mprj_datal >> 16) & 0x0f;

        heartbeat = !heartbeat;
        led_bar(enc | (heartbeat ? 0x80 : 0x00));
        reg_gpio_out = heartbeat;

        delay(1200000);     // 100 ms at 12 MHz
    }
}
