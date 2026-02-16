/*
 * app.c
 *
 *  Created on: Nov 6, 2024
 *      Author: TK
 */

#include "app.h"

// some import hierarchy bugs, these two can only be placed in app.c
#include "lwip.h"
#include "lwip/igmp.h"
#include "udp.h"

#define ADC_SAMPLES    128

#define ADC_BASELINE    0


extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ETH_HandleTypeDef heth;
extern CAN_HandleTypeDef hcan1;
extern SPI_HandleTypeDef hspi4;
extern SPI_HandleTypeDef hspi5;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;
extern UART_HandleTypeDef huart3;

extern struct netif gnetif;


// load the weight data block from the model.bin file
INCLUDE_FILE(".rodata", "./iris.bin", iris_normal);
extern uint8_t iris_normal_data[];
extern size_t iris_normal_start[];
extern size_t iris_normal_end[];

INCLUDE_FILE(".rodata", "./iris_heart.bin", iris_heart);
extern uint8_t iris_heart_data[];
extern size_t iris_heart_start[];
extern size_t iris_heart_end[];

INCLUDE_FILE(".rodata", "./iris_large.bin", iris_large);
extern uint8_t iris_large_data[];
extern size_t iris_large_start[];
extern size_t iris_large_end[];


GC9A01A tft1;
GC9A01A tft2;

ActuatorControl actuators;


/**
 * state[0]: HeadX
 * state[1]: HeadY
 * state[2]: HeadZ
 * state[3]: EyeLeftX
 * state[4]: EyeLeftY
 * state[5]: EyeRightX
 * state[6]: EyeRightY
 * state[7]: EyeOpenLeft
 * state[8]: EyeOpenRight
 * state[9]: Iris
 *
 * state[10]: gesture
 */
float states[N_STATES];

/**
 * action[0]: EyeLeftX
 * action[1]: EyeLeftY
 * action[2]: EyeRightX
 * action[3]: EyeRightY
 * action[4]: EyeOpenLeft
 * action[5]: EyeOpenRight
 * action[6]: EyebrowLeftX
 * action[7]: EyebrowRightX
 * action[8]: Iris Change
 * action[9]: Reserved
 */
float acs[N_ACTIONS];


float time_delay_avg = 0;

size_t last_detected_sound = 0;

uint8_t animation_type = 0;
size_t animation_counter = 0;
size_t blink_counter = 0;


uint16_t adc_data_buffer_1[ADC_SAMPLES];
uint16_t adc_data_buffer_2[ADC_SAMPLES];
int16_t data_1[ADC_SAMPLES];
int16_t data_2[ADC_SAMPLES];


uint8_t completed = 0;
uint8_t debug_counter = 0;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
  completed = 1;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim == &htim6) {
    int16_t eye_l_x_pixels = (int16_t)(-acs[0] * EYE_MOVEMENT_X_SCALE);
    int16_t eye_l_y_pixels = (int16_t)(-acs[1] * EYE_MOVEMENT_Y_SCALE);

    int16_t eye_r_x_pixels = (int16_t)(-acs[2] * EYE_MOVEMENT_X_SCALE);
    int16_t eye_r_y_pixels = (int16_t)(-acs[3] * EYE_MOVEMENT_Y_SCALE);

    uint16_t *iris_img_data;
    if (acs[8] == 1.f) {
      iris_img_data = (uint16_t *)iris_large_data;
      eye_l_x_pixels /= 2;
      eye_l_y_pixels /= 2;
      eye_r_x_pixels /= 2;
      eye_r_y_pixels /= 2;
    }
    else if (acs[8] == 2.f) {
      iris_img_data = (uint16_t *)iris_heart_data;
    }
    else {
      iris_img_data = (uint16_t *)iris_normal_data;
    }

    // left eye
    GC9A01A_draw_pixels(&tft1, eye_l_x_pixels, eye_l_y_pixels, iris_img_data, 240, 240);
    // right eye
    GC9A01A_draw_pixels(&tft2, eye_r_x_pixels, eye_r_y_pixels, iris_img_data, 240, 240);
  }
  else if (htim == &htim7) {
    robot_set_eyelid(&actuators, acs[4], acs[5]);
    robot_set_eyebrow(&actuators, acs[6], acs[7]);
  }
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
  if (hspi == &hspi4) {
    HAL_GPIO_WritePin(GPIO_SPI4_CS_GPIO_Port, GPIO_SPI4_CS_Pin, 1);
  }
  else if (hspi == &hspi5) {
    HAL_GPIO_WritePin(GPIO_SPI5_CS_GPIO_Port, GPIO_SPI5_CS_Pin, 1);
  }

}


void UDP_receive_handler(void *arg, struct udp_pcb *udp_control, struct pbuf *rx_packet, const ip_addr_t *addr, u16_t port) {
  for (size_t i=0; i<N_STATES; i+=1) {
    states[i] = ((float *)rx_packet->payload)[i];
  }

  pbuf_free(rx_packet);
}

void UDP_init_receive() {
  /* 1. Create a new UDP control block  */
  struct udp_pcb *udp_control = udp_new();

  /* 2. Bind the udp_control to the local port */
  ip_addr_t ip_addr;
  u16_t port = 7000;
  IP_ADDR4(&ip_addr, 172, 28, 0, 64);

  err_t err = udp_bind(udp_control, &ip_addr, port);

  /* 3. Set a receive callback for the upcb */
  if(err == ERR_OK) {
    udp_recv(udp_control, UDP_receive_handler, NULL);
  }
  else {
    udp_remove(udp_control);
  }
}


char buffer[128];
uint32_t counter = 0;
struct udp_pcb *upcb;

void UDP_client_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
  /* Copy the data from the pbuf */
  strncpy(buffer, (char *)p->payload, p->len);

  /*increment message count */
  counter += 1;

  /* Free receive pbuf */
  pbuf_free(p);
}


void UDP_transmit() {
  struct pbuf *tx_buf;

  float acs_buf[N_ACTIONS];
  size_t len = N_ACTIONS * sizeof(float);

  memcpy(acs_buf, acs, len);


  /* allocate pbuf from pool*/
  tx_buf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);

  if (tx_buf != NULL) {
    /* copy data to pbuf */
    pbuf_take(tx_buf, acs_buf, len);

    /* send udp data */
    udp_send(upcb, tx_buf);

    /* free pbuf */
    pbuf_free(tx_buf);
  }
}

void UDP_init_transmit(void) {
  /* 1. Create a new UDP control block  */
  upcb = udp_new();

  /* Bind the block to module's IP and port */
  ip_addr_t my_ip_addr;
  IP_ADDR4(&my_ip_addr, 172, 28, 0, 64);
  udp_bind(upcb, &my_ip_addr, 8);


  /* configure destination IP address and port */
  ip_addr_t dest_ip_addr;
  IP_ADDR4(&dest_ip_addr, 172, 28, 0, 10);
  err_t err = udp_connect(upcb, &dest_ip_addr, 8000);

  if (err) {
    // Debug print
    char str[256];
    sprintf(str, "Error initializing UDP TX!\n");
    HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), 100);

  }
}



void recCallBack(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
  if (p != NULL) {
    // Copy received data to buffer
    char buffer[128];
    memcpy(buffer, p->payload, p->len < sizeof(buffer) ? p->len : sizeof(buffer));

    // Debug print
    char str[256];
    sprintf(str, "Received Multicast: %s\n", buffer);
    HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), 100);

    // Free the packet buffer
    pbuf_free(p);
  }
}

void UDP_multicast_init() {
  struct ip4_addr ipgroup, localIP;
  static struct udp_pcb *g_udppcb = NULL; // Make static to prevent deallocation
  err_t err;
  char str[128];

  // Clean up any existing PCB
  if (g_udppcb != NULL) {
    udp_remove(g_udppcb);
    g_udppcb = NULL;
  }

  IP4_ADDR(&ipgroup, 224, 1, 1, 1);
  IP4_ADDR(&localIP, 172, 28, 0, 64);

  // Create new UDP PCB
  g_udppcb = udp_new();
  if (g_udppcb == NULL) {
    sprintf(str, "Failed to create UDP PCB\n");
    HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), 100);
    return;
  }

  // Bind to ANY address and specific port
  err = udp_bind(g_udppcb, IP_ADDR_ANY, 7000);
  if (err != ERR_OK) {
    sprintf(str, "Failed to bind UDP: %d\n", err);
    HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), 100);
    udp_remove(g_udppcb);
    return;
  }

  // Join multicast group
  err = igmp_joingroup(&localIP, &ipgroup);
  if (err != ERR_OK) {
    sprintf(str, "Failed to join multicast group: %d\n", err);
    HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), 100);
    udp_remove(g_udppcb);
    return;
  }

  // Set receive callback
  udp_recv(g_udppcb, recCallBack, NULL);

  sprintf(str, "Multicast initialized successfully\n");
  HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), 100);
}


void APP_init() {
  ETH_MACFilterConfigTypeDef filterConfig = {0};
  filterConfig.PromiscuousMode = ENABLE;
  filterConfig.PassAllMulticast = ENABLE;

  if (HAL_ETH_SetMACFilterConfig(&heth, &filterConfig) != HAL_OK) {
    char str[128];
    sprintf(str, "Failed to set MAC filter\n");
    HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), 100);
  }

  igmp_init();

  robot_actuator_init(&actuators,
      &htim1, TIM_CHANNEL_1, TIM_CHANNEL_2,
      &htim4, TIM_CHANNEL_3, TIM_CHANNEL_4
    );

  GC9A01A_init(&tft1, &hspi4,
    GPIO_SPI4_CS_GPIO_Port, GPIO_SPI4_CS_Pin,
    GPIO_SPI4_DC_GPIO_Port, GPIO_SPI4_DC_Pin,
    GPIO_LCD_BLK_GPIO_Port, GPIO_LCD_BLK_Pin,
    GPIO_LCD_RESET_GPIO_Port, GPIO_LCD_RESET_Pin
  );

  GC9A01A_init(&tft2, &hspi5,
    GPIO_SPI5_CS_GPIO_Port, GPIO_SPI5_CS_Pin,
    GPIO_SPI5_DC_GPIO_Port, GPIO_SPI5_DC_Pin,
    NULL, 0,
    NULL, 0
  );

  // start triggers
  HAL_TIM_Base_Start_IT(&htim6);
  HAL_TIM_Base_Start_IT(&htim7);


  UDP_init_receive();
  UDP_init_transmit();
//  UDP_multicast_init();

  acs[4] = 0.5f;
  acs[5] = 0.5f;
  acs[6] = -0.8f;
  acs[7] = -0.8f;
}

void APP_main() {
  /* receive UDP packet */
  ethernetif_input(&gnetif);
  sys_check_timeouts();

  /* transmit UDP packet */
  UDP_transmit();

  /* animation state machine */
  float guesture = states[10];

  if (guesture == -1.f || guesture == 0.f) {  // no guesture detected
    acs[0] = states[0];
    acs[1] = states[1];
    acs[3] = states[3];
    acs[4] = states[4];
    acs[5] = states[5];
    acs[6] = states[6];
    if (guesture == 0.f) {
      acs[7] = 1.f;
      acs[8] = 1.f;
    }
    else {
      acs[7] = states[7];
      acs[8] = states[8];
    }
    acs[9] = states[9];
    if (guesture == 0.f) {
      acs[2] = time_delay_avg * 0.3f;
    }
    else {
      acs[2] = states[2];
    }
  }
  else if (guesture == 1.f) {  // ok
    if (!animation_type) {
      animation_type = 1;
      animation_counter = 100;
    }
  }
  else if (guesture == 2.f) {  // open ("hi")
    acs[0] = 0.8f;
    acs[1] = -0.4f;
    acs[7] = 1.f;
    acs[8] = 1.f;

    acs[9] = 1.f;  // enlarged eyes
  }
  else if (guesture == 3.f) {  // close
    acs[7] = 0.f;  // closed eyes
    acs[8] = 0.f;  // closed eyes
    acs[9] = 0.f;
  }
  else if (guesture == 4.f) {  // point
    acs[7] = 0.f;  // wink
    acs[8] = 1.f;  //
    acs[9] = 0.f;
  }
  else if (guesture == 5.f) {  // heart
    acs[2] = time_delay_avg * 0.3f;
    acs[7] = 1.f;
    acs[8] = 1.f;
    acs[9] = 2.f;
  }

  if (animation_type == 1) {
    animation_counter -= 1;

    if (animation_counter > 50) {
      acs[1] = 1.f;
    }
    else {
      acs[1] = 0.f;
    }

    if (animation_counter == 0) {
      animation_type = 0;
      states[10] = 0;
    }
  }


  if (random() % 1000 < 1) {
    blink_counter = 10;
  }

  if (blink_counter) {
    acs[7] = 0.f;  // blink
    acs[8] = 0.f;  //
    blink_counter -= 1;
  }






  debug_counter += 1;

  if (debug_counter >= 50) {
    char str[128];
    sprintf(str, "states: %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\n",
        states[0], states[1], states[2],
        states[3], states[4], states[5], states[6],
        states[7], states[8]);
    HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), 100);


    debug_counter = 0;
  }

  HAL_Delay(10);
}
