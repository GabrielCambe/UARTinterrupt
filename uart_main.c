#include "cMIPS.h"
#include "uart_defs.h"

#define QUEUE_SIZE 210
#define SPEED 3

Tstatus iostat(void);
void ioctl(Tcontrol);
int proberx(void); // retorna "nrx": o número de  caracteres na fila de recepção
int probetx(void); // retorna "ntx"
char Getc(void);
void Putc(char);
void alinha(char *str);


int main(void){
  
  extern UARTdriver Ud;
  volatile UARTdriver *Ud_ptr;

  volatile Tserial *uart;
  Tstatus stat;
  Tcontrol ctrl;
  Tinterr interr;
  
  char filaENTR[QUEUE_SIZE]; int f_ent_tl;
  char filaSAIDA[QUEUE_SIZE]; int f_said_hd, f_said_tl, n_said;

  char c; int i, newline;

  uart = (void *)IO_UART_ADDR; // o endereço base da UART
  Ud_ptr = &Ud; // passa o endereço da variável Ud para o ponteiro
   
  ctrl.rts   = 0;
  ctrl.speed = SPEED;
  ioctl(ctrl); // inicializa UART
  

  Ud_ptr->rx_hd = 0;
  Ud_ptr->rx_tl = 0;
  Ud_ptr->tx_hd = 0;
  Ud_ptr->tx_tl = 0;
  Ud_ptr->nrx = 0;
  Ud_ptr->ntx = 16;

 
  uart->interr.i = UART_INT_progRX;// ativa interrupções de recepção
  ctrl.rts   = 1;
  ctrl.speed = SPEED;
  ioctl(ctrl); // sinaliza a UART que o programa irá transmitir

  
  f_said_hd = -1; // inicialização das filas
  f_said_tl = -1;
  n_said = 0;
  f_ent_tl = -1;
  newline = 0;
  
  do {
    while ( (proberx()) == 0 ){ // existe algum caractere na fila de RX? 
      delay_cycle(1);
    }    
    c = Getc(); // leio um caractere da fila de RX
    //to_stdout( c );
    
    if(c == '\n'){
      if(newline == 1){ //são 2 newlines seguidos		
	c = EOT;
	continue;
      }else{ // newline != 1
	/* RECEBI UM '\n', ISSO QUER DIZER QUE TEM UMA STRING COMPLETA */


	if(f_ent_tl >= 0){  /* ATUALIZO A FILA E ALINHO A STRING À DIREITA */
	  newline = 1;
	  f_ent_tl += 1; //atualiza fila de entrada
	  filaENTR[f_ent_tl] = c;
	}else{
	  newline = 1;
	  f_ent_tl = 0; //atualiza fila de entrada
	  filaENTR[f_ent_tl] = c;
	}

	 alinha(filaENTR); //alinha a STRING recebida da fila de entrada.


	if(f_said_tl < 0){ /* copiando a STRING alinhada para a fila de saída. */ 

	  i = 0;
	  while(i <= 20){
	    filaSAIDA[i] = filaENTR[i];
	    //to_stdout( filaSAIDA[i] );////////////////////////////////
	    i += 1;
	  }
	  f_said_tl = 20;
	  n_said = 21;
	  f_ent_tl = -1;
	}else{
	  
	  f_said_tl += 1;	
	  i = 0; 
	  while(i <= 20){
	    filaSAIDA[(f_said_tl + i)] = filaENTR[i];
	    //to_stdout( filaSAIDA[f_said_tl + i] );////////////////////////////////
	    i += 1;
	  }
	  f_said_tl += 20;
	  n_said += 21;
	  f_ent_tl = -1; 
	}
      }
      
    }else{ // c != '\n'
      if(f_ent_tl >= 0){  /* ATUALIZO A FILA E ALINHO ELA À DIREITA */
	newline = 0;
	f_ent_tl += 1; //atualiza fila de entrada
	filaENTR[f_ent_tl] = c;
      }else{
	newline = 0;
	f_ent_tl = 0; //atualiza fila de entrada
	filaENTR[f_ent_tl] = c;  
      }
      
    }

    //if((probetx()) > 0){
      if(f_said_hd < 0){ // insere octeto na fila de TRANSMISSÃO
	f_said_hd = 0;
	Putc( filaSAIDA[f_said_hd] );
	//to_stdout( filaSAIDA[f_said_hd] );//////////////////////////////////////////////////////  
	f_said_hd = (f_said_hd + 1) % QUEUE_SIZE;
	n_said -= 1;
      }else{
	Putc( filaSAIDA[f_said_hd] );
	//to_stdout( filaSAIDA[f_said_hd] );////////////////////////////////////////////////////
	f_said_hd = (f_said_hd + 1) % QUEUE_SIZE;
	n_said -= 1;
      }    
      // }
    
  }while (c != EOT); // vai até achar EOT

  while (n_said >= 0){
    if(f_said_hd < 0){ // insere octeto na fila de TRANSMISSÃO
      f_said_hd = 0;
      Putc( filaSAIDA[f_said_hd] );
      //to_stdout( filaSAIDA[f_said_hd] );//////////////////////////////////////////////////////  
      f_said_hd = (f_said_hd + 1) % QUEUE_SIZE;
      n_said -= 1;
    }else{
      Putc( filaSAIDA[f_said_hd] );
      //to_stdout( filaSAIDA[f_said_hd] );////////////////////////////////////////////////////
      f_said_hd = (f_said_hd + 1) % QUEUE_SIZE;
      n_said -= 1;
    }
  }
  
  delay_cycle(200);
  exit(0);
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
  Tstatus status;
  
  uart = (void *)IO_UART_ADDR; // o endereço base da UART
  status = uart->stat;
  
  return(status);
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
  
  
  if((probetx()) == 16){ // a fila de transmissão está vazia
    stat = iostat();
    if(stat.txEmpty == 1){ // e txreg está vazio
      
      disableInterr();
      Ud_ptr->ntx -= 1; // diminui o numero de espaços
      Ud_ptr->tx_tl = (Ud_ptr->tx_tl + 1) % Q_SZ; // aumenta a cauda da lista
      Ud_ptr->tx_q[Ud_ptr->tx_tl] = c; // coloca na fila de transmissão
      enableInterr();
      
      uart->interr.i |= UART_INT_setTX; // provoca interrupção de tx
    }else{
      
      disableInterr();
      Ud_ptr->ntx -= 1; // diminui o numero de espaços
      Ud_ptr->tx_tl = (Ud_ptr->tx_tl + 1) % Q_SZ; // aumenta a cauda da lista
      Ud_ptr->tx_q[Ud_ptr->tx_tl] = c; // coloca na fila de transmissão
      enableInterr();
      
    }
  }else{
    
    disableInterr();
    Ud_ptr->ntx -= 1; // diminui o numero de espaços
    Ud_ptr->tx_tl = (Ud_ptr->tx_tl + 1) % Q_SZ; // aumenta a cauda da lista
    Ud_ptr->tx_q[Ud_ptr->tx_tl] = c; // coloca na fila de transmissão
    enableInterr();
    
  }
}

char Getc(){
  extern UARTdriver Ud;
  volatile UARTdriver *Ud_ptr;
  char c;

  Ud_ptr = &Ud;

  if(proberx() > 0){    
    disableInterr();// atualização da fila

    Ud_ptr->nrx -= 1;
    c = Ud_ptr->rx_q[Ud_ptr->rx_hd];
    Ud_ptr->rx_hd = (Ud_ptr->rx_hd + 1) % Q_SZ;
    
    enableInterr();
    return c;

  }else{
    return EOT;

  }
}


void alinha(char *str){
  char temp[21], c;
  int i_al, j;

  i_al = 0;
  while(str[i_al] != '\n'){
    temp[i_al] = str[i_al];
    i_al += 1;
  }
  temp[i_al] = str[i_al];

  //i == indice de '\n' em temp.
  j = 20;
  while(j >= 0){
    if(i_al >= 0){
      str[j] = temp[i_al];
      i_al -= 1;
      j -= 1;
      
    }else{
      str[j] = ' ';
      j -= 1;
    }
  }

  return;
}

//int *str2int(char *str){
  //int i, *out[21];
  //i = 0;
  //while(){
    
    //}
  //}


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
