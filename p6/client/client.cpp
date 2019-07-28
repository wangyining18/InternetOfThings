#include "nrfx.h"
#include "nrfx_ppi.h"
#include "nrfx_saadc.h"
#include "nrfx_timer.h"

#include "application.h"

#define LED D7
#define MIC_IN A0
#define TIMER_INSTANCE 3
#define TIMER_TICK_US 62

#define MY_SSID  "MY_SSID"
#define WIFI_PASSCODE "MY_PASSCODE"

#define MAX_PACKET_SIZE 256


SYSTEM_MODE(MANUAL);

volatile static uint32_t adc_evt_cnt = 0;

const nrfx_timer_t TIMER_LED = NRFX_TIMER_INSTANCE(TIMER_INSTANCE);

#define BUF0_SIZE 16000
#define BUF1_SIZE 16000

int16_t BUF0 [BUF0_SIZE];
int16_t BUF1 [BUF1_SIZE];



char status[10];
int count;



int16_t *send_buffer = nullptr;
uint16_t send_sz = 0;

char my_ip_addr[24];
char my_ssid[30];
IPAddress SERVER_ADDR = IPAddress(192,168,0,9);
int SERVER_PORT = 59993;
UDP udp;

bool transmit = false;

void button_handler(system_event_t event, int duration, void* )
{
  if (!duration)
    {
      transmit = !transmit;
    }
}


void  timer_handler( nrf_timer_event_t event_type, void *p_context)
{
}


uint32_t timer_init()
{

  uint32_t time_us = TIMER_TICK_US;
  uint32_t err_code = NRF_SUCCESS;
  uint32_t time_ticks;

  const nrfx_timer_config_t TIMER_CFG = {
    .frequency  = NRF_TIMER_FREQ_1MHz,  //2MHz
    .mode       = NRF_TIMER_MODE_TIMER,
    .bit_width  = NRF_TIMER_BIT_WIDTH_32,
    .interrupt_priority = NRFX_TIMER_DEFAULT_CONFIG_IRQ_PRIORITY,
    .p_context  = NULL
  };

  err_code = nrfx_timer_init(&TIMER_LED, &TIMER_CFG, timer_handler);

  time_ticks =  nrfx_timer_us_to_ticks( &TIMER_LED, time_us);

  nrfx_timer_extended_compare (&TIMER_LED, NRF_TIMER_CC_CHANNEL0, time_ticks,
                               NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);

  // return timer address
  return nrfx_timer_compare_event_address_get( &TIMER_LED, NRF_TIMER_CC_CHANNEL0);

}

void timer_start ( const nrfx_timer_t timer_id )
{
  nrfx_timer_enable(&timer_id);
}

void adc_handler( const nrfx_saadc_evt_t * event_type)
{
  if (event_type->type == NRFX_SAADC_EVT_DONE)
    {
      //digitalWrite(LED, HIGH);
      nrf_saadc_value_t * buf = event_type->data.done.p_buffer;
      uint16_t buf_sz = event_type->data.done.size;

      if (send_buffer == nullptr)
        {
          send_buffer = buf;
          send_sz = buf_sz;
        }
      nrfx_saadc_buffer_convert(buf, buf_sz);
      adc_evt_cnt++;
      //digitalWrite(LED, LOW);
    } else {
    SPARK_ASSERT(0);
  }
}

uint32_t adc_init( const uint16_t pin)
{

  uint32_t err_code = NRF_SUCCESS;

  const nrfx_saadc_config_t saadc_config =
    {
      .resolution         = NRF_SAADC_RESOLUTION_12BIT,
      .oversample         = NRF_SAADC_OVERSAMPLE_DISABLED,
      .interrupt_priority = NRFX_SAADC_CONFIG_IRQ_PRIORITY
    };

  //
  // Configure the ADC Channel
  //
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
  if (PIN_MAP[pin].pin_func != PF_NONE && PIN_MAP[pin].pin_func != PF_DIO)
    {
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

  //setup double buffering
  err_code = nrfx_saadc_buffer_convert (BUF0, BUF0_SIZE);
  SPARK_ASSERT( err_code == NRF_SUCCESS);

  err_code = nrfx_saadc_buffer_convert (BUF1, BUF1_SIZE);
  SPARK_ASSERT( err_code == NRF_SUCCESS);

  //let the Timer start the adc

  return nrfx_saadc_sample_task_get();
}

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

void connect_wifi()
{
  WiFi.on();
  // WiFi.setCredentials(MY_SSID, WIFI_PASSCODE, WPA2);
  WiFi.connect();
  while(!WiFi.ready());
}

void setup()
{

  Serial.begin(9600);
  //wait 10sec for serial connection
  waitFor(Serial.isConnected, 10000);

  pinMode(LED, OUTPUT);

  uint32_t adc_addr = adc_init(MIC_IN);
  uint32_t timer_addr = timer_init();
  ppi_init(timer_addr, adc_addr);

  timer_start(TIMER_LED);

  connect_wifi();

  //System.on(button_status, button_handler);

  IPAddress my_ip = WiFi.localIP();
  sprintf(my_ip_addr, "%d.%d.%d.%d", my_ip[0], my_ip[1], my_ip[2], my_ip[3]);
  sprintf(my_ssid, "%s", WiFi.SSID());
  udp.begin(8888);

}



void loop()
{
  uint16_t j =0;

  if ((send_sz >=0) && (send_buffer != nullptr)) {
      for (uint16_t i = 0; i < send_sz; i += MAX_PACKET_SIZE) {
          j = ( i + MAX_PACKET_SIZE < send_sz ? i + MAX_PACKET_SIZE : send_sz);
          udp.sendPacket( (const char *) &send_buffer[i], sizeof(uint16_t) * (j-i), SERVER_ADDR, SERVER_PORT);
        }

      send_buffer = nullptr;
      send_sz = 0;
    }

  if (udp.parsePacket() > 0){
               count = udp.read(status, 2);
               Serial.println("status received. \n");

               if ((*status) == 'Y') {
                   digitalWrite(LED, HIGH);  
               }
               else if ((*status) == 'N') { 
        digitalWrite(LED, LOW);
      }
    }

}