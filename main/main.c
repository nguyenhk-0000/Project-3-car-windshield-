#include "driver/gpio.h"
#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "driver/ledc.h"
#include <hd44780.h>
#include <esp_idf_lib_helpers.h>
#include <inttypes.h>
#include <stdio.h>



// Defining GPIO pins, loop delay
#define LED_green GPIO_NUM_10
#define LED_red GPIO_NUM_12
#define ALARM GPIO_NUM_1
#define BUTTON_DS GPIO_NUM_4
#define BUTTON_PS GPIO_NUM_5
#define BUTTON_DSSB GPIO_NUM_6
#define BUTTON_PSSB GPIO_NUM_7
#define BUTTON_IG GPIO_NUM_15
#define LOOP_DELAY_MS 10

//Defining motor control parameters
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          18
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits

//Set the PWM signal frequency required by servo motor
#define LEDC_FREQUENCY          50 // Frequency in Hertz. 


//Defining ADC parameters 
#define ADC_CHANNEL_GPIO16   ADC_CHANNEL_5 // GPIO16 is ADC Channel 5 
#define ADC_CHANNEL_GPIO17   ADC_CHANNEL_6 // GPIO17 is ADC Channel 6  
#define ADC_ATTEN           ADC_ATTEN_DB_12 // 0-3.6V
#define BITWIDTH            ADC_BITWIDTH_12 // 12 bit width


// Defining states for delay control
#define SHORT  1000 
#define MEDIUM 3000
#define LONG   5000
#define duty_update 10 //defines how often the duty cycle is updated when changing speed modes
#define duty_update_ms duty_update/1000 //duty_update in ms

//Calculate the values for the minimum (0.75ms) and maximum (2.25) servo pulse widths
#define LEDC_DUTY_MIN           (350) // Set duty to lowest.
#define LEDC_DUTY_MAX           (717) // Set duty to highest.
#define LEDC_DUTY_DIFFERENCE      (LEDC_DUTY_MAX - LEDC_DUTY_MIN) // Difference between max and min duty
#define LEDC_DUTY_INCREMENT       (LEDC_DUTY_DIFFERENCE * duty_update_ms) // Duty increment for each update based on desired update frequency   


// Defining states for speed control
#define OFF 0
#define HIGH                LEDC_DUTY_INCREMENT/0.6   // Calculate 25rpm --> check the README on detail for calculation
#define LOW                 LEDC_DUTY_INCREMENT/1.5    // Calculate 10 rpm --> check the README on detail for calculation
                                                      

int duty = 0; //variable to control duty cylce of wipers

adc_oneshot_unit_handle_t adc2_handle;
adc_cali_handle_t cali_gpio16;
adc_cali_handle_t cali_gpio17;

hd44780_t lcd; //intializing lcd variable to use it globally

// initializing the LEDC 
static void example_ledc_init(void)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 50 Hz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
ledc_channel_config(&ledc_channel);
}


int read_adc_channel(adc_channel_t channel, adc_cali_handle_t cali_handle);
int check_speed(int mv17);
int check_delay(int mv16);
void wipe_wipers (int duty, int speed, int delay, int mv17);
void check_for_engine_fail (void);


// Initializing flags for printing correctly (only once per condition)
   bool welcomeflag = true;
   bool enginestartedflag = true;
   bool ignitioninhibitflag = true;
   bool drivseatflag = true;
   bool passeatflag = true;
   bool drivbeltflag = true;
   bool pasbeltflag = true;
   bool lstate = false; // green light state to indicate when all of the checks are completed
   bool rlstate = false;// Red light state to indicate when the engine has been put on correctly 
   bool ignition_pushed = false;
   volatile bool interm_mode = false; 

void wiper_task(void *pvParameters) {

    int mv16;
    int mv17;
    int speed;
    int delay = 0; // Initializing the delay to 0 incase when the windshield is not in intermittent mode 
    
    while(1) {  // Tasks have their own infinite loop
        
        if(rlstate == true) {  // Only run if engine is on
            
            // Read potentiometers
            mv17 = read_adc_channel(ADC_CHANNEL_GPIO17, cali_gpio17);
            speed = check_speed(mv17);

            if (interm_mode == true){
            mv16 = read_adc_channel(ADC_CHANNEL_GPIO16, cali_gpio16);
            delay = check_delay(mv16);
            }

            if(speed == OFF) {
                // OFF mode - wipers at 0 degrees
                ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY_MIN);
                ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
            } else {
                wipe_wipers(duty, speed, delay, mv17);    
            }
        } else {
            // Engine off - ensure wipers at 0
            ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY_MIN);
            ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));  // Small delay
    }
}
void ignition_check_task(void *pvParameters) {
    while(1){
        if(!gpio_get_level(BUTTON_IG)){
            ignition_pushed = !ignition_pushed;
            vTaskDelay(pdMS_TO_TICKS(300)); // debounce
        }
        vTaskDelay(pdMS_TO_TICKS(20)); // polling delay - always runs
    }
}


void app_main(void)
{
// Initializing the LCD display
    lcd = (hd44780_t) 
    {
        .write_cb = NULL,
        .font = HD44780_FONT_5X8,
        .lines = 2,
        .pins = {
            .rs = GPIO_NUM_38,
            .e  = GPIO_NUM_37,
            .d4 = GPIO_NUM_36,
            .d5 = GPIO_NUM_35,
            .d6 = GPIO_NUM_48,
            .d7 = GPIO_NUM_47,
            .bl = HD44780_NOT_USED
        }
    };

    ESP_ERROR_CHECK(hd44780_init(&lcd));

    // initializing the ADC channel 
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_2,
    };
    adc_oneshot_new_unit(&init_config, &adc2_handle);

   
    adc_oneshot_chan_cfg_t config = { // Channel configuration
        .atten = ADC_ATTEN,
        .bitwidth = BITWIDTH
    };
    adc_oneshot_config_channel(adc2_handle, ADC_CHANNEL_GPIO16, &config); // Configure ADC channel for GPIO16
    adc_oneshot_config_channel(adc2_handle, ADC_CHANNEL_GPIO17, &config); // Configure ADC channel for GPIO17

    
    adc_cali_curve_fitting_config_t cali16_cfg = { // Calibration configuration for GPIO16
        .unit_id = ADC_UNIT_2,
        .chan = ADC_CHANNEL_GPIO16,
        .atten = ADC_ATTEN,
        .bitwidth = BITWIDTH
    };
    adc_cali_curve_fitting_config_t cali17_cfg = { // Calibration configuration for GPIO17
        .unit_id = ADC_UNIT_2,
        .chan = ADC_CHANNEL_GPIO17,
        .atten = ADC_ATTEN,
        .bitwidth = BITWIDTH
    };
    adc_cali_create_scheme_curve_fitting(&cali16_cfg, &cali_gpio16);
    adc_cali_create_scheme_curve_fitting(&cali17_cfg, &cali_gpio17);

    // Initializing pins, setting directions and pull-ups, booleans, flags
   gpio_reset_pin(LED_green);
   gpio_set_direction(LED_green, GPIO_MODE_OUTPUT);


   gpio_reset_pin(LED_red);
   gpio_set_direction(LED_red, GPIO_MODE_OUTPUT);


   gpio_reset_pin(ALARM);
   gpio_set_direction(ALARM, GPIO_MODE_OUTPUT);


   gpio_reset_pin(BUTTON_DS);
   gpio_set_direction(BUTTON_DS, GPIO_MODE_INPUT);


   gpio_reset_pin(BUTTON_PS);
   gpio_set_direction(BUTTON_PS, GPIO_MODE_INPUT);


   gpio_reset_pin(BUTTON_DSSB);
   gpio_set_direction(BUTTON_DSSB, GPIO_MODE_INPUT);


   gpio_reset_pin(BUTTON_PSSB);
   gpio_set_direction(BUTTON_PSSB, GPIO_MODE_INPUT);

   gpio_reset_pin(BUTTON_IG);
   gpio_set_direction(BUTTON_IG, GPIO_MODE_INPUT);


    // Set the LEDC peripheral configuration
    example_ledc_init();
    // Set duty to 3.75% (0 degrees)
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY_MIN);
    // Update duty to apply the new value
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

   gpio_set_level(LED_green, 0);
   gpio_set_level(LED_red, 0);
   gpio_set_level(ALARM, 0);
   gpio_pullup_en(BUTTON_DS);
   gpio_pullup_en(BUTTON_PS);
   gpio_pullup_en(BUTTON_DSSB);
   gpio_pullup_en(BUTTON_PSSB);
   gpio_pullup_en(BUTTON_IG);

xTaskCreate(wiper_task, "wiper", 4096, NULL, 5, NULL); // Initializing the wiper task
xTaskCreate(ignition_check_task, "ignition", 4096, NULL, 5, NULL); // Initializing the ignition task 

  // Main loop



  while (1) {
    

   if (!gpio_get_level(BUTTON_DS) && welcomeflag == true) { // Initial welcome message when driver seat is occupied
           printf("Welcome to enhanced alarm system model 218-W25\n");
           welcomeflag = false;
   }


   if (!gpio_get_level(BUTTON_DS) && !gpio_get_level(BUTTON_PS) && !gpio_get_level(BUTTON_DSSB) \
   && !gpio_get_level(BUTTON_PSSB) && rlstate == false) { // All conditions met for enabling engine start
           gpio_set_level(LED_green, 1); // Turn on green LED
           gpio_set_level(ALARM,0); // Disable alarm in case previously enabled
           lstate = true;
       }
   else {
           gpio_set_level(LED_green, 0); // Keep green LED off if conditions not met
           lstate = false;
       }

if (ignition_pushed) {

    if (rlstate == false) {
        // Engine is OFF - try to start
        if (lstate == true) {
            // Conditions met - start engine
            gpio_set_level(LED_green, 0);
            gpio_set_level(LED_red, 1);
            rlstate = true;
            ignition_pushed = false;
            
            if(enginestartedflag == true) {
                printf("Engine Started!\n");
                enginestartedflag = false;
            }
        } else {
            // Conditions NOT met
            check_for_engine_fail();
        }
    } 
    else {  // rlstate == true
        // Engine is ON - turn it off
        printf("Engine Off!\n");
        gpio_set_level(LED_red, 0);
        rlstate = false;
        enginestartedflag = true;
        ignitioninhibitflag = true;
        drivseatflag = true;
        passeatflag = true;
        drivbeltflag = true;
        pasbeltflag = true;
        gpio_set_level(ALARM, 0);
        gpio_set_level(LED_green, 0);
        hd44780_clear(&lcd);
        lstate = false;
        ignition_pushed = false;
    }
}
 
vTaskDelay(pdMS_TO_TICKS(10));
  }

}

// Initialing function for reading ADC channel 

int read_adc_channel(adc_channel_t channel, adc_cali_handle_t cali_handle){
    int raw, mv;
    adc_oneshot_read(adc2_handle, channel, &raw);
    adc_cali_raw_to_voltage(cali_handle, raw, &mv);
    return mv;
}

// displaying the speed that the user selected for the windshield on the LCD display
int check_speed(int mv17){
    hd44780_clear(&lcd);
    if (mv17 >= 0 && mv17 < 900) { //range for speed mode off
        hd44780_gotoxy(&lcd, 0, 0);
        hd44780_puts(&lcd, "SPEED: OFF ");
        interm_mode = false;
        return OFF;
   
    } else if (mv17 >= 900 && mv17 < 1800) { //range speed mode high
        hd44780_gotoxy(&lcd, 0, 0);
        hd44780_puts(&lcd, "SPEED: HIGH ");
        interm_mode = false;
        return HIGH;

    }
    if (mv17 >= 1800 && mv17 < 2700) { //range for when speed mode INTERMIITTENT
         hd44780_gotoxy(&lcd, 0, 0);
         hd44780_puts(&lcd, "SPEED: INTERMITTENT");
        interm_mode = true;
        return LOW;
    }
    if (mv17 >= 2700){ //range for speed mode on low 
        hd44780_gotoxy(&lcd, 0, 0);
        hd44780_puts(&lcd, "SPEED: LOW ");
        interm_mode = false;
        return LOW; 
    }
    else{
        interm_mode = false;
        return 0;
    }

    }

// Displaying the delays that the user selected for the windshield while in intermittent mode on the LCD display 
int check_delay(int mv16){
if (mv16 >= 0 && mv16 < 1060 ) { //range for when light mode is set to off
    hd44780_gotoxy(&lcd, 0, 1);
    hd44780_puts(&lcd, "DELAY: SHORT ");
    return SHORT;
} else if (mv16 >= 1060 && mv16 < 2120) { //range for when light mode is set to on
    hd44780_gotoxy(&lcd, 0, 1);
    hd44780_puts(&lcd, "DELAY: MEDIUM ");
    return MEDIUM;
} else if (mv16 >= 2120 && mv16 < 3180) { //final range for when mode is set to auto
    hd44780_gotoxy(&lcd, 0, 1);
    hd44780_puts(&lcd, "DELAY: LONG ");
    return LONG;
}
else {
    return 0; //default delay for other speed modes
}

}

// Windshield wiping motion
void wipe_wipers (int duty, int speed, int delay, int mv17){
  
    for (int duty = LEDC_DUTY_MIN; duty <= LEDC_DUTY_MAX; duty += speed) {
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
        vTaskDelay(duty_update/portTICK_PERIOD_MS);
    }
    for (int duty = LEDC_DUTY_MAX; duty >= LEDC_DUTY_MIN; duty -= speed) {
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
        vTaskDelay(duty_update /portTICK_PERIOD_MS);
    }
    if (mv17 >= 1800 && mv17 < 2700){
    vTaskDelay(delay/portTICK_PERIOD_MS);
    }
    }
// The alarm system 
void check_for_engine_fail(void) {
    gpio_set_level(ALARM, 1); // Sound alarm
    ignition_pushed = false; //reset ignition push variable
    
    if(ignitioninhibitflag == true) {
        printf("Ignition Inhibited!\n");
        ignitioninhibitflag = false;
    }
    if(gpio_get_level(BUTTON_DS) && drivseatflag == true) {
        printf("Driver seat not occupied!\n");
        drivseatflag = false;
    }
    if(gpio_get_level(BUTTON_PS) && passeatflag == true) {
        printf("Passenger seat not occupied!\n");
        passeatflag = false;
    }
    if(gpio_get_level(BUTTON_DSSB) && drivbeltflag == true) {
        printf("Driver seatbelt not fastened!\n");
        drivbeltflag = false;
    }
    if(gpio_get_level(BUTTON_PSSB) && pasbeltflag == true) {
        printf("Passenger seatbelt not fastened!\n");
        pasbeltflag = false;
    }
}
