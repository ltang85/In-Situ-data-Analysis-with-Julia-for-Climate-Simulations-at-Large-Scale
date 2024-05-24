This documentation shows how to run the E3SM in situ framework with the streaming Julia modules on LANL's Grizzly supercomputer.  
  
Contact: Li Tang (ltang@lanl.gov)  
  
<h1>STEP 1. Install and run E3SM example on Grizzly</h1>

Setup an E3SM case:  
  
```
git clone git@github.com:E3SM-Project/E3SM.git  
cd E3SM  
git fetch origin  
git reset --hard origin/vanroekel/lanl_dr  
git submodule update --init  
cd cime/scripts  
./create_newcase -case /users/sdutta/climate/e3sm_case_1 -compset FC5AV1C-L -mach grizzly -res ne4_ne4 -compiler gnu --pecount S  
```

Set the path of the input data and caserun infomation (e.g., run the model for 5 months) for the created E3SM case in **env_run.xml**:  
  
```
DIN_LOC_ROOT = "your input_data path"  
STOP_OPTION = nmonths  
STOP_N = 5
```

Configure **user_nl_cam** and add the following to the configuration file (create it if it does not exist):  
```
avgflag_pertape = 'A','A','I','A','A'  
fincl1 = 'extinct_sw_inp','extinct_lw_bnd7','extinct_lw_inp'  
fincl2 = 'FLUT', 'PRECT', 'U200', 'V200', 'U850', 'V850', 'Z500', 'OMEGA500', 'UBOT', 'VBOT',  
'TREFHT', 'TREFHTMN', 'TREFHTMX', 'QREFHT', 'TS', 'PS', 'TMQ', 'TUQ', 'TVQ'  
fincl3 = 'PSL','T200','T500','U850','V850','UBOT','VBOT','TREFHT'  
fincl4 = 'FLUT','U200','U850','PRECT','OMEGA500'  
fincl5 = 'PRECT','PRECC'  
mfilt = 1,30,120,120,240  
nhtfrq = 0,-24,-6,-6,-3
```

Test run:  
  
```
./case.setup  
./case.build  
./case.submit 
```


  
<h1>STEP 2. Add Julia compilation and linking support to E3SM for Grizzly</h1>  

Add include path to E3SM/components/CMakeLists.txt (line 370):  
```
set(INCLDIR "${INCLDIR} -I/turquoise/users/ltang/Install/julia-1.4.0/include/julia")  
```

  
Add library path to E3SM/cime/scripts/Tools/Makefile (line 956):    
```
JULIALIBS = -L'/turquoise/users/ltang/Install/julia-1.4.0/lib' -Wl,--export-dynamic -Wl,-rpath,'/turquoise/users/ltang/Install/julia-1.4.0/lib' -Wl,-rpath,'/turquoise/users/ltang/Install/julia1.4.0/lib/julia' -ljulia  
```


Setup correct dependencies in cime/config/e3sm/machines/config_machines.xml. For example, the following works for Grizzly.  
```
</modules>  
<modules compiler="gnu">  
<command name="load">gcc/8.3.0</command>  
</modules>  
<modules compiler="intel">  
<command name="load">intel/17.0.1</command>  
</modules>  
<modules mpilib="openmpi">  
<command name="load">openmpi/2.1.2</command>  
</modules>  
<modules mpilib="mvapich">  
<command name="load">mvapich2/2.3</command>  
</modules>  
<modules>  
<command name="load">netcdf/4.7.4</command>  
</modules>  
<modules>  
```

  
<h1>STEP 3. Run the E3SM+Julia example on Grizzly</h1>  

Add the following declaration in subroutine stepon_run3 of stepon.F90  
```
real(r8) :: temp_to_julia(nlev)  
byte :: res_julia  
byte,external :: jl_eval  
byte,external :: jl_call_1d_double_array  
byte,external :: jl_call_subroutine   
```

  
Put the following code between “#endif” and “subroutine stepon_run3”  
```
if (iam<96) then ! 96 causes invalid memory reference for temp  
  tl_f = TimeLevel%n0  
  do k=1,nlev  
    temp_to_julia(k) = dyn_in%elem(1)%state%T(1,1,k,tl_f)  
  enddo  
  print*,"===MPI RANK",iam,"TIME STEP",get_nstep(),"TEMP",temp_to_julia(1:4)  
  if (is_first_step()) then  
    call jl_init()  
  endif  
  res_julia = jl_call_subroutine("streamean", temp_to_julia(1:4), 4, iam, get_nstep())  
  if (is_last_step()) then  
    call jl_exit()  
  endif  
endif  
```

  
Copy the interface.c file in the cam folder E3SM/components/cam/src/dynamics/se, and copy the Julia file insitu.jl to the case run folder (e.g., /lustre/scratch3/turquoise/sdutta/E3SM/e3sm_case_1/run). Then submit the job.    

  
  
