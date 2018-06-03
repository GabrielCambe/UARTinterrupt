#include "cMIPS.h"
#include "uart_defs.h"

#define QUEUE_SIZE 210 // espaço para dez strings ao mesmo tempo
#define SPEED 3

Tstatus iostat(void);
void ioctl(Tcontrol);
int proberx(void); // retorna "nrx": o número de  caracteres na fila de recepção
int probetx(void); // retorna "ntx"
char Getc(void);
void Putc(char);
void alinha(char *str);


int main(void){
  extern int tx_has_started;
  extern UARTdriver Ud;
  volatile UARTdriver *Ud_ptr;
  volatile Tserial *uart;
  Tstatus stat;
  Tcontrol ctrl;
  Tinterr interr;
  char filaENTR[QUEUE_SIZE]; int f_ent_tl;
  char filaSAIDA[QUEUE_SIZE]; int f_said_hd, f_said_tl, n_said;
  char c; int i;

  // os endereços da UART e da struct UARTdriver
  uart = (void *)IO_UART_ADDR;
  Ud_ptr = &Ud; 
  // inicialização da UART
  ctrl.rts   = 0;
  ctrl.speed = SPEED;
  ioctl(ctrl);
  // inicialização de UARTdriver
  Ud_ptr->rx_hd = 0;
  Ud_ptr->rx_tl = 0;
  Ud_ptr->tx_hd = 0;
  Ud_ptr->tx_tl = 0;
  Ud_ptr->nrx = 0;
  Ud_ptr->ntx = 16;
  // inicialização das filas
  f_said_hd = 0; 
  f_said_tl = 0;
  n_said = 0;
  f_ent_tl = 0;
  // ativa interrupções de recepção
  uart->interr.i = (UART_INT_progRX | UART_INT_progTX);
  ctrl.rts   = 1;
  ctrl.speed = SPEED;
  ioctl(ctrl); // sinaliza a UART que o programa irá transmitir

 
  do {
    // espero por caractere na fila de RX
    while ( (proberx()) == 0 ){ delay_cycle(1); }
    // leio um caractere da fila de RX
    c = Getc(); 
  
    if ( c == '\n' ){
     /* ATUALIZO A FILA */  
     filaENTR[ f_ent_tl ] = c;  
     /* RECEBI UM '\n', ISSO QUER DIZER QUE TEM UMA STRING COMPLETA */
     alinha( filaENTR );
     /*copia a STRING alinhada para a fila de saída*/       
     i = 0;
     while ( i < 21 ){
       filaSAIDA[ f_said_tl ] = filaENTR[ i ];
       i += 1;
       f_said_tl += 1;	
       n_said += 1;
     }
     f_ent_tl = 0;
    } else { /* c != '\n' */
      filaENTR[ f_ent_tl ] = c;
      f_ent_tl += 1;
    }    

    while ( n_said > 0 ){ 
      if ( (probetx()) > 0 ){ // insere octeto na fila de TX
	Putc( filaSAIDA[ f_said_hd ] );
	//to_stdout( filaSAIDA[ f_said_hd ] );
	f_said_hd += 1;
	n_said -= 1;
      }
    }
  }while ( c != EOT ); // vai até achar EOT
  
  while ( n_said > 0 ){
    if ( (probetx()) > 0 ){
      Putc( filaSAIDA[ f_said_hd ] );
      //to_stdout( filaSAIDA[ f_said_hd ] );
      f_said_hd += 1;
      n_said -= 1;
    }
  }

  delay_cycle( 200 );
  return 0;
}


void ioctl(Tcontrol ctrl){
  volatile Tserial *uart;

  uart = (void *)IO_UART_ADDR; // o endereço base da UART
  ctrl.ign   = 0;
  ctrl.ign4  = 0;
  uart->ctl  = ctrl;
}


Tstatus iostat(){
  volatile Tserial *uart;
  
  uart = (void *)IO_UART_ADDR; // o endereço base da UART
  
  return(uart->stat);
}


int proberx(){
  extern UARTdriver Ud;
  volatile UARTdriver *Ud_ptr;

  Ud_ptr = &Ud;
  
  return( Ud_ptr->nrx );
}


int probetx(){
  extern UARTdriver Ud;
  volatile UARTdriver *Ud_ptr;

  Ud_ptr = &Ud;
  
  return( Ud_ptr->ntx );
}


void Putc(char c){
  extern UARTdriver Ud;
  volatile UARTdriver *Ud_ptr;  
  volatile Tserial *uart;
  Tstatus stat;
  Tinterr interr;
  
  Ud_ptr = &Ud;
  uart = (void *)IO_UART_ADDR;

  if ( (probetx()) == 16 ){ // se a fila de transmissão está vazia    
    stat = iostat();
    if ( stat.txEmpty == 1 ){ // e txreg está vazio
      disableInterr();
      Ud_ptr->ntx -= 1; // diminui o numero de espaços
      Ud_ptr->tx_q[ Ud_ptr->tx_tl ] = c; // coloca na fila de transmissão
      Ud_ptr->tx_tl = ( Ud_ptr->tx_tl + 1 ) % Q_SZ; // aumenta a cauda da lista
      enableInterr();
      uart->interr.i |= UART_INT_setTX; // provoca interrupção de tx
    } else {
      disableInterr();
      Ud_ptr->ntx -= 1; // diminui o numero de espaços
      Ud_ptr->tx_q[Ud_ptr->tx_tl] = c; // coloca na fila de transmissão
      Ud_ptr->tx_tl = ( Ud_ptr->tx_tl + 1 ) % Q_SZ; // aumenta a cauda da lista
      enableInterr();
    } 
  } else {
    if ( (probetx()) > 0 ){
      disableInterr();
      Ud_ptr->ntx -= 1; // diminui o numero de espaços
      Ud_ptr->tx_q[ Ud_ptr->tx_tl ] = c; // coloca na fila de transmissão
      Ud_ptr->tx_tl = ( Ud_ptr->tx_tl + 1 ) % Q_SZ; // aumenta a cauda da lista
      enableInterr();
    }
  }
}


char Getc(){
  extern UARTdriver Ud;
  volatile UARTdriver *Ud_ptr;
  char c;

  Ud_ptr = &Ud;

  disableInterr();
  c = Ud_ptr->rx_q[Ud_ptr->rx_hd];
  Ud_ptr->rx_hd = (Ud_ptr->rx_hd + 1) % Q_SZ;
  Ud_ptr->nrx -= 1;
  enableInterr();

  return c;
}


void alinha(char *str){
  char temp[21];
  int i, j;

  i = 0;
  while(str[i] != '\n'){
    temp[i] = str[i];
    i += 1;
  }
  temp[i] = str[i];

  //i == indice de '\n' em temp.
  j = 20;
  while(j >= 0){
    if(i >= 0){
      str[j] = temp[i];
      i -= 1;
    }else{
      str[j] = ' ';
    }
    j -= 1;
  }

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
