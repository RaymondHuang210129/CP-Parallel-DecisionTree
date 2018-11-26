build the random forest seperate with each rank.(i.e.    100trees,4 threads , then 25 trees for each thread)
validate the shared data(from rank=0), reduce the result to rank0,compute the result
