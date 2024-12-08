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


extern ETH_HandleTypeDef heth;
extern SPI_HandleTypeDef hspi4;
extern SPI_HandleTypeDef hspi5;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim4;
extern UART_HandleTypeDef huart3;

extern struct netif gnetif;


// load the weight data block from the model.bin file
INCLUDE_FILE(".rodata", "./img.bin", image);
extern uint8_t image_data[];
extern size_t image_start[];
extern size_t image_end[];


GC9A01A tft1;
GC9A01A tft2;


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
 */
float states[N_STATES];


void set_left_eye_openness(float val) {
  val = val < 1 ? val : 1;
  val = val > 0 ? val : 0;

  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, EYELID_L_FULLCLOSE * (1 - val) + EYELID_L_FULLOPEN * val);
}

void set_right_eye_openness(float val) {
  val = val < 1 ? val : 1;
  val = val > 0 ? val : 0;

  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, EYELID_R_FULLCLOSE * (1 - val) + EYELID_R_FULLOPEN * val);
}


void UDP_receive_handler(void *arg, struct udp_pcb *udp_control, struct pbuf *rx_packet, const ip_addr_t *addr, u16_t port) {
  for (size_t i=0; i<N_STATES; i+=1) {
    states[i] = ((float *)rx_packet->payload)[i];
  }

  char str[128];
  sprintf(str, "Receive UDP of len %d\n", rx_packet->len);
  HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), 100);

  pbuf_free(rx_packet);

}

void UDP_init_receive() {
  /* 1. Create a new UDP control block  */
  struct udp_pcb *udp_control = udp_new();

  /* 2. Bind the udp_control to the local port */
  ip_addr_t ip_addr;
  u16_t port = 7000;
  IP_ADDR4(&ip_addr, 10, 0, 64, 64);

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
  struct pbuf *txBuf;
  char data[100];

  int len = sprintf(data, "sending UDP client message %lu", counter);

  /* allocate pbuf from pool*/
  txBuf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);

  if (txBuf != NULL)
  {
    /* copy data to pbuf */
    pbuf_take(txBuf, data, len);

    /* send udp data */
    udp_send(upcb, txBuf);

    /* free pbuf */
    pbuf_free(txBuf);
  }
}

void UDP_init_transmit(void) {
  err_t err;

  /* 1. Create a new UDP control block  */
  upcb = udp_new();

  /* Bind the block to module's IP and port */
  ip_addr_t myIPaddr;
  IP_ADDR4(&myIPaddr, 10, 0, 64, 64);
  udp_bind(upcb, &myIPaddr, 8);


  /* configure destination IP address and port */
  ip_addr_t DestIPaddr;
  IP_ADDR4(&DestIPaddr, 10, 0, 0, 10);
  err= udp_connect(upcb, &DestIPaddr, 7000);

  if (err == ERR_OK) {
    /* 2. Send message to server */
    UDP_transmit();

    /* 3. Set a receive callback for the upcb */
    udp_recv(upcb, UDP_client_receive_callback, NULL);
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
  IP4_ADDR(&localIP, 10, 0, 64, 64);

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

  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

  HAL_TIM_Base_Start(&htim4);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);

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

//  for (size_t i=0; i<tft1.width; i+=1) {
//    for (size_t j=0; j<tft1.height; j+=1) {
//      GC9A01A_draw_pixel(&tft1, j, i, 0xFFFF);
//      GC9A01A_draw_pixel(&tft2, j, i, 0xFFFF);
//    }
//  }


  UDP_init_receive();
//  UDP_init_transmit();
//  UDP_multicast_init();
}

void APP_main() {

  char str[128];
//  sprintf(str, "hello \n");
//  HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), 100);

  ethernetif_input(&gnetif);
  igmp_tmr();
  sys_check_timeouts();

//  UDP_transmit();



  int16_t eye_l_x_pixels = (int16_t)(-states[3] * EYE_MOVEMENT_X_SCALE);
  int16_t eye_l_y_pixels = (int16_t)(-states[4] * EYE_MOVEMENT_Y_SCALE);

  int16_t eye_r_x_pixels = (int16_t)(-states[5] * EYE_MOVEMENT_X_SCALE);
  int16_t eye_r_y_pixels = (int16_t)(-states[6] * EYE_MOVEMENT_Y_SCALE);

//  set_left_eye_openness(states[7]);
//  set_right_eye_openness(states[8]);
//
//  // left eye
//  GC9A01A_draw_pixels(&tft1, eye_l_x_pixels, eye_l_y_pixels, (uint16_t *)image_data, 240, 240);
//  // right eye
//  GC9A01A_draw_pixels(&tft2, eye_r_x_pixels, eye_r_y_pixels, (uint16_t *)image_data, 240, 240);

  sprintf(str, "states: %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\n",
      states[0], states[1], states[2],
      states[3], states[4], states[5], states[6],
      states[7], states[8]);
  HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), 100);

  HAL_Delay(100);
}
