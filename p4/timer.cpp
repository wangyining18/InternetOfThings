#include "application.h" 
#include "nrfx.h"
#include "nrfx_timer.h"	


int ledD7 = D7;



bool LED_on = false;


const nrfx_timer_t TIMER_LED = NRFX_TIMER_INSTANCE(3);	



//create a timer interrupt handler
void timer_handler( nrf_timer_event_t event_type, void *p_context){
    if(LED_on){
        digitalWrite(ledD7, LOW);
        LED_on = false;
    } else {
        digitalWrite(ledD7, HIGH);
        LED_on = true;
    }
}



SYSTEM_MODE(SEMI_AUTOMATIC);


void setup() {
    Serial.begin(9600);
    pinMode(ledD7, OUTPUT);

    const nrfx_timer_config_t TIMER_CFG = {
        .frequency =          NRF_TIMER_FREQ_1MHz,
        .mode =               NRF_TIMER_MODE_TIMER,
        .bit_width =          NRF_TIMER_BIT_WIDTH_32,
        .interrupt_priority = NRFX_TIMER_DEFAULT_CONFIG_IRQ_PRIORITY,
        .p_context =          NULL
    };


    nrfx_timer_init( &TIMER_LED, &TIMER_CFG, timer_handler);

    uint32_t time_ms = 500;
    uint32_t time_ticks = nrfx_timer_ms_to_ticks(&TIMER_LED, time_ms);
    Serial.printf("%s:%d timer ticks: %d \r\n", __FUNCTION__, __LINE__, time_ticks);

    
    nrfx_timer_extended_compare (&TIMER_LED, 
                                 NRF_TIMER_CC_CHANNEL0, 
                                 time_ticks, 
                                 NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, 
                                 true);


    nrfx_timer_enable(&TIMER_LED);
}



void loop() {
	
}
