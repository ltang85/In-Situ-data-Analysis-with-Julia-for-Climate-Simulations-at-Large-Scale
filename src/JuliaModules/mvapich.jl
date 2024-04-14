module mymodule

using MPI



function streaming(temp_from_cam, iam, Fcomm)
   Jcomm = MPI.Comm(Cint(Fcomm))

   r = MPI.Comm_rank(Jcomm) + 0.1
   println("Rank#: $r")
   sr = MPI.Reduce(r, +, 0, Jcomm)


   if iam == 0
     println("@@@JULIA@@@ ",sr)
   end

end

end
