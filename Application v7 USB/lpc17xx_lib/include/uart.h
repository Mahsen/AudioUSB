#ifndef UART_H_
#define UART_H_
// ***********************
#define Parity_none    0
#define Parity_event   1
#define Parity_old     2
// ***********************
// Function to set up UART  [ bits, Parity, Stop bit ]
void UART_Setting(int baudrate,char bits,char Stop_bit,char Parity);
// ***********************
// Function to set up UART
void UART_Init(char uart,int baudrate,char bits,char Stop_bit,char Parity);
void UART0_Init(int baudrate,char bits,char Stop_bit,char Parity);
void UART1_Init(int baudrate,char bits,char Stop_bit,char Parity);
void UART2_Init(int baudrate,char bits,char Stop_bit,char Parity);
void UART3_Init(int baudrate,char bits,char Stop_bit,char Parity);
// ***********************
// Function to send character over UART
void UART_Sendchar(char uart,char c);
void UART0_Sendchar(char c);
void UART1_Sendchar(char c);
void UART2_Sendchar(char c);
void UART3_Sendchar(char c);
// ***********************
// Function to get character from UART
char UART_Getchar(char uart);
char UART0_Getchar();
char UART1_Getchar();
char UART2_Getchar();
char UART3_Getchar();
// ***********************
// Function to prints the string out over the UART
void UART_PrintString(char uart,char *pcString);
#endif /*UART_H_*/
