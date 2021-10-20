IO_WRITE        EQU     FFFEh
TIMER_CONTROL   EQU     FFF7h
TIMER_VALUE     EQU     FFF6h
GSENSOR_X       EQU     FFEBh
INT_MASK        EQU     FFFAh
SP_ADDRESS      EQU     FDFFh
POSICAO_C       EQU     FFFCh
INT_MASK_VALUE  EQU     1000000000000000b ;int numero 15 do timer
;_----------------------------------------------------------------------------------------
;_-------------------------------------------------------------------------------------
;_-------------------------------------------------------------------------------------
;_-------------------------------------------------------------------------------------
;-------------------------------------------------------------------------------------;-------------------------------------------------------------------------------------
;_-------------------------------------------------------------------------------------
;_-------------------------------------------------------------------------------------
;_-------------------------------------------------------------------------------------

                ORIG    0000h
V0              EQU     0000h
POS0            EQU     0000h
                ;;;
                MVI     R6,2001h ;2001h e velocidade inicial
                MVI     R1,V0
                STOR    M[R6],R1
                INC     R6       ;2002h e posicao inicial
                MVI     R1,POS0
                STOR    M[R6],R1
                MVI     R6,2000h ;por posicao inicial na posicao atual tambem
                DEC     R6
                DEC     R6
                STOR    M[R6],R1
                ;;;
                MVI     R1,FFFEh
                MVI     R2,42d
BASEAST:        MVI     R3,80
                STOR    M[R1],R2
                INC     R5
                CMP     R3,R5
                BR.NN   BASEAST
                
                MOV     R5,R0
BASEESP:        MVI     R2,0
                MVI     R3,77
                STOR    M[R1],R2
                INC     R5
                CMP     R3,R5
                BR.NN   BASEESP
                
                MOV     R5,R0
BASEAST2:       MVI     R2,42d
                MVI     R3,80
                STOR    M[R1],R2
                INC     R5
                CMP     R3,R5
                BR.NN   BASEAST2
             
                MOV     R1,R0
                MOV     R2,R0
                MOV     R3,R0
                MOV     R4,R0
                MOV     R5,R0
                MVI     R6,8000h
                MOV     R7,R0

Main:
                 MVI     R3, INT_MASK
                 MVI     R2, INT_MASK_VALUE
                 STOR    M[R3], R2               ; set interrupt mask
                 ENI                             ; enable interrupts

                 MVI     R3, TIMER_VALUE
                 MVI     R2, 1d
                 STOR    M[R3], R2               ; set timer value = 0.1s

                 MVI     R2, 1
                 MVI     R3, TIMER_CONTROL
                 STOR    M[R3], R2               ; Start timer           

                 ; start code here
                    
MERMAO:         MVI     R6,7000h ;guarda tempo
                STOR    M[R6],R1
                ;;
                MVI     R1,POSICAO_C
                MVI     R4,0000000100000000b
                STOR    M[R1],R4
                MVI     R1,IO_WRITE
                MVI     R4,42d
                STOR    M[R1],R4
                
                MVI     R6,4000h
                MVI     R1,POSICAO_C
                LOAD    R4,M[R6] ;METE EM R4 O CURSOR
                STOR    M[R1],R4
                
                CMP     R4,R0 ;se cursor e 0 
                BR.Z    SKIPCURSOR
                MOV     R3,R0
                
                MVI     R1,IO_WRITE
                MVI     R3,0
                STOR    M[R1],R3 ;escreve espaco sem nada
                
SKIPCURSOR:     MOV     R4,R0
                
                MVI     R6, 6000h ;guardar jal do do timer la em baixo para ir para o rti
                STOR    M[R6],R7
                MVI     R1, POSICAO_C
                
                MVI     R6,2000h ;por em R6 o endereco da posicao
                DEC     R6
                DEC     R6
                
                LOAD    R2,M[R6] ;POR em r2 o valor da posicao
                SHR     R2
                SHR     R2
                SHR     R2 
                SHR     R2
                SHR     R2
                SHR     R2 ;VALOR EM METROS
                
                MVI     R3,0000000100000000b ;adicionar ao cursor o numero da linha
                ADD     R2,R3,R2 ;ADICIONA
                
                MVI     R6,4000h ;guarda em 4000h o cursor da bola
                STOR    M[R6],R2
                
                STOR    M[R1],R2 ;coloca cursor
                MOV     R2,R0
                
                MVI     R3,111d
                MVI     R1, IO_WRITE ;escreve bola
                STOR    M[R1],R3
                
                JAL     ACELERACAOX
                JAL     VELOCIDADE
                JAL     POSICAO
                
                MVI     R6,2000h ;se velocidade e 0 entao velocidade e velocidade apos 1st ressalto 
                DEC     R6
                LOAD    R1,M[R6]
                CMP     R1,R0
                JAL.Z   VELEPOSAPOSRESSALTO
                
                MVI     R6,7000h ;recuperar tempo
                LOAD    R1,M[R6]
                ;;;;;;;;;;;
                
                MVI     R2,1380h    ;78 2*6 EM HEXA PARA FAZER CONDICAO
                
                MVI     R6,2000h
                DEC     R6
                DEC     R6
                LOAD    R4,M[R6] ;colocar em R4 o valor da posicao
                CMP     R4,R2 ;se o valor da posicao for maior que 78 vai para funcao ressalto
                JAL.NN  RESSALTINHO
                
                MVI     R6,2000h
                DEC     R6
                DEC     R6
                LOAD    R4,M[R6] ;colocar em R4 o valor da posicao
                CMP     R4,R0 ;se o valor da posicao for menor que 0 vai para funcao ressaltoes
                JAL.N   RessaltinhoEs
                            
                MVI     R6,6000h
                LOAD    R7,M[R6] ;salta para onde mermao foi chamado
                JMP     R7
               
;-------------------------------------------------------------------------------
                
ACELERACAOX:    MVI     R6,8000h
                STOR    M[R6],R7 ;guarda na memoria 8000h decrescendo os JALS main
                MVI     R2,GSENSOR_X 
                LOAD    R2,M[R2] ;ACELERACAO DA PLACA
                MVI     R1,0002h ;(9.8/255) * 2**6
                
                CMP     R2,R0 ;ve se aceleracao e negativa, se for entao passa 
                                 ;para positiva para fazer o produto
                BR.P    SKIPNEG2
                
                NEG     R2
                
SKIPNEG2:       MVI     R6,3000h
                
                JAL     Produto
                
                MVI     R2,GSENSOR_X
                LOAD    R2,M[R2]
                CMP     R2,R0
                BR.P    SKIPNEG3
                
                NEG     R3
                
SKIPNEG3:       MVI     R6,7000h
                LOAD    R1,M[R6] ;restaura tempo
                MOV     R5,R0
                
                MVI     R6,2000h
                STOR    M[R6],R3 ;guarda aceleracao no 2000h
                
                MVI     R6,8000H
                LOAD    R7,M[R6] ;vai para os main JALS 
                JMP     R7

                ;v=v0+at
VELOCIDADE:     MVI     R6,8000h
                DEC     R6
                STOR    M[R6],R7
                
                MVI     R6,2000H
                DEC     R6
                DEC     R6
                LOAD    R1,M[R6]
                MVI     R2,0040H
                CMP     R2,R1
                JAL.Z   TEMPINHO
                
                MVI     R6,7000H
                LOAD    R1,M[R6] ;restaura tempo para r1
                
                MOV     R2,R3 ;ACELERACAOX esta em 2**6
                MVI     R6,3000h ;para guardar carry's de R5
                ;;;;;
                CMP     R2,R0
                BR.NN   PASSARNEG3 ;se aceleracao nao for negativa passar a linha seguinte
                
                NEG     R2 ;negar aceleracao para n dar overflow
                
PASSARNEG3:     JAL     Produto ;aceleracaox * tempo 
                ;;;
                INC     R6
                LOAD    R5,M[R6] ;buscar carry's 
                ADD     R3,R3,R5 ;somar carry's
                CMP     R5,R0 
                JMP.NZ  R7 ;se houver carry's ainda, voltar a a busca-los e somar a R3
                ;;;
                MVI     R6,2000h
                LOAD    R2,M[R6];ir buscar valor da aceleracao da funcao anterior
                CMP     R2,R0
                BR.NN   PASSARNEG4 ;se aceleracao nao for negativa passar a linha seguinte
                
                NEG     R3 ;negar resultado para compensar a negacao de R2 acima
                ;;;
PASSARNEG4:     MVI     R4, V0 ;velocidade inicial esta em 2**6
                
                MVI     R6,2001h
                LOAD    R4,M[R6] ;vai buscar a 2001h a velocidade 
                            
VOLTAR:         ADD     R3,R3,R4 ;at + velocidade inicial
                MVI     R6,2000h 
                DEC     R6 
                STOR    M[R6],R3 ;guarda velocidade no 1fffh
                
                MVI     R6,8000H
                DEC     R6
                LOAD    R7,M[R6] ;vai para os main JALS
                JMP     R7
                
                ;x=x0+vt
POSICAO:        MVI     R6,2002h
                LOAD    R4,M[R6] ;posicao inicial para r4

                MVI     R6,2000H  ;AQUI TOU A IR BUSCAR A POSICAO 
                DEC     R6        ;A MEMORIA, SE ELA FOR IGUAL A 78 ENTAO
                DEC     R6        ;ENTAO POSICAO INICIAL VAI SER 78
                LOAD    R5,M[R6]
                MVI     R2,1380H
                CMP     R5,R2
                BR.NN   XNEG ;se bola esta em 78 entao P0 e 78 
                
VOLTAR2:        MOV     R2,R0
                MOV     R5,R0
                
                MVI     R6,8000h
                DEC     R6
                DEC     R6
                STOR    M[R6],R7 ;para voltar para os main jals
               
                MVI     R6,7000H
                LOAD    R1,M[R6] ;restaura tempo para r1
               
                MVI     R6,2000h
                DEC     R6
                LOAD    R2,M[R6];ir buscar valor da velocidade da funcao anterior
                CMP     R2,R0
                BR.NN   PASSARNEG1 ;se velocidade nao for negativa passar a linha seguinte
                
                NEG     R2 ;negar velocidade para n dar overflow
                
PASSARNEG1:     MVI     R6,3000h
                
                JAL     Produto ;velocidade anterior x tempo   
                
                INC     R6
                LOAD    R5,M[R6] ;ir buscar os carrys e somar 
                ADD     R3,R3,R5
                CMP     R5,R0
                JMP.NZ  R7 ;se ainda houver carrys voltar a busca-los e somar
                
                MVI     R6,2000h
                DEC     R6
                
                LOAD    R2,M[R6];ir buscar valor da velocidade da funcao anterior
                CMP     R2,R0
                BR.NN   PASSARNEG2 ;se velocidade nao for negativa passar a linha seguinte
                
                NEG     R3 ;negar resultado para compensar a negacao de R2 acima
                
                MVI     R6,2002H
                LOAD    R4,M[R6]
                ;;;
PASSARNEG2:     MVI     R6,2002h
                LOAD    R4,M[R6]
                ;;;
                ADD     R3,R3,R4 ; x0 e posicao inicial que esta em R4
                MVI     R6,2000h 
                DEC     R6
                DEC     R6 
                STOR    M[R6],R3 ; guardar posicao em 2000h dec dec dec
                
                MVI     R6,8000h
                DEC     R6
                DEC     R6
                LOAD    R7,M[R6] ;voltar para main jals
                
                JMP     R7
                
;---------------------------------------------------------
VELEPOSAPOSRESSALTO:
                MVI     R6,2001h ;buscar velocidade apos 1st ressalto
                LOAD    R1,M[R6]
                MVI     R6,2000h ;por como velocidade atual
                DEC     R6
                STOR    M[R6],R1
                
                MVI     R6,2002h ;buscar pos apos 1st ressalto
                LOAD    R1,M[R6]
                MVI     R6,2000h ;por como velocidade atual
                DEC     R6
                DEC     R6
                STOR    M[R6],R1
                
                JMP     R7
                
TEMPINHO:       MVI     R6,7000H
                MVI     R1,0010H
                STOR    M[R6],R1
                JMP     R7
;---------------------------------------------------------------
RESSALTINHO:    MVI     R6,6000H
                DEC     R6
                DEC     R6
                STOR    M[R6],R7 ;GUARDAR ENDERECO DE ONDE RESSALTINHO FOI CHAMADO
                
                MVI     R6,2000H
                DEC     R6
                LOAD    R5,M[R6] ;VELOCIDADE ESTA EM R5
                
                CMP     R5,R0
                BR.N    SKIPNEG
                NEG     R5
                
SKIPNEG:        CMP     R5,R0
                BR.Z    PASSARVELSIMETRICA
                
                STOR    M[R6],R5 ;GUARDAR NA PILHA VALOR DA VELOCIDADE SIMETRICA              
         
                MVI     R6,2000h
                INC     R6
                STOR    M[R6],R5 ;guarda na pilha no endereco 2001h a velocidade simetrica
                
                ;;;;;;;;;;;;
                MVI     R6,2000h ;por 78 no 2002h quando ressalta
                INC     R6
                INC     R6
                MVI     R5,1380h
                STOR    M[R6],R5
                ;;;;;;;;;;;;
                
VOLTAR3:        MVI     R6, 7000H
                MOV     R5,R0
                STOR    M[R6],R5  ;TENDO EM CONTA QUE RESSALTOU AGR O TEMPO REINICIA
                
                MVI     R6,2000H
                DEC     R6
                DEC     R6
                MVI     R5,1380h ;CORRESPONDE A 78*2**6 EM HEXA
                STOR    M[R6],R5 ;coloca posicao com o valor 78
                MOV     R1,R0
                
                JMP     R7 ;salta para jal ressaltinho

XNEG:           MOV     R4,R2 
                MVI     R6,2002H ;GUARDA O VALOR DE R4 EM 2002
                STOR    M[R6],R4
                BR      VOLTAR2
                
PASSARVELSIMETRICA:     
                MVI     R6,2001h ;buscar valor velocidade apos 1st ressalto 
                LOAD    R5,M[R6]
                MVI     R6,2000h ;guardar como propria velocidade
                DEC     R6
                STOR    M[R6],R5
                BR      VOLTAR3
                
PASSARVELSIMETRICA2:
                MVI     R6,2001h ;buscar valor velocidade apos 1st ressalto 
                LOAD    R5,M[R6]
                MVI     R6,2000h ;guardar como propria velocidade
                DEC     R6
                STOR    M[R6],R5
                BR      VOLTAR3es                
  ;.....................................................................   
  ;.....................................................................   
  ;.....................................................................   
  ;.....................................................................   
RessaltinhoEs:  MVI     R6,6000H
                DEC     R6
                DEC     R6
                DEC     R6
                STOR    M[R6],R7 ;GUARDAR ENDERECO DE ONDE RESSALTINHOES FOI CHAMADO
                
                MVI     R6,2000H
                DEC     R6
                LOAD    R5,M[R6] ;VELOCIDADE ESTA EM R5
                
                CMP     R5,R0 ;se velocidade com que bate na parede e negativa, torna-a positiva
                BR.P    SKIPPOS
                NEG     R5
                
SKIPPOS:        CMP     R5,R0
                BR.Z    PASSARVELSIMETRICA2
                STOR    M[R6],R5 ;GUARDAR NA PILHA VALOR DA VELOCIDADE SIMETRICA              
         
                MVI     R6,2000h
                INC     R6
                STOR    M[R6],R5 ;guarda na pilha no endereco 2001h a velocidade simetrica
                                    
                ;;;;;;;;;;;;
                MVI     R6,2000h ;por x=1 no 2002h quando ressalta
                INC     R6
                INC     R6
                MVI     R5,0040h
                STOR    M[R6],R5
                ;;;;;;;;;;;
VOLTAR3es:      MVI     R6, 7000H
                MOV     R5,R0
                STOR    M[R6],R5  ;TENDO EM CONTA QUE RESSALTOU AGR O TEMPO REINICIA
                
                MVI     R6,2000H
                DEC     R6
                DEC     R6
                MVI     R5,0040h ;CORRESPONDE A 1*2**6 EM HEXA
                STOR    M[R6],R5 ;coloca posicao com o valor 1
                MOV     R1,R0
                
                JMP     R7 ;salta para jal ressaltinhoes                               
;----------------------------------------------
               
Produto:        MOV     R3, R0
                CMP     R2, R0
                JMP.Z   R7
                
Loop:           ADD     R3, R3, R1
                BR.C    CARRY ;se houver carry ir para funcao carry
RESUMIR:        DEC     R2
                BR.NZ   Loop

SHIFTS:         MVI     R4,6 ;SHR 6 vezes para obter as 6 casa decimais pois valor esta com 12 casas decimais
                BR      SOBRE2
COMPSOBRE2:     CMP     R4,R0
                BR.NZ   SOBRE2   
                JMP     R7 ;acaba os shifts, volta para a funcao

SOBRE2:         SHR     R3
                DEC     R4
                BR      COMPSOBRE2 
                
CARRY:          MVI     R5,0400h ;valor do numero que foi perdido, ja com 6 SHR 
                STOR    M[R6],R5 ;guardar carry's em 3000h, decrescendo
                DEC     R6
                BR      RESUMIR ;continuar com o produto
                              
;------------------------TEMPORIZADOR---------------------------------------------------

                 
SUPERERRO:      MVI     R6,6000H ;guarda chamada do jal supererro
                DEC     R6
                STOR    M[R6],R7
                INC     R1 ;aumenta tempo por 1
                SHL     R1 ;poe tempo em 2**4
                SHL     R1
                SHL     R1
                SHL     R1
                
                MVI     R2, 1
                MVI     R3, TIMER_CONTROL
                STOR    M[R3], R2 ; Restart timer
                 
                JAL     MERMAO
                MVI     R6,6000h
                DEC     R6
                LOAD    R7,M[R6]
                
                JMP     R7
                              
;-------------- Interrupts------------------------------------------------------

                 ; Timer
                ORIG    7FF0h ;handler do timer salta para aqui de 0,1s em 0,1s
                JAL     SUPERERRO
                SHR     R1 ;poe tempo a dividir por 2**4 p compensar
                SHR     R1
                SHR     R1
                SHR     R1
                RTI