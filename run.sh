  
  #Default
  for i in `seq 0 9`
  do
     ./arraySum.out -c 0 >> results.txt
  done

  #Worst
  for i in `seq 0 9`
  do
     ./arraySum.out -c 1 >> results.txt
  done

  #Best
  for i in `seq 0 9`
  do
     ./arraySum.out -c 2 >> results.txt
  done
