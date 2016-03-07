/*
 *  Copyright (c) 2015 Robin Callender. All Rights Reserved.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>

#include "nordic_common.h"
#include "nrf.h"
#include "nrf_error.h"
#include "nrf_soc.h"
#include "nrf_mbr.h"
#include "nrf_gpio.h"
#include "softdevice_handler.h"
#include "ble.h"
#include "ble_dis.h"
#include "ble_hci.h"
#include "app_error.h"
#include "app_scheduler.h"
#include "app_util_platform.h"
#include "app_timer_appsh.h"

#include "boards.h"
#include "bootloader.h"
#include "bootloader_util.h"
#include "dfu_transport.h"
#include "uart.h"
#include "pstorage_platform.h"
#include "dbglog.h"

/*
 *  Device Information defines
 */
#define DEVICE_NAME          "Eddystone"
#define MANUFACTURER_NAME    "Callender Consulting"
#define MODEL_NUM            "Model-1"
#define MANUFACTURER_ID      0x1122334455
#define FIRMWARE_REV_NAME    "SD 11.0"
#define SOFTWARE_REV_NAME    "Built on "__DATE__ " at " __TIME__

#define LFCLKSRC_OPTION      NRF_CLOCK_LFCLKSRC_XTAL_20_PPM

#define HARDWARE_REV_NAME   "PCA10036"

/*
 *  Include or not the service_changed characteristic. 
 *  if not enabled, the server's database cannot be changed for 
 *  the lifetime of the device.
 */
#define IS_SRVC_CHANGED_CHARACT_PRESENT 1

/*
 *  Button used to enter SW update mode.
 */
#if defined(BUTTON_SUPPORT)
#define BOOTLOADER_BUTTON               BUTTON_1
#endif

/*
 *  Led used to indicate that DFU is active. 
 */
#define UPDATE_IN_PROGRESS_LED          LED_1

/*
 *  Value of the RTC1 PRESCALER register. 
 */
#define APP_TIMER_PRESCALER             0

/*
 *  Maximum number of simultaneously created timers.
 */
#define APP_TIMER_MAX_TIMERS            3

/*
 *  Size of timer operation queues.
 */
#define APP_TIMER_OP_QUEUE_SIZE         8

/*
 *  Debounce delay from a GPIOTE event until a button is reported as 
 *  pushed (in number of timer ticks).
 */
#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)

/*
 *  Maximum size of scheduler events.
 */
#define SCHED_MAX_EVENT_DATA_SIZE       MAX(APP_TIMER_SCHED_EVT_SIZE, 0)

/*
 *  Maximum number of events in the scheduler queue.
 */
#define SCHED_QUEUE_SIZE                20

#if 0  // FIXME what is new way to do this ?
/*
 *  Function for error handling, which is called when an error has occurred. 
 *
 *  warning This handler is an example only and does not fit a final product. You need to analyze 
 *          how your product is supposed to react in case of error.
 *
 *  [in] error_code  Error code supplied to the handler.
 *  [in] line_num    Line number where the handler is called.
 *  [in] p_file_name Pointer to the file name. 
 */
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{    
    PRINTF("app_error: err(0x%x)  line(%u) in %s\n", 
           (unsigned)error_code, (unsigned)line_num, p_file_name);

#if defined(DEBUG)
    __disable_irq();
    __BKPT(0);
    while (1) { /* spin */}
#else
    /* On assert, the system can only recover with a reset. */
    NVIC_SystemReset();
#endif
}
#endif

/*
 *  Callback function for asserts in the SoftDevice.
 *
 *  This function will be called in case of an assert in the SoftDevice.
 *
 *  warning This handler is an example only and does not fit a final product. 
 *          You need to analyze  how your product is supposed to react in 
 *          case of Assert.
 *  warning On assert from the SoftDevice, the system can only recover on reset.
 *
 *  param[in] line_num    Line number of the failing ASSERT call.
 *  param[in] file_name   File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(0xDEADBEEF, line_num, p_file_name);
}


/*
 *  Function for initialization of LEDs.
 */
static void leds_init(void)
{
    nrf_gpio_cfg_output(UPDATE_IN_PROGRESS_LED);
    nrf_gpio_pin_set(UPDATE_IN_PROGRESS_LED);
}


/*
 *  Function for initializing the timer handler module (app_timer).
 */
static void timers_init(void)
{
    // Initialize timer module, making it use the scheduler.
    APP_TIMER_APPSH_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, true);
}

#if defined(BUTTON_SUPPORT)
/*
 *  Function for initializing the button module.
 */
static void buttons_init(void)
 {
    nrf_gpio_cfg_sense_input(BOOTLOADER_BUTTON,
                             PULLUP,
                             NRF_GPIO_PIN_SENSE_LOW);

 }
#endif

/*
 *  Function for dispatching a BLE stack event to all modules with a 
 *  BLE stack event handler.
 *
 *  This function is called from the scheduler in the main loop after a BLE stack
 *          event has been received.
 *
 *  param[in]   p_ble_evt   Bluetooth stack event.
 */
static void sys_evt_dispatch(uint32_t event)
{
    pstorage_sys_event_handler(event);
}


/*
 *  Function for initializing the BLE stack.
 *
 *  Initializes the SoftDevice and the BLE event interrupt.
 *
 *  param[in] init_softdevice  true if SoftDevice should be initialized. 
 *                             The SoftDevice must only  be initialized 
 *                             if a chip reset has occured. Soft reset from 
 *                             application must not reinitialize the SoftDevice.
 */
static void ble_stack_init(bool init_softdevice)
{
    uint32_t         err_code;
    uint32_t         app_ram_base;
    sd_mbr_command_t com = {SD_MBR_COMMAND_INIT_SD, };

    if (init_softdevice)
    {
        err_code = sd_mbr_command(&com);
        APP_ERROR_CHECK(err_code);
    }

    err_code = sd_softdevice_vector_table_base_set(BOOTLOADER_REGION_START);
    APP_ERROR_CHECK(err_code);

    /* Initialize the SoftDevice handler module. See specific board header file */
    SOFTDEVICE_HANDLER_INIT(LFCLKSRC_OPTION, NULL);

    // Enable BLE stack 
    ble_enable_params_t ble_enable_params;
    memset(&ble_enable_params, 0, sizeof(ble_enable_params));
    ble_enable_params.gatts_enable_params.service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT;
    err_code = sd_ble_enable(&ble_enable_params, &app_ram_base);
    APP_ERROR_CHECK(err_code);

    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}


/*
 *  Function for event scheduler initialization.
 */
static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}

/*
 *  Function for initializing the Device Information Service.
 */
static void dis_init(void)
{
    uint32_t       err_code;
    ble_dis_init_t dis_init;

    // Initialize Device Information Service
    memset(&dis_init, 0, sizeof(dis_init));

    ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, MANUFACTURER_NAME);

    ble_srv_ascii_to_utf8(&dis_init.hw_rev_str, HARDWARE_REV_NAME);
    ble_srv_ascii_to_utf8(&dis_init.fw_rev_str, FIRMWARE_REV_NAME);
    ble_srv_ascii_to_utf8(&dis_init.sw_rev_str, SOFTWARE_REV_NAME);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dis_init.dis_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init.dis_attr_md.write_perm);

    err_code = ble_dis_init(&dis_init);
    APP_ERROR_CHECK(err_code);

    PUTS("Device Info Service");
}

/*
 *  Function for bootloader main entry.
 */
int main(void)
{
    uint32_t err_code;
    bool     dfu_start = false;
    bool     app_reset = (NRF_POWER->GPREGRET == BOOTLOADER_DFU_START);

    if (app_reset) {
        NRF_POWER->GPREGRET = 0;
    }

    leds_init();

#if defined(DBGLOG_SUPPORT)
    uart_init();
#endif

    PUTS("\nBootloader *** " __DATE__ "  " __TIME__ " ***");

    // This check ensures that the defined fields in the bootloader 
    // corresponds with actual setting in the nRF51 chip.
    APP_ERROR_CHECK_BOOL(*((uint32_t *)NRF_UICR_BOOT_START_ADDRESS) == BOOTLOADER_REGION_START);
    APP_ERROR_CHECK_BOOL(NRF_FICR->CODEPAGESIZE == CODE_PAGE_SIZE);

    // Initialize.
    timers_init();

#if defined(BUTTON_SUPPORT)
    buttons_init();
#endif

    (void)bootloader_init();

    if (bootloader_dfu_sd_in_progress()) {
        nrf_gpio_pin_clear(UPDATE_IN_PROGRESS_LED);

        err_code = bootloader_dfu_sd_update_continue();
        APP_ERROR_CHECK(err_code);

        ble_stack_init(!app_reset);
        scheduler_init();

        err_code = bootloader_dfu_sd_update_finalize();
        APP_ERROR_CHECK(err_code);

        nrf_gpio_pin_set(UPDATE_IN_PROGRESS_LED);
    }
    else {
        // If stack is present then continue initialization of bootloader.
        ble_stack_init(!app_reset);
        scheduler_init();
        dis_init();
    }

    dfu_start  = app_reset;

#if defined(BUTTON_SUPPORT)
    dfu_start |= ((nrf_gpio_pin_read(BOOTLOADER_BUTTON) == 0) ? true: false);
#endif

    if (dfu_start || (!bootloader_app_is_valid(DFU_BANK_0_REGION_START))) {

        nrf_gpio_pin_clear(UPDATE_IN_PROGRESS_LED);

        PUTS("Start DFU");

        // Initiate an update of the firmware.
        err_code = bootloader_dfu_start();
        APP_ERROR_CHECK(err_code);

        nrf_gpio_pin_set(UPDATE_IN_PROGRESS_LED);
    }

    if (bootloader_app_is_valid(DFU_BANK_0_REGION_START) && !bootloader_dfu_sd_in_progress()) {

        PUTS("Start App");

        // Select a bank region to use as application region.
        // @note: Only applications running from DFU_BANK_0_REGION_START is supported.
        bootloader_app_start(DFU_BANK_0_REGION_START);
    }

    NVIC_SystemReset();
}
