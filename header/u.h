#ifndef U_H
#define U_H
void USART_Init( unsigned int ubrr)
{
  UCSR0C = (1<<USBS0)|(3<<UCSZ00); 
}
#endif //USART_H
