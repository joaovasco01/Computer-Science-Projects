                ORIG    0000h
SINTAB          STR     0 ,1 ,2 ,3 ,4 ,5 ,6 ,7 ,8 ,10 ,11 ,12 ,13 ,14 ,15 ,16 ,17 ,18 ,19 ,20 ,21 ,22 ,23 ,25 ,26 ,27 ,28 ,29 ,30,31 ,31 ,32 ,33 ,34 ,35 ,36 ,37 ,38 ,39 ,40 ,41 ,41 ,42 ,43 ,44 ,45 ,46 ,46 ,47 ,48 ,49 ,49 ,50 ,51 ,51 ,52 ,53 ,53 ,54 ,54 ,55 ,55 ,56 ,57 ,57 ,58 ,58 ,58 ,59 ,59 ,60 ,60 ,60 ,61 ,61 ,61 ,62 ,62 ,62 ,62 ,63 ,63,63 ,63 ,63 ,63 ,63 ,63 ,63 ,63 ,64

ANGULO          EQU     45  
GRAV            EQU     627 ;9.8*2**6
V0              EQU     128 ;2*2**6

                ORIG    1000h
TEMPO           STR     0, 64 ,128 ,192 ,256 ,320 ,384 ,448 ,512 ,576 ,640 ,704 ,768 ,832 ,896 ,960 ,1024 ,1088 ,1152 ,1216 ,1280 ,1344 ,1408 ,1472 ,1536 ,1600 ,1664 ,1728 ,1792 ,1856 ,1920 ,1984 ,2048 ,2112 ,2176 ,2240 ,2304 ,2368 ,2432 ,2496 ,2560 ,2624 ,2688 ,2752 ,2816 ,2880 ,2944 ,3008 ,3072 ,3136 ,3200 ,3264 ,3328 ,3392 ,3456 ,3520 ,3584 ,3648 ,3712 ,3776 ,3840 ,3904 ,3968 ,4032 ,4096 ,4160 ,4224 ,4288 ,4352 ,4416 ,4480 ,4544 ,4608 ,4672 ,4736 ,4800 ,4864 ,4928 ,4992 ,5056 ,5120 ,5184 ,5248 ,5312 ,5376 ,5440 ,5504 ,5568 ,5632 ,5696 ,5760
POS0            EQU     0
                MVI     R6,8000h
                
                JAL     ACELERACAOX
UPDATE_TEMPO:   INC     R1 
                MVI     R6,256
                CMP     R1,R6
                BR.NZ   UPDATE_TEMPO
                JAL     VELOCIDADE
                JAL     POSICAO                
FIM:            BR      FIM
        
;-------------------------------------------------------------------------------
                
ACELERACAOX:    STOR    M[R6],R7 ;guarda na memoria 8000h decrescendo os JALS main
                MVI     R1,ANGULO
                LOAD    R1,M[R1] ;sen(angulo)*2*6
                MVI     R2,GRAV ;9.8*2**6
                
                JAL     Produto
                
                MVI     R6,2000h
                STOR    M[R6],R3 ;guarda aceleracao no 2000h
                MVI     R6,8000H
                LOAD    R7,M[R6] ;vai para os main JALS 
                JMP     R7
                
                ;v=v0+at
VELOCIDADE:     MVI     R6,8000h
                DEC     R6
                STOR    M[R6],R7
                
                MOV     R2,R3 ;ACELERACAOX esta em 2**6
                MVI     R6,3000h ;para guardar carry's de R5
                
                JAL     Produto ;aceleracaox x tempo 
                
                INC     R6
                LOAD    R5,M[R6] ;buscar carry's 
                ADD     R3,R3,R5 ;somar carry's
                CMP     R5,R0 
                JMP.NZ  R7 ;se houver carry's ainda, voltar a a busca-los e somar a R3
                
                MVI     R4, V0 ;velocidade inicial ta em 2**6
                ADD     R3,R3,R4 ;at + velociade inicicial
                MVI     R6,2000h 
                DEC     R6 
                STOR    M[R6],R3 ;guarda velocidade no 1fffh
                
                MVI     R6,8000H
                DEC     R6
                LOAD    R7,M[R6] ;vai para os main JALS
                JMP     R7
                
                ;x=x0+vt
POSICAO:        MVI     R4,POS0
                MVI     R6,8000h
                DEC     R6
                DEC     R6
                STOR    M[R6],R7 ;para voltar para os main jals
               
                MOV     R2,R3 ;ir buscar valor da velocidade da funcao anterior 
                MVI     R6,3000h
                
                JAL     Produto ;velocidade anterior x tempo
                       
                INC     R6
                LOAD    R5,M[R6] ;ir buscar os carrys e somar 
                ADD     R3,R3,R5
                CMP     R5,R0
                JMP.NZ  R7 ;se ainda houver carrys voltar a busca-los e somar
                
                ADD     R3,R3,R4 ; x0 e posicao inicial que esta em R4
                MVI     R6,2000h 
                DEC     R6
                DEC     R6 
                STOR    M[R6],R3 ; guardar posicao em 1ffeh
                
                MVI     R6,8000h
                DEC     R6
                DEC     R6
                LOAD    R7,M[R6] ;voltar para main jals
                
                JMP     R7
                
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
                