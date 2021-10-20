from math import sin,pi

    
def acelerando (angulograu,gravi):                   #funcao aceleracao
    
    radiano = ((angulograu * 2*( pi))/360)        
    
    global aceleracaox
    
    aceleracaox = (sin(radiano))*gravi
    
    return (print((round(aceleracaox,2)), ' m/(s**2)'))


def velocidando (velocidadei , tempo):            #funcao velocidade
    
    global velocidadei2, tempo2, velocidadef
     
    tempo2 = tempo  
    
    velocidadei2 = velocidadei
    
    velocidadef = velocidadei2 + aceleracaox * tempo
     
    return (print ((round (velocidadef,2)),' m/s'))
    
    
def posicionando (posicaoi):                         #funcao posicao
    
    posicaof = posicaoi + (velocidadef) * (tempo2) 
    
    return (print((round(posicaof,2)),' no eixo'))
    
    
    
for tempo in range (4):                    #loop do tempo
    
    print ('--------------------------------------------')
    print ('Para o segundo ', tempo, ' a aceleracao é de: ')
    acelerando (45,9.8)
    print ('')
    print ('A velocidade é de: ')
    velocidando (2,tempo)
    print ('')
    print ('E a posição é de: ')
    posicionando(0)
    print ('')