#include <defs.h>
#include <stub.h>
//#include <limits.h>

//#include "EF_TMR32.h"

#define CSR_PWM0_BASE 0x30000000L
#define CSR_PWM1_BASE 0x30010000L
#define CSR_PWM2_BASE 0x30020000L
#define CSR_PWM3_BASE 0x30030000L

//#define TMR320 ((EF_TMR32_TYPE_PTR)CSR_PWM0_BASE)

// Pmod 8LD on header JA: LD0-LD7 are mprj_io[28] through mprj_io[35],
// which straddle the datal/datah register boundary at mprj_io[32].
// Pmod ENC on header JC: A, B, BTN, SWT are mprj_io[16] through mprj_io[19].
#define ENC_BTN_MASK     (1u << 18)
#define ENC_SWT_MASK     (1u << 19)
#define SERVO_MIN_TICKS  6000
#define SERVO_MID_TICKS  18000
#define SERVO_MAX_TICKS  30000
#define SERVO_STEP_TICKS 1500
// Quadrature needs a fast poll, but the sweep only has to notice the switch,
// so poll slowly there to keep the original dwell between poses.
#define POLL_TICKS_MANUAL 12000      // 1 ms at 12 MHz
#define POLL_TICKS_AUTO   400000     // 33 ms at 12 MHz
#define AUTO_DWELL_POLLS  20         // 20 x 33 ms, as the original 8M-tick delay
// With no encoder attached the switch pad floats, so only act on a reading
// that has held steady across several polls.
#define SWT_DEBOUNCE_POLLS 8


// --------------------------------------------------------
// Firmware routines
// --------------------------------------------------------

void configure_io()
{

//  ======= Useful GPIO mode values =============

//      GPIO_MODE_MGMT_STD_INPUT_NOPULL
//      GPIO_MODE_MGMT_STD_INPUT_PULLDOWN
//      GPIO_MODE_MGMT_STD_INPUT_PULLUP
//      GPIO_MODE_MGMT_STD_OUTPUT
//      GPIO_MODE_MGMT_STD_BIDIRECTIONAL
//      GPIO_MODE_MGMT_STD_ANALOG

//      GPIO_MODE_USER_STD_INPUT_NOPULL
//      GPIO_MODE_USER_STD_INPUT_PULLDOWN
//      GPIO_MODE_USER_STD_INPUT_PULLUP
//      GPIO_MODE_USER_STD_OUTPUT
//      GPIO_MODE_USER_STD_BIDIRECTIONAL
//      GPIO_MODE_USER_STD_ANALOG


//  ======= set each IO to the desired configuration =============

    //  GPIO 0 is turned off to prevent toggling the debug pin; For debug, make this an output and
    //  drive it externally to ground.

    reg_mprj_io_0 = GPIO_MODE_USER_STD_OUTPUT;

    // Changing configuration for IO[1-4] will interfere with programming flash. if you change them,
    // You may need to hold reset while powering up the board and initiating flash to keep the process
    // configuring these IO from their default values.

    reg_mprj_io_1 = GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_2 = GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_3 = GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_4 = GPIO_MODE_USER_STD_OUTPUT;

    // -------------------------------------------

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
    // The Pmod ENC carries its own resistor network; an internal pull fights it
    // hard enough that the pads never reach a valid level.
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

    // Initiate the serial transfer to configure IO
    reg_mprj_xfer = 1;
    while (reg_mprj_xfer == 1);
}

void led_bar(unsigned int bits)
{
    reg_mprj_datal = (reg_mprj_datal & 0x0fffffff) | ((bits & 0x0f) << 28);
    reg_mprj_datah = (reg_mprj_datah & 0xfffffff0) | ((bits >> 4) & 0x0f);
}

unsigned int led_bar_for_ticks(int ticks)
{
    unsigned int level = 1;
    int threshold = SERVO_MIN_TICKS + 3000;

    while ((level < 8) && (ticks >= threshold)) {
        level++;
        threshold += 3000;
    }

    return (1u << level) - 1u;
}

int encoder_delta(unsigned int transition)
{
    // Index is previous AB in bits [3:2], current AB in bits [1:0].
    switch (transition) {
        case 0x1: case 0x7: case 0x8: case 0xe:
            return -1;
        case 0x2: case 0x4: case 0xb: case 0xd:
            return 1;
        default:
            return 0;
    }
}

void delay(const int d)
{

//    /* Configure timer for a single-shot countdown */
//	reg_timer0_config = 0;
//	reg_timer0_value = d;
//    reg_timer0_config = 3;
//
//    // Loop, waiting for value to reach zero
//   while (reg_timer0_data > 0) {
//   }

   /* Configure timer for a single-shot countdown */
	reg_timer0_config = 0;
	reg_timer0_data = d;     // load value to timer
    reg_timer0_config = 1;   // enable timer

    // Loop, waiting for value to reach zero
   reg_timer0_update = 1;  // latch current value
   while (reg_timer0_value > 0) {  // check latched value
           reg_timer0_update = 1;
   }

}

//int multiply(int a, int b) {
//    // Handle negative numbers and avoid potential overflow issues.
//    int sign = 1;
//    if ((a < 0) && ( b > 0) || ( a > 0) && ( b < 0 )) {
//        sign = -1;
//    }
//
//    if (b < 0) {
//        b = -b;
//    }
//    if (a < 0) {
//        a = -a;
//    }
//
//    uint32_t ua = (uint32_t)a; // Ensure positive values for shifting.
//    uint32_t ub = (uint32_t)b;
//    uint32_t result = 0;
//
//    while (ub > 0) {
//        if (ub & 1) {
//            result += ua;
//        }
//        ua <<= 1;
//        ub >>= 1;
//    }
//
//    // Apply the sign and handle potential overflow.
//    if (sign == -1) {
//        if (result > (uint32_t)INT_MAX + 1) { // Check for overflow when negating
//            return INT_MIN; // Return INT_MIN on overflow
//        }
//        return -(int)result;
//    } else {
//        if (result > INT_MAX) { // Check for overflow
//            return INT_MAX; // Return INT_MAX on overflow
//        }
//        return (int)result;
//    }
//}
//
//int divide(int dividend, int divisor) {
//
//    unsigned int abs_dividend;
//    unsigned int abs_divisor = divisor;
//    unsigned int quotient = 0;
//    unsigned int temp = 0;
//
//    if ( dividend < 0 )
//        abs_dividend = -dividend;
//
//    for (int i = 31; i >= 0; i--) {
//        if (temp + (abs_divisor << i) <= abs_dividend) {
//            temp += abs_divisor << i;
//            quotient |= 1 << i;
//        }
//    }
//
//    return (dividend < 0) ? (int)-quotient : (int)quotient;
//}


//void config_pwm(int p0, int p1, int p2, int p3)
//{
//
//    const int reload = 240000;
//    const int prescale = 0;
//    const int config = 0b110;
//    const int disable = 0b0000;
//    const int enable = 0b1101;
//    const int match  = 0b000000000110;   // cmpx_down | cmpy_down | reload | cmpy_up | cmpx_up | zero
//
//    int check_range(int x)
//    {
//        if (x < -180)
//            return -180;
//        else if ( x > 180)
//            return 180;
//        else
//            return x;
//    }
//
//    int p0_ticks = 18000 + divide(multiply(p0, 12000), 180);
//    int p1_ticks = 18000 + divide(multiply(p1, 12000), 180);
//    int p2_ticks = 18000 + divide(multiply(p2, 12000), 180);
//    int p3_ticks = 18000 + divide(multiply(p3, 12000), 180);
//
//    // clock = 12MHz period = 83.33 nsec
//    // reload =  12000 = 1 ms
//    // reload = 240000 = 20 ms = 50 Hz
//    // cmpx =  6000 =  500 usec  -180 deg
//    // cmpx = 12000 = 1000 usec   -90 deg
//    // cmpx = 18000 = 1500 usec     0 deg
//    // cmpx = 24000 = 2000 usec    90 deg
//    // cmpx = 30000 = 2500 usec   180 deg
//
//    csr_write_simple(disable, CSR_PWM0_BASE + 0x14L);       // Disable Timer
//    csr_write_simple(reload, CSR_PWM0_BASE + 0x04L);        // Reload
//    csr_write_simple(prescale, CSR_PWM0_BASE + 0x08L);      // Set prescale
//    csr_write_simple(config, CSR_PWM0_BASE + 0x18L);        // Set Cfg
//    csr_write_simple(p0_ticks, CSR_PWM0_BASE + 0x0cL);      // Cmpx
//    csr_write_simple(p0_ticks, CSR_PWM0_BASE + 0x10L);      // Cmpy
//    csr_write_simple(match, CSR_PWM0_BASE + 0x1cL);         // pwm0cfg - match high
//    csr_write_simple(match, CSR_PWM0_BASE + 0x20L);         // pwm1cfg - match high
//    csr_write_simple(enable, CSR_PWM0_BASE + 0x14L);        // Ctrl - Enable PWM
//
//    csr_write_simple(disable, CSR_PWM1_BASE + 0x14L);       // Disable Timer
//    csr_write_simple(reload, CSR_PWM1_BASE + 0x04L);        // Reload
//    csr_write_simple(prescale, CSR_PWM1_BASE + 0x08L);      // Set prescale
//    csr_write_simple(config, CSR_PWM1_BASE + 0x18L);        // Set Cfg
//    csr_write_simple(p1_ticks, CSR_PWM1_BASE + 0x0cL);      // Cmpx
//    csr_write_simple(p1_ticks, CSR_PWM1_BASE + 0x10L);      // Cmpy
//    csr_write_simple(match, CSR_PWM1_BASE + 0x1cL);         // pwm0cfg - match high
//    csr_write_simple(match, CSR_PWM1_BASE + 0x20L);         // pwm1cfg - match high
//    csr_write_simple(enable, CSR_PWM1_BASE + 0x14L);        // Ctrl - Enable PWM
//
//    csr_write_simple(disable, CSR_PWM2_BASE + 0x14L);       // Disable Timer
//    csr_write_simple(reload, CSR_PWM2_BASE + 0x04L);        // Reload
//    csr_write_simple(prescale, CSR_PWM2_BASE + 0x08L);      // Set prescale
//    csr_write_simple(config, CSR_PWM2_BASE + 0x18L);        // Set Cfg
//    csr_write_simple(p1_ticks, CSR_PWM2_BASE + 0x0cL);      // Cmpx
//    csr_write_simple(p1_ticks, CSR_PWM2_BASE + 0x10L);      // Cmpy
//    csr_write_simple(match, CSR_PWM2_BASE + 0x1cL);         // pwm0cfg - match high
//    csr_write_simple(match, CSR_PWM2_BASE + 0x20L);         // pwm1cfg - match high
//    csr_write_simple(enable, CSR_PWM2_BASE + 0x14L);        // Ctrl - Enable PWM
//
//    csr_write_simple(disable, CSR_PWM3_BASE + 0x14L);       // Disable Timer
//    csr_write_simple(reload, CSR_PWM3_BASE + 0x04L);        // Reload
//    csr_write_simple(prescale, CSR_PWM3_BASE + 0x08L);      // Set prescale
//    csr_write_simple(config, CSR_PWM3_BASE + 0x18L);        // Set Cfg
//    csr_write_simple(p1_ticks, CSR_PWM3_BASE + 0x0cL);      // Cmpx
//    csr_write_simple(p1_ticks, CSR_PWM3_BASE + 0x10L);      // Cmpy
//    csr_write_simple(match, CSR_PWM3_BASE + 0x1cL);         // pwm0cfg - match high
//    csr_write_simple(match, CSR_PWM3_BASE + 0x20L);         // pwm1cfg - match high
//    csr_write_simple(enable, CSR_PWM3_BASE + 0x14L);        // Ctrl - Enable PWM
//
//}

void config_pwm_ticks(int p0_ticks, int p1_ticks, int p2_ticks, int p3_ticks)
{

    const int reload = 240000;   // servo requires 50 Hz period
    const int prescale = 0;
    const int config = 0b110;            // count up | periodic
    const int cmpy_value = 2400000;      // count up | periodic
    const int disable = 0b0000;
    const int enable = 0b1101;
    const int match  = 0b000010000110;   // cmpx_down | cmpy_down | reload | cmpy_up | cmpx_up | zero

    // clock = 12MHz period = 83.33 nsec
    // reload = 240000 = 20 ms = 50 Hz
    // cmpx =  6000 =  500 usec  -180 deg
    // cmpx = 12000 = 1000 usec   -90 deg
    // cmpx = 18000 = 1500 usec     0 deg
    // cmpx = 24000 = 2000 usec    90 deg
    // cmpx = 30000 = 2500 usec   180 deg

//        csr_write_simple(1, CSR_PWM0_BASE + 0xff10L);           // enable clock via gate
        csr_write_simple(disable, CSR_PWM0_BASE + 0x14L);       // Disable Timer
        csr_write_simple(reload, CSR_PWM0_BASE + 0x04L);        // Reload
        csr_write_simple(prescale, CSR_PWM0_BASE + 0x08L);      // Set prescale
        csr_write_simple(config, CSR_PWM0_BASE + 0x18L);        // Set Cfg
        csr_write_simple(p0_ticks, CSR_PWM0_BASE + 0x0cL);      // Cmpx
        csr_write_simple(cmpy_value, CSR_PWM0_BASE + 0x10L);    // Cmpy
        csr_write_simple(match, CSR_PWM0_BASE + 0x1cL);         // pwm0cfg - match high
        csr_write_simple(match, CSR_PWM0_BASE + 0x20L);         // pwm1cfg - match high
        csr_write_simple(enable, CSR_PWM0_BASE + 0x14L);        // Ctrl - Enable PWM

//        csr_write_simple(1, CSR_PWM0_BASE + 0xff10L);         // enable clock gating
        csr_write_simple(disable, CSR_PWM1_BASE + 0x14L);       // Disable Timer
        csr_write_simple(reload, CSR_PWM1_BASE + 0x04L);        // Reload
        csr_write_simple(prescale, CSR_PWM1_BASE + 0x08L);      // Set prescale
        csr_write_simple(config, CSR_PWM1_BASE + 0x18L);        // Set Cfg
        csr_write_simple(p1_ticks, CSR_PWM1_BASE + 0x0cL);      // Cmpx
        csr_write_simple(cmpy_value, CSR_PWM1_BASE + 0x10L);    // Cmpy
        csr_write_simple(match, CSR_PWM1_BASE + 0x1cL);         // pwm0cfg - match high
        csr_write_simple(match, CSR_PWM1_BASE + 0x20L);         // pwm1cfg - match high
        csr_write_simple(enable, CSR_PWM1_BASE + 0x14L);        // Ctrl - Enable PWM

//        csr_write_simple(1, CSR_PWM0_BASE + 0xff10L);         // enable clock gating
        csr_write_simple(disable, CSR_PWM2_BASE + 0x14L);       // Disable Timer
        csr_write_simple(reload, CSR_PWM2_BASE + 0x04L);        // Reload
        csr_write_simple(prescale, CSR_PWM2_BASE + 0x08L);      // Set prescale
        csr_write_simple(config, CSR_PWM2_BASE + 0x18L);        // Set Cfg
        csr_write_simple(p2_ticks, CSR_PWM2_BASE + 0x0cL);      // Cmpx
        csr_write_simple(cmpy_value, CSR_PWM2_BASE + 0x10L);    // Cmpy
        csr_write_simple(match, CSR_PWM2_BASE + 0x1cL);         // pwm0cfg - match high
        csr_write_simple(match, CSR_PWM2_BASE + 0x20L);         // pwm1cfg - match high
        csr_write_simple(enable, CSR_PWM2_BASE + 0x14L);        // Ctrl - Enable PWM

//        csr_write_simple(1, CSR_PWM0_BASE + 0xff10L);         // enable clock gating
        csr_write_simple(disable, CSR_PWM3_BASE + 0x14L);       // Disable Timer
        csr_write_simple(reload, CSR_PWM3_BASE + 0x04L);        // Reload
        csr_write_simple(prescale, CSR_PWM3_BASE + 0x08L);      // Set prescale
        csr_write_simple(config, CSR_PWM3_BASE + 0x18L);        // Set Cfg
        csr_write_simple(p3_ticks, CSR_PWM3_BASE + 0x0cL);      // Cmpx
        csr_write_simple(cmpy_value, CSR_PWM3_BASE + 0x10L);    // Cmpy
        csr_write_simple(match, CSR_PWM3_BASE + 0x1cL);         // pwm0cfg - match high
        csr_write_simple(match, CSR_PWM3_BASE + 0x20L);         // pwm1cfg - match high
        csr_write_simple(enable, CSR_PWM3_BASE + 0x14L);        // Ctrl - Enable PWM

}

void set_servo_position(int ticks)
{
    config_pwm_ticks(ticks, ticks, ticks, ticks);
    led_bar(led_bar_for_ticks(ticks));
    reg_gpio_out = !reg_gpio_out;
}

void main()
{
    unsigned int inputs;
    unsigned int ab;
    unsigned int last_ab = 3;
    unsigned int auto_pose = 0;
    unsigned int auto_dwell = 0;
    unsigned int swt_raw;
    unsigned int swt_settle = 0;
    unsigned int swt = 0;
    int quarter_steps = 0;
    int ticks = SERVO_MID_TICKS;
    int manual_mode;
    int last_manual_mode = -1;

    reg_gpio_mode1 = 1;
    reg_gpio_mode0 = 0;
    reg_gpio_ien = 1;
    reg_gpio_oe = 1;

    reg_wb_enable  = 1;

    configure_io();

    reg_uart_enable = 1;

    // Configure All LA probes as inputs to the cpu
	reg_la0_oenb = reg_la0_iena = 0x00000000;    // [31:0]
	reg_la1_oenb = reg_la1_iena = 0x00000000;    // [63:32]
	reg_la2_oenb = reg_la2_iena = 0x00000000;    // [95:64]
	reg_la3_oenb = reg_la3_iena = 0x00000000;    // [127:96]

//    print("Hello World !!\n");

    reg_gpio_out = 0; // ON

    // clock = 12MHz period = 83.33 nsec
    // reload = 240000 = 20 ms = 50 Hz
    // cmpx =  6000 =  500 usec  -180 deg
    // cmpx = 12000 = 1000 usec   -90 deg
    // cmpx = 18000 = 1500 usec     0 deg
    // cmpx = 24000 = 2000 usec    90 deg
    // cmpx = 30000 = 2500 usec   180 deg

    set_servo_position(ticks);

	while (1) {
        inputs = reg_mprj_datal;
        ab = (inputs >> 16) & 0x3;

        // SWT on selects interactive encoder mode; off preserves auto-sweep.
        swt_raw = ((inputs & ENC_SWT_MASK) != 0);
        if (swt_raw == swt) {
            swt_settle = 0;
        } else if (++swt_settle >= SWT_DEBOUNCE_POLLS) {
            swt_settle = 0;
            swt = swt_raw;
        }

        manual_mode = (int)swt;
        if (manual_mode != last_manual_mode) {
            last_manual_mode = manual_mode;
            last_ab = ab;
            quarter_steps = 0;
            auto_dwell = 0;

            if (!manual_mode) {
                auto_pose = 0;
                ticks = SERVO_MID_TICKS;
                set_servo_position(ticks);
            }
        }

        if (manual_mode) {
            // The shaft button is active high and recenters all four servos.
            if ((inputs & ENC_BTN_MASK) != 0) {
                quarter_steps = 0;
                last_ab = ab;
                if (ticks != SERVO_MID_TICKS) {
                    ticks = SERVO_MID_TICKS;
                    set_servo_position(ticks);
                }
            } else {
                quarter_steps += encoder_delta((last_ab << 2) | ab);
                last_ab = ab;

                if (quarter_steps >= 4) {
                    quarter_steps = 0;
                    if (ticks < SERVO_MAX_TICKS) {
                        ticks += SERVO_STEP_TICKS;
                        if (ticks > SERVO_MAX_TICKS)
                            ticks = SERVO_MAX_TICKS;
                        set_servo_position(ticks);
                    }
                } else if (quarter_steps <= -4) {
                    quarter_steps = 0;
                    if (ticks > SERVO_MIN_TICKS) {
                        ticks -= SERVO_STEP_TICKS;
                        if (ticks < SERVO_MIN_TICKS)
                            ticks = SERVO_MIN_TICKS;
                        set_servo_position(ticks);
                    }
                }
            }
        } else if (++auto_dwell >= AUTO_DWELL_POLLS) {
            auto_dwell = 0;
            auto_pose = (auto_pose + 1) & 0x3;

            if (auto_pose == 1)
                ticks = SERVO_MIN_TICKS;
            else if (auto_pose == 3)
                ticks = SERVO_MAX_TICKS;
            else
                ticks = SERVO_MID_TICKS;

            set_servo_position(ticks);
        }

		delay(manual_mode ? POLL_TICKS_MANUAL : POLL_TICKS_AUTO);
    }


}

