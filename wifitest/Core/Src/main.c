/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Simple HTTP Server for B-L475E-IOT01A
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"
#include "wifi.h"
#include <string.h>
#include <stdio.h>

/* --- APNA WIFI DETAILS YAHA DALO --- */
#define WIFI_SSID       "LENOVO"
#define WIFI_PASSWORD   "7477097594"
#define PORT            80

/* Webpage Content */
const char *WebPage =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n\r\n"
    "<html><head><title>STM32 Wi-Fi Server</title></head>"
    "<body style='text-align: center; font-family: Arial; margin-top: 50px;'>"
    "<h1>Hello from STM32L475!</h1>"
    "<p>Bhai ka HTTP Server successfully chal raha hai!</p>"
    "</body></html>";

uint8_t IP_Addr[4];
int32_t Socket = -1;
uint8_t RxData[512];
uint16_t RxLen;

/* Function Prototypes */
void SystemClock_Config(void);
void WebServer_Process(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  /* ST Board Specific Inits (LEDs, UART for debugging, etc.) */
  BSP_LED_Init(LED2);

  printf("\r\n\r\n--- STM32 Wi-Fi HTTP Server --- \r\n");

  /* 1. Initialize Wi-Fi */
  if (WIFI_Init() == WIFI_STATUS_OK) {
      printf("Wi-Fi Initialized.\r\n");
  } else {
      printf("Wi-Fi Init Failed!\r\n");
      Error_Handler();
  }

  /* 2. Connect to Router */
  printf("Connecting to: %s\r\n", WIFI_SSID);
  if (WIFI_Connect(WIFI_SSID, WIFI_PASSWORD, WIFI_ECN_WPA2_PSK) == WIFI_STATUS_OK) {
      WIFI_GetIP_Address(IP_Addr);
      printf("Connected! IP Address: %d.%d.%d.%d\r\n", IP_Addr[0], IP_Addr[1], IP_Addr[2], IP_Addr[3]);
      BSP_LED_On(LED2); // Turn on LED when connected
  } else {
      printf("Connection Failed!\r\n");
      Error_Handler();
  }

  /* 3. Start Server */
  if (WIFI_StartServer(PORT, WIFI_TCP_PROTOCOL) == WIFI_STATUS_OK) {
      printf("Server started on Port %d\r\n", PORT);
  } else {
      printf("Server Start Failed!\r\n");
      Error_Handler();
  }

  /* Infinite loop */
  while (1)
  {
      WebServer_Process();
  }
}

void WebServer_Process(void)
{
  uint8_t RemoteIP[4];
  uint16_t RemotePort;

  Socket = -1;
  /* Wait for client to connect */
  WIFI_WaitServerConnection(&Socket, 1000, RemoteIP, &RemotePort);

  if (Socket >= 0) {
      printf("Client Connected!\r\n");

      /* Receive request from browser */
      if (WIFI_ReceiveData(Socket, RxData, sizeof(RxData), &RxLen, 1000) == WIFI_STATUS_OK) {
          if (RxLen > 0) {
              /* Send HTML webpage */
              WIFI_SendData(Socket, (uint8_t *)WebPage, strlen(WebPage), &RxLen, 1000);
              printf("Webpage Sent to Client.\r\n");
          }
      }
      /* Close socket */
      WIFI_CloseServerConnection(Socket);
  }
}

void Error_Handler(void)
{
  while (1) {
      BSP_LED_Toggle(LED2);
      HAL_Delay(200);
  }
}
