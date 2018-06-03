        .set noat
        .set noreorder
	# para operações com  UART.INTERR
        .set UART_clr_tx_irq, 0x10
	.set UART_clr_rx_irq, 0x08
	.set UART_prog_int_tx, 0x02
	.set UART_prog_int_rx, 0x01	
	# para comparações com UART.STATUS
	.set UART_tx_empty, 0x40
	.set UART_rx_full, 0x20
        .set UART_tx_irq, 0x10
	.set UART_rx_irq, 0x08
	# limpando o pipeline
        nop
	# pegando o endereço do buffer para o salvamento dos registradores
        lui	$k0, %hi(_uart_buff)
        nop
        ori	$k0, $k0, %lo(_uart_buff)        
        # O SALVAMENTO DOS REGISTRADORES DEVE SER ATOMICO, ENTÃO DEVE-SE INTERROMPER AS INTERRUPÇÕES
        sw $a0, 3*4($k0)
        sw $a1, 4*4($k0)
        sw $a2, 5*4($k0)
        sw $a3, 6*4($k0)
        sw $v0, 7*4($k0)
        sw $v1, 8*4($k0)
        sw $ra, 9*4($k0)

	# salva o endereço da UART
	lui	$k1, %hi(HW_uart_addr)
        nop
        ori	$k1, $k1, %lo(HW_uart_addr)

	# descobrindo a causa da interrupção
	lw 	$a0, USTAT($k1)
        nop
        sw	$a0, 0($k0)

        # pulando para o tratamento
	andi	$a3, $a0, UART_rx_irq
        nop
        bne	$a3, $zero, UARTrec
        nop
        
        lw	$a0, 0($k0)
        nop
        andi	$a3, $a0, UART_tx_irq
        nop
        bne	$a3, $zero, UARTtra
        nop
	
        j     _return
	nop

UARTrec:
	# limpando o interupt request
        lw $a0, UINTER($k1) # $a0 = INTERR da UART
        nop
        ori $a0, $a0, UART_clr_rx_irq
        nop
        sw $a0, UINTER($k1)

        lui    $a0, %hi(Ud) # carregando o endereço da struct Ud
        nop                       # Ud_ptr = &Ud;
        ori $a0, $a0, %lo(Ud)
        nop

        lw    $a1, NRX($a0) # O NÚMERO DE OCTETOS RECEBIDOS nrx AUMENTA
        nop
        addiu $a1, $a1, 1
        nop
        sw    $a1, NRX($a0)

        lw    $a3, RXTL($a0) # pegando o valor de Ud.rx_tl
        nop
        addu  $a3, $a3, $a0 # Ud + RX_TL

        lw    $a2, UDATA($k1) 	# lê o elemento do registrador rxreg
	nop
	sb    $a2, RX_Q($a3)       # guarda o elemento nacauda da fila
        
        lw    $a1, RXTL($a0) # pegando o valor de Ud.rx_tl
        nop
        addiu $a1, $a1, 1
        nop
        andi  $a1, $a1, (Q_SZ - 1)
        nop
        sw    $a1, RXTL($a0)

        j     _return
	nop

UARTtra:	
        # limpando o interupt request de TRANSMISSÃO
        lw $a0, UINTER($k1) # $a0 = INTERR da UART
        nop
        ori $a0, $a0, UART_clr_tx_irq
        nop
        sw $a0, UINTER($k1)
        nop

        lui    $a0, %hi(Ud) # carregando o endereço da struct Ud
        nop                  # Ud_ptr = &Ud;
        ori $a0, $a0, %lo(Ud)
        nop

        lw    $a1, NTX($a0) # O NÚMERO DE ESPAÇOS NA FILA DE TX AUMENTA
        nop
        addiu $a1, $a1, 1
        nop
        sw    $a1, NTX($a0)
        nop

        lw $a1, TXHD($a0) # calculando o valor do endreço do elemento
        nop
        addu $a1, $a1, $a0
        nop

        lb    $a2, TX_Q($a1) 	# lê o elemento da cabeça da fila
	nop
	sw    $a2, UDATA($k1)       # transmite o elemento

        lw    $a3, TXHD($a0) # Ud.tx_hd = (Ud.tx_hd + 1) % Q_SZ
        nop
        addiu $a3, $a3, 1
        nop
        andi  $a3, $a3, (Q_SZ - 1)
        nop
        sw    $a3, TXHD($a0)

	j     _return	
        nop

_return:
        # o momento de reconstriur o contexto de execução
        lui	$k0, %hi(_uart_buff)
        nop
        ori	$k0, $k0, %lo(_uart_buff)
        nop
        lw $a0, 3*4($k0)
        lw $a1, 4*4($k0)
        lw $a2, 5*4($k0)
        lw $a3, 6*4($k0)
        lw $v0, 7*4($k0)
        lw $v1, 8*4($k0)
        lw $ra, 9*4($k0)

        # limpando o pipeline
        nop
        eret			    # Return from interrupt
