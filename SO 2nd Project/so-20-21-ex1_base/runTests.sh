#!/bin/bash

#ciclo for que percorre os inputs
for a in $(ls ${1}/*.txt)
  do
  #ciclo for que percorre o programa para varios numeros de threads, ate ao dado
  for i in $(seq $3)
  do
    echo InputFile=$a Numthreads=$i

    inputWithoutDirectory="$(basename -- $a)";
    inputWithoutDirectory=$(echo $inputWithoutDirectory | cut -d '.' -f 1)

    #comando que permite so printar um determinado output
    ./tecnicofs $a $2/$inputWithoutDirectory-$i.txt $i | grep "TecnicoFS completed in"

  done

done
