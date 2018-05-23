 
	#para operações com  UART.INTERR
        .set UART_clr_tx_irq, 0x10
	.set UART_clr_rx_irq, 0x08
	.set UART_prog_int_tx, 0x02
	.set UART_prog_int_rx, 0x01

	
	#para comparações com UART.STATUS
	.set UART_tx_empty, 0x40
	.set UART_rx_full, 0x20
        .set UART_tx_irq, 0x10
	.set UART_rx_irq, 0x08


	#limpando o pipeline
	#nop

	
	#------------------------------------------------------------
	#pegando o endereço do buffer para o salvamento dos registradores
        lui	$k0, %hi(_uart_buff)
        ori	$k0, $k0, %lo(_uart_buff)        
        #O SALVAMENTO DOS REGISTRADORES DEVE SER ATOMICO, ENTÃO DEVE-SE INTERROMPER AS INTERRUPÇÕES
        sw $a0, 3*4($k0)
        sw $a1, 4*4($k0)
        sw $a2, 4*5($k0)
	sw $a3, 4*6($k0)

	#salva o endereço da UART
	lui	$k1, %hi(HW_uart_addr)
	ori	$k1, $k1, %lo(HW_uart_addr)

	#descobrindo a causa da interrupção
	lw 	$a0, USTAT($k1)
	sw	$a0, 0($k0)

	andi	$a3, $a0, UART_rx_irq
	beq	$a3, $0, 2*4
	j UARTrec
	nop

	#continua testando



UARTrec:
	#limpando o interupt request
        lw $a0, UINTER($k1) #$a0 = INTERR da UART
        nop
        ori $a0, $a0, UART_clr_rx_irq
        sw $a0, UINTER($k1)


        lui    $a0, %hi(Ud) #carregando o endereço da struct Ud (extern UARTdriver Ud)
        ori $a0, $a0, %lo(Ud)

        lw    $a1, NRX($a0) # Ud.nrx++
        nop
        addiu $a1, $a1, 1
        sw    $a1, NRX($a0)

        lw    $a3, RXTL($a0) # Ud.rx_tl = (Ud.rx_tl + 1) % Q_SZ
        nop
        addiu $a3, $a3, 1
        andi  $a3, $a3, (Q_SZ - 1)
        sw    $a3, RXTL($a0)


        addu  $a3, $a3, $a0 # Ud + RX_TL


        lw    $a2, UDATA($k1) 	# lê o elemento do registrador rxreg
	nop
	sb    $a2, RX_Q($a3)       # guarda o elemento nacauda da fila
		

        j     _return
	nop

	

	
#UARTtra:	

	# your code goes here
	
	#sw    $k1, UDATA($k0) 	# Read data from device


	#---------------------------------------------------
	# return	
	
_return:
        #o momento de reconstriur o contexto de execução
        lui	$k0, %hi(_uart_buff)
        ori	$k0, $k0, %lo(_uart_buff)

        lw $a0, 3*4($k0)
        lw $a1, 4*4($k0)
        lw $a2, 5*4($k0)
	lw $a3, 6*4($k0)
	

	eret			    # Return from interrupt


################################################################
#handle <- uart:
#    lê status da UART e guarda em memória
#salva contexto
#
#se (status.rxInterr == TRUE)
#trata recepção
#
#se (status.txInterr == TRUE)
#trata transmissão
#
#recompõe contexto
#
#    eret
