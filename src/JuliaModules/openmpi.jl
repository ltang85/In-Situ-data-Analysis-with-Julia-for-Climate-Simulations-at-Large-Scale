module mymodule

using MPI



function streaming(temp_from_cam, iam, Fcomm)
   J = ccall((:MPI_Comm_f2c,"/usr/projects/hpcsoft/toss3/grizzly/openmpi/3.1.6-gcc-8.3.0/lib/libmpi.so"),Ptr{Cvoid},(Cint,),Cint(Fcomm))
   Jcomm = MPI.Comm(J)

   println("@@@JULIA@@@ iam",iam,"MPI RANK ",MPI.Comm_rank(Jcomm))

end

end
