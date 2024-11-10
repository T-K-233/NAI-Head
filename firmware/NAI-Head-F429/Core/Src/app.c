/*
 * app.c
 *
 *  Created on: Nov 6, 2024
 *      Author: TK
 */

#include "app.h"

// some import hierarchy bugs, these two can only be placed in app.c
#include "lwip.h"
#include "udp.h"


extern ETH_HandleTypeDef heth;
extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim4;
extern UART_HandleTypeDef huart3;


extern struct netif gnetif;


void UDP_receive_handler(void *arg, struct udp_pcb *udp_control, struct pbuf *packet, const ip_addr_t *addr, u16_t port) {
  struct pbuf *tx_buf;

  // Get the IP of the Client
//  char *remote_ip = ipaddr_ntoa(addr);

  char buf[100];

  int len = sprintf(buf,"Hello %s From UDP SERVER\n", (char*)packet->payload);

  // allocate pbuf from RAM
  tx_buf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);

  // copy the data into the buffer
  pbuf_take(tx_buf, buf, len);

  // Connect to the remote client
//  udp_connect(udp_control, addr, port);

  // Send a Reply to the Client
//  udp_send(udp_control, tx_buf);

  // free the UDP connection, so we can accept new clients
//  udp_disconnect(udp_control);

  // Free the buffers
  pbuf_free(tx_buf);
  pbuf_free(packet);

  char str[128];
  sprintf(str, "Receive UDP\n");
  HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), 100);

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

void UDP_client_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
  /* Copy the data from the pbuf */
  strncpy (buffer, (char *)p->payload, p->len);

  /*increment message count */
  counter += 1;

  /* Free receive pbuf */
  pbuf_free(p);
}


void UDP_transmit() {
  struct pbuf *txBuf;
  char data[100];

  int len = sprintf(data, "sending UDP client message %d", counter);

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




void recCallBack(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                const ip_addr_t *addr, u16_t port)
{
    if (p != NULL) {
        // Copy received data to buffer
        char buffer[128];
        memcpy(buffer, p->payload, p->len < sizeof(buffer) ? p->len : sizeof(buffer));
        
        // Debug print
        char str[128];
        sprintf(str, "Received Multicast: %s\n", buffer);
        HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), 100);
        
        // Free the packet buffer
        pbuf_free(p);
    }
}

static void UDP_Multicast_init()
{
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

    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);

//  GC9A01A_init(&tft1, &hspi1,
//    TFT1_CS_GPIO, TFT1_CS_PIN,
//    TFT1_DC_GPIO, TFT1_DC_PIN,
//    TFT1_BL_GPIO, TFT1_BL_PIN,
//    TFT1_RST_GPIO, TFT1_RST_PIN
//  );
//
//  GC9A01A_init(&tft2, &hspi2,
//    TFT2_CS_GPIO, TFT2_CS_PIN,
//    TFT2_DC_GPIO, TFT2_DC_PIN,
//    TFT2_BL_GPIO, TFT2_BL_PIN,
//    TFT2_RST_GPIO, TFT2_RST_PIN
//  );


//  UDP_init_receive();
//  UDP_init_transmit();
    UDP_Multicast_init();
}

void APP_main() {
//
//  char str[128];
//  sprintf(str, "hello \n");
//  HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), 100);
//
//  HAL_Delay(100);

  //    char str[128];
  //    sprintf(str, "hello\n");
  //    HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), 100);

  ethernetif_input(&gnetif);
  igmp_tmr();
  sys_check_timeouts();

//      UDP_transmit();

//  set_left_eye_openness(state[0]);
//  set_right_eye_openness(state[1]);
//
//  int16_t eye_l_x_pixels = (int16_t)(-state[2] * EYE_MOVEMENT_X_SCALE);
//  int16_t eye_l_y_pixels = (int16_t)(-state[3] * EYE_MOVEMENT_Y_SCALE);
//
//  int16_t eye_r_x_pixels = (int16_t)(-state[4] * EYE_MOVEMENT_X_SCALE);
//  int16_t eye_r_y_pixels = (int16_t)(-state[5] * EYE_MOVEMENT_Y_SCALE);
//
//  // left eye
//  GC9A01A_draw_pixels(&tft1, eye_l_x_pixels, eye_l_y_pixels, (uint16_t *)image_data, 240, 240);
//
//  // right eye
//  GC9A01A_draw_pixels(&tft2, eye_r_x_pixels, eye_r_y_pixels, (uint16_t *)image_data, 240, 240);
//
//  counter += 1;
}
