#ifndef __HYPERTERMINAL_H
#define __HYPERTERMINAL_H

/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
//ypedef  void (*pFunction)(void);
/* Exported constants --------------------------------------------------------*/
/* Constants used by Serial Command Line Mode */
#define CMD_STRING_SIZE     128
/* Exported macro ------------------------------------------------------------*/
#define IS_AF(c)	((c >= 'A') && (c <= 'F'))
#define IS_af(c)	((c >= 'a') && (c <= 'f'))
#define IS_09(c)	((c >= '0') && (c <= '9'))
#define ISVALIDHEX(c)	IS_AF(c) || IS_af(c) || IS_09(c)
#define ISVALIDDEC(c)	IS_09(c)
#define CONVERTDEC(c)	(c - '0')
#define CONVERTHEX_alpha(c)	(IS_AF(c) ? (c - 'A'+10) : (c - 'a'+10))
#define CONVERTHEX(c)   (IS_09(c) ? (c - '0') : CONVERTHEX_alpha(c))
/* Exported functions ------------------------------------------------------- */
void Int2Str(char* str, uint32_t intnum);
uint8_t Str2Int(char* inputstr, uint32_t* intnum);
uint8_t GetIntegerInput(uint32_t* num);
void SerialPutChar(char c);
void SerialPutString(char* s);
void GetInputString(char* buffP);
void Main_Menu(void);

void Control_Point_Task(void);

#endif
