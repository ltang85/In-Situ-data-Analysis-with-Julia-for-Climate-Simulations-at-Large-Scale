module mymodule

using MPI
using Statistics
using Interpolations

function streaming(data_array, nstep, Fcomm)
#    Jcomm = MPI.Comm(Cint(Fcomm))


#    println("@@JULIA@@ MPI# ", MPI.Comm_rank(Jcomm), " Nstep ", nstep, " Len ", local_length, " Sum ",local_sum)

  println("@@@@")


#     rr = MPI.Comm_rank(Jcomm)
#     trr = MPI.Reduce(rr, +, 0, Jcomm)

#    if MPI.Comm_rank(Jcomm) == 0
#       println("@@ sum",trr)
#    end



end


end


