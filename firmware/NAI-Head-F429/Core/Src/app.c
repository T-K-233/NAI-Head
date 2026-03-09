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


typedef struct {
  float RESERVED0;
  float face_angle_x;
  float face_angle_y;
  float face_angle_z;
  float brow_height_left;
  float brow_height_right;
  float eye_open_left;
  float eye_open_right;
  float eye_left_x;
  float eye_left_y;
  float eye_right_x;
  float eye_right_y;
} UDPMessage;


UDPMessage message;


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim == &htim6) {
    int16_t eye_l_x_pixels = (int16_t)(-message.eye_left_x * EYE_MOVEMENT_X_SCALE);
    int16_t eye_l_y_pixels = (int16_t)(-message.eye_left_y * EYE_MOVEMENT_Y_SCALE);

    int16_t eye_r_x_pixels = (int16_t)(-message.eye_right_x * EYE_MOVEMENT_X_SCALE);
    int16_t eye_r_y_pixels = (int16_t)(-message.eye_right_y* EYE_MOVEMENT_Y_SCALE);

    uint16_t *iris_img_data;
    iris_img_data = (uint16_t *)iris_normal_data;

//    if (commands[8] == 1.f) {
//      iris_img_data = (uint16_t *)iris_large_data;
//      eye_l_x_pixels /= 2;
//      eye_l_y_pixels /= 2;
//      eye_r_x_pixels /= 2;
//      eye_r_y_pixels /= 2;
//    }
//    else if (commands[8] == 2.f) {
//      iris_img_data = (uint16_t *)iris_heart_data;
//    }
//    else {
//      iris_img_data = (uint16_t *)iris_normal_data;
//    }

    // left eye
    GC9A01A_draw_pixels(&tft1, eye_l_x_pixels, eye_l_y_pixels, iris_img_data, 240, 240);
    // right eye
    GC9A01A_draw_pixels(&tft2, eye_r_x_pixels, eye_r_y_pixels, iris_img_data, 240, 240);
  }
  else if (htim == &htim7) {
    robot_set_eyebrow(&actuators, message.brow_height_left, message.brow_height_right);
    robot_set_eyelid(&actuators, message.eye_open_left, message.eye_open_right);
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
  memcpy(&message, rx_packet->payload, sizeof(UDPMessage));

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

  size_t len = sizeof(UDPMessage);

  /* allocate pbuf from pool*/
  tx_buf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);

  if (tx_buf != NULL) {
    /* copy data to pbuf */
    pbuf_take(tx_buf, &message, len);

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


uint8_t debug_counter = 0;

void APP_init() {
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

  message.eye_open_left = 0.5f;
  message.eye_open_right = 0.5f;
  message.brow_height_left = -0.8f;
  message.brow_height_right = -0.8f;
}

void APP_main() {
  /* receive UDP packet */
  ethernetif_input(&gnetif);
  sys_check_timeouts();

  /* transmit UDP packet */
  UDP_transmit();

  /* animation state machine */
  debug_counter += 1;

  if (debug_counter >= 50) {
    char str[128];
    sprintf(str, "face=(%.2f, %.2f, %.2f), brows=(%.2f, %.2f), eyelid=(%.2f, %.2f), eyeball=(%.2f, %.2f)(%.2f, %.2f)\n",
        message.face_angle_x, message.face_angle_y, message.face_angle_z,
        message.brow_height_left, message.brow_height_right,
        message.eye_open_left, message.eye_open_right,
        message.eye_left_x, message.eye_left_y, message.eye_right_x, message.eye_right_y);
    HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), 100);

    debug_counter = 0;
  }

  HAL_Delay(10);
}
