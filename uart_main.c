#include "cMIPS.h"
#include "uart_defs.h"

#define SPEED 3

int proberx(void); // retorna "nrx": o número de  caracteres na fila de recepção
int probetx(void); // retorna "ntx"
Tstatus iostat(void);
void ioctl(Tcontrol);
char Getc(void);
void Putc(char);


//char alinhamento[1][21];

int main(void){
  volatile Tserial *uart;
  Tstatus stat;
  extern UARTdriver Ud;
  volatile UARTdriver *Ud_ptr;
  Tcontrol ctrl;
  char c;

  uart = (void *)IO_UART_ADDR; // o endereço base da UART
  Ud_ptr = &Ud;
  
  ctrl.rts   = 0;
  ctrl.speed = SPEED;
  ioctl(ctrl); // inicializa UART
  

  Ud_ptr->rx_hd = 0;
  Ud_ptr->rx_tl = 0;
  Ud_ptr->tx_hd = 0;
  Ud_ptr->tx_tl = 0;
  Ud_ptr->nrx = 0;
  Ud_ptr->ntx = 0;
  
  uart->interr.i = UART_INT_progRX; // interrupção de recepção apenas
  
  ctrl.rts   = 1;
  ctrl.speed = SPEED;
  ioctl(ctrl); // sinaliza a UART que o programa irá transmitir
  
  do {
    while ( (proberx()) == 0 ){};  // checo se existe algum caractere na fila de receção 
      //delay_cycle(1);                       // nothing new, wait

    c = Getc();  // recebo um caractere da fila
    
    if (c != EOT)
      to_stdout( c ); // imprime o caractere
    else
      to_stdout( '\n' ); // imprime newline

  } while (c != EOT); // vai até achar EOT

  delay_cycle(200);
  
  exit(0);
}

//int main (void) {
//
//  extern UD;
//
//  laço com Getc() para receber todas as strings e guardar na filaEntr[]
//
//       se há string completa em filaEntr[]
//
//       alinha a string e insere em filaSai[]
//
//    laço com Putc() para esvaziar filaSai[];
//
//  delay_cycle(200);  // espera até esvaziar a fila do handler;
//
//  exit(0);
//
//}
//
//Putc(char c) {
//  ...
//}
//
char Getc() {
  extern UARTdriver Ud;
  volatile UARTdriver *Ud_ptr;
  char c;

  Ud_ptr = &Ud;
  
  disableInterr();// atualização da fila

  Ud_ptr->nrx -= 1;
  c = Ud_ptr->rx_q[Ud_ptr->rx_hd];
  Ud_ptr->rx_hd = (Ud_ptr->rx_hd + 1) & (Q_SZ - 1);

  enableInterr();

  return c;
}

void ioctl (Tcontrol ctrl) {
  volatile Tserial *uart;
  uart = (void *)IO_UART_ADDR; // o endereço base da UART

  ctrl.ign   = 0;
  ctrl.ign4  = 0;
  uart->ctl  = ctrl;
}

Tstatus iostat() {
  volatile Tserial *uart;
  Tstatus status;
  
  uart = (void *)IO_UART_ADDR; // o endereço base da UART
  status = uart->stat;
  
  return(status);
}

int proberx() {
  extern UARTdriver Ud;
  volatile UARTdriver *Ud_ptr;

  Ud_ptr = &Ud;
  
  return( Ud_ptr->nrx );
}

int probetx() {
  extern UARTdriver Ud;
  volatile UARTdriver *Ud_ptr;

  Ud_ptr = &Ud;
  
  return( Ud_ptr->ntx );
}


/* int main (void) {

   while ( entrada não acabou ) {
         laço com Getc() para receber todas as strings e guardar na filaEntr[]

         se há string completa em filaEntr[]

         alinha a string e insere em filaSai[]

         se há espaço na TXQueue (probeTX > 0) então Putc( filaSai[] )
    }

    laço com Putc() para esvaziar filaSai[];

    delay_cycle(200);  // espera até esvaziar a fila do handler;

    exit(0);

}*/
