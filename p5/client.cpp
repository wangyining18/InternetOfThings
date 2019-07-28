#include "nrfx.h"
#include "nrfx_ppi.h"
#include "nrfx_saadc.h"
#include "nrfx_timer.h"
#include "stdio.h"
#include "string.h"
#include "application.h"


const uint16_t analogPin = A4; 

UDP Udp;
IPAddress remoteIP(149,160,196,228);
unsigned int localPort = 8888;
int port = 23333;

bool button = false;

#define TIMER_INSTANCE 3
#define TIMER_TICK_US 62 // this shoud contain the proper time for the timer
#define BUF0_SIZE 256
#define BUF1_SIZE 256

int16_t BUF0 [BUF0_SIZE];
int16_t BUF1 [BUF1_SIZE];

int16_t udpArr[16000];
int csize = 0;

const nrfx_timer_t TIMER_LED = NRFX_TIMER_INSTANCE(TIMER_INSTANCE);


void timer_handler(nrf_timer_event_t event_type, void *p_context)
{
}


uint32_t timer_init( const nrfx_timer_t timer_id )
{

  uint32_t time_us = TIMER_TICK_US; // define the appropiate time for the timer triggers
  uint32_t err_code = NRF_SUCCESS;
  uint32_t time_ticks;

  const nrfx_timer_config_t TIMER_CFG = {
					 .frequency  = NRF_TIMER_FREQ_1MHz,
					 .mode       = NRF_TIMER_MODE_TIMER,
					 .bit_width  = NRF_TIMER_BIT_WIDTH_32,
					 .interrupt_priority = NRFX_TIMER_DEFAULT_CONFIG_IRQ_PRIORITY,
					 .p_context  = NULL
  };

  err_code = nrfx_timer_init(&TIMER_LED, &TIMER_CFG, timer_handler);

  time_ticks =  nrfx_timer_ms_to_ticks( &TIMER_LED, time_us);

  nrfx_timer_extended_compare (&TIMER_LED, NRF_TIMER_CC_CHANNEL0, time_ticks,
			       NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);

  return nrfx_timer_compare_event_address_get( &TIMER_LED, NRF_TIMER_CC_CHANNEL0);
}

/* function takes 2 arguments the address of the configured timer (timer_addr), */
/*   the address of the configured ADC (adc_addr) and configures the ADC sampling */
/*   to be driven by the timer output */
void ppi_init( const uint32_t timer_addr, const uint32_t adc_addr)
{
  uint32_t err_code = NRF_SUCCESS;
  nrf_ppi_channel_t ppi_channel;

  err_code = nrfx_ppi_channel_alloc (&ppi_channel);
  SPARK_ASSERT( err_code == NRF_SUCCESS);
      
  err_code = nrfx_ppi_channel_assign (ppi_channel, timer_addr, adc_addr);
  SPARK_ASSERT( err_code == NRF_SUCCESS);
  
  err_code =nrfx_ppi_channel_enable (ppi_channel);
  SPARK_ASSERT( err_code == NRF_SUCCESS);
}

void adc_handler( const nrfx_saadc_evt_t * event_type)
{
  if (event_type->type == NRFX_SAADC_EVT_DONE)
    {
      nrf_saadc_value_t * buf = event_type->data.done.p_buffer;
      uint16_t buf_sz = event_type->data.done.size;
      
      if (csize < 16000){
        memcpy(& udpArr[csize], buf, buf_sz);
        csize += buf_sz;
      }
      /*else{
        csize = 0;
      }*/
      
    } else {
    SPARK_ASSERT(0);
  }
}

/* function for configuring the adc */
/* takes the input pin as input argument */
uint32_t adc_init( const uint16_t pin)
{
  uint32_t err_code = NRF_SUCCESS;
  
  const nrfx_saadc_config_t saadc_config =
    {
     .resolution         = NRF_SAADC_RESOLUTION_12BIT,
     .oversample         = NRF_SAADC_OVERSAMPLE_DISABLED,
     .interrupt_priority = NRFX_SAADC_CONFIG_IRQ_PRIORITY
    };

  NRF5x_Pin_Info *PIN_MAP = HAL_Pin_Map();

  nrf_saadc_input_t nrf_adc_channel = NRF_SAADC_INPUT_AIN0;
  switch (PIN_MAP[pin].adc_channel)
    {
    case 0: nrf_adc_channel = NRF_SAADC_INPUT_AIN0; break;
    case 1: nrf_adc_channel = NRF_SAADC_INPUT_AIN1; break;
    case 2: nrf_adc_channel = NRF_SAADC_INPUT_AIN2; break;
    case 3: nrf_adc_channel = NRF_SAADC_INPUT_AIN3; break;
    case 4: nrf_adc_channel = NRF_SAADC_INPUT_AIN4; break;
    case 5: nrf_adc_channel = NRF_SAADC_INPUT_AIN5; break;
    case 6: nrf_adc_channel = NRF_SAADC_INPUT_AIN6; break;
    case 7: nrf_adc_channel = NRF_SAADC_INPUT_AIN7; break;
    default: SPARK_ASSERT(0);
    }

  //make sure pin is configured for Analog Input
  if (PIN_MAP[pin].pin_func != PF_NONE && PIN_MAP[pin].pin_func != PF_DIO) {
    SPARK_ASSERT(0);
  }

  //Single ended, negative input to ADC shorted to GND.
  nrf_saadc_channel_config_t channel_config = {
					       .resistor_p = NRF_SAADC_RESISTOR_DISABLED,
					       .resistor_n = NRF_SAADC_RESISTOR_DISABLED,
					       .gain       = NRF_SAADC_GAIN1_4,
					       .reference  = NRF_SAADC_REFERENCE_VDD4,
					       .acq_time   = NRF_SAADC_ACQTIME_10US,
					       .mode       = NRF_SAADC_MODE_SINGLE_ENDED,
					       .burst      = NRF_SAADC_BURST_DISABLED,
					       .pin_p      = (nrf_saadc_input_t)(nrf_adc_channel),
					       .pin_n      = NRF_SAADC_INPUT_DISABLED
  };

  //setup adc itself
  err_code = nrfx_saadc_init (&saadc_config, adc_handler);
  SPARK_ASSERT( err_code == NRF_SUCCESS);

  //setup channel
  err_code = nrfx_saadc_channel_init(PIN_MAP[pin].adc_channel, &channel_config);
  SPARK_ASSERT( err_code == NRF_SUCCESS);

  // fix this to set up double buffering as opposed to a
  // single sampling buffer now
  err_code = nrfx_saadc_buffer_convert (BUF0, BUF0_SIZE);
  SPARK_ASSERT( err_code == NRF_SUCCESS);
  err_code = nrfx_saadc_buffer_convert (BUF1, BUF1_SIZE);
  SPARK_ASSERT( err_code == NRF_SUCCESS);

  // timer starts the adc
  return nrfx_saadc_sample_task_get();
}


void button_handler(system_event_t event, int duration, void* )
{
  if(!duration && !button){	
    button = true;
  } else if (!duration && button){
    button = false;
  }	
}


void setup()
{
  Serial.begin(9600);
  pinMode(analogPin, INPUT);
  Udp.begin(8888);
  const uint32_t timer_addr = timer_init(TIMER_LED);
  const uint32_t adc_addr = adc_init(analogPin);
  ppi_init(timer_addr,adc_addr);
  nrfx_timer_enable(&TIMER_LED);
}

void loop()
{
  const int MAX_PACKET_SIZE = 256;
  int i = 0;

  bool stop = false; 

  if (button){
    while (i < 16000 && !stop){
      Udp.sendPacket( (const char *) &udpArr[i], sizeof(float) * MAX_PACKET_SIZE, 
						remoteIP, port);
      i +=  MAX_PACKET_SIZE;
    }
  }

  if (i >= 16000){
    csize = 0;  
  }

}
