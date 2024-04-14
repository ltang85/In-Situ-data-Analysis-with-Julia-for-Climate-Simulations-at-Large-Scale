module mymodule

using MPI
using Statistics
using Interpolations

function streaming(data_array, nstep, Fcomm)
    Jcomm = MPI.Comm(Cint(Fcomm))

    TH_min = 57 #59.75
    TH_max = 64 #60.25

    lat_array = zeros(Int(length(data_array)/4))
    lon_array = zeros(Int(length(data_array)/4))
    uvel_filtered1 = zeros(Int(length(data_array)/4))
    uvel_filtered2 = zeros(Int(length(data_array)/4))
    
    index = 1
    for i = 1:4:length(data_array)
        lat_array[index] = data_array[i]
        lon_array[index] = data_array[i+1]
        uvel_filtered1[index] = data_array[i+2]
        uvel_filtered2[index] = data_array[i+3]
        index = index + 1
    end
 
    filtered_lat = Array{Float64}[]
    filtered_lon = Array{Float64}[]
    filtered_var1  = Array{Float64}[]
    filtered_var2  = Array{Float64}[]
    
    for i = 1:length(lat_array)
        if lat_array[i] > TH_min && lat_array[i] < TH_max
            push!(filtered_lat,[lat_array[i]])
            push!(filtered_lon,[lon_array[i]])
            push!(filtered_var1,[uvel_filtered1[i]])
            push!(filtered_var2,[uvel_filtered2[i]])
        end
    end

    local_length = 0
    local_sum = Float64(0)
    global_length = 0
    global_sum = Float64(0)
    
    if length(filtered_var2) > 0
        filtered_var  = Array{Float64,1}(undef,length(filtered_var2))
        ## Now interpolate the filtered values only
        for jj = 1:length(filtered_var2)
            xp = [8.37740438e+00, 1.14737872e+01] ## top and bottom hPA values available
            fp = [filtered_var1[jj], filtered_var2[jj]] ## uvel values at top and bottom layer
            itp = LinearInterpolation(xp,fp) # create interpolation function
            val = itp(10)
            filtered_var[jj]=val[1]
        end

        avgval = 0
        if length(filtered_var) > 0
            avgval = mean(filtered_var) 
        end

        local_length = length(filtered_var)
        local_sum = Float64(sum(filtered_var))
 
 
    end

    global_sum = MPI.Reduce(local_sum, +, 0, Jcomm)
    global_length = MPI.Reduce(local_length, +, 0, Jcomm)

    if MPI.Comm_rank(Jcomm) == 0
        if nstep == 0
            io = open(string(pwd(),"/e3sm.jout"),"w")
        end
        if nstep > 0
            io = open(string(pwd(),"/e3sm.jout"),"a")
            println(io,"Nstep $nstep GLOBAL Len $global_length Sum $global_sum Avg ",global_sum/global_length)
            close(io)
        end

#        println("@@@JULIA@@@ Nstep $nstep GLOBAL Len $global_length Sum $global_sum Avg ",global_sum/global_length)
    end

end


end


