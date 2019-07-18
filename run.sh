  
  #Default
  for i in `seq 0 9`
  do
     ./arraySum.out -c 0 >> results2.txt
  done

  #Worst
  for i in `seq 0 9`
  do
     ./arraySum.out -c 1 >> results2.txt
  done

  #Best
  for i in `seq 0 9`
  do
     ./arraySum.out -c 2 >> results2.txt
  done

