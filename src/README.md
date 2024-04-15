This documentation shows how to run the E3SM in situ framework with the streaming Julia module on LANL's Grizzly.  
  
Contact: Li Tang (ltang@lanl.gov)  
  
## STEP 1. Install and run E3SM example on Grizzly  
=====================================================================  
Get the repository: (make sure to add ssh key on github before you clone the repository)  
git clone git@github.com:E3SM-Project/E3SM.git  
cd E3SM  
git fetch origin  
git reset --hard origin/vanroekel/lanl_dr  
git submodule update --init  
Setup a new case:  
cd cime/scripts  
Now create the new case. In this step you can specify the model you want to run, what resolution  
of mesh you want and the compiler that will be used to build and run.  
./create_newcase -case /users/sdutta/climate/e3sm_case_1 -compset FC5AV1C-L -mach grizzly -res ne4_ne4 -compiler gnu --pecount S  
Here e3sm_case_1 is the name of the directory I gave. It can be anything where all the case specific files will be produced necessary for bulding and running.  
open file: env_run.xml  
set value for DIN_LOC_ROOT = /lustre/scratch3/turquoise/lvanroekel/E3SM/input_data (line 885)  
change STOP_OPTION to nmonths (this sets the unit for STOP_OPTION to months)  
STOP_N to 5 (this specifies that we are looking to run the model for 5 months) save and exit:  
env_run.xml  
open file: user_nl_cam  
add the following:  
avgflag_pertape = 'A','A','I','A','A'  
fincl1 = 'extinct_sw_inp','extinct_lw_bnd7','extinct_lw_inp'  
fincl2 = 'FLUT', 'PRECT', 'U200', 'V200', 'U850', 'V850', 'Z500', 'OMEGA500', 'UBOT', 'VBOT',  
'TREFHT', 'TREFHTMN', 'TREFHTMX', 'QREFHT', 'TS', 'PS', 'TMQ', 'TUQ', 'TVQ'  
fincl3 = 'PSL','T200','T500','U850','V850','UBOT','VBOT','TREFHT'  
fincl4 = 'FLUT','U200','U850','PRECT','OMEGA500'  
fincl5 = 'PRECT','PRECC'  
mfilt = 1,30,120,120,240  
nhtfrq = 0,-24,-6,-6,-3  
save and exit: user_nl_cam  
setup and build the case:  
./case.setup  
./case.build  
open file: env_workflow.xml  
under case.run section set the value of JOB_WALLCLOCK_TIME = 00:25:00.  
JOB_WALLCLOCK_TIME determines how long you want to run your job. save and exit:  
env_workflow.xml  
Run the case:  
./case.submit  
The process will create a directory called E3SM/e3sm_case_1 in your scratch space.  
For me the path is: /lustre/scratch3/turquoise/sdutta/E3SM/e3sm_case_1  
Check job status: squeue -u $USER  
  
## STEP 2. Add Julia compilation and linking support to E3SM for Grizzly  
=====================================================================  
The E3SM version installed by STEP 1 has two separate systems for compilation and linking.  
1. The E3SM compilation uses the new CIME CMake system and the changes are made in E3SM/components/CMakeLists.txt  
IN line 370  
CHANGE set(INCLDIR "-I.")  
TO set(INCLDIR "${INCLDIR} -I/turquoise/users/ltang/Install/julia-1.4.0/include/julia")  
2. The E3SM top linking process uses the old Make system and the changes are made in E3SM/cime/scripts/Tools/Makefile  
IN Line 956  
CHANGE  
\$(EXEC\_SE)\: \$(OBJS)  
\$(EXEC\_SE): \$(OBJS) \$(ULIBDEP) \$(CSMSHARELIB) \$(MCTLIBS) \$(PIOLIB) \$(GPTLLIB)  
$(LD) -o $(EXEC\_SE) $(OBJS) $(CLIBS) $(ULIBS) $(SLIBS) $(MLIBS) $(F90\_LDFLAGS)  
TO (Note: /turquoise/users/ltang/Install/julia-1.4.0 is my Julia installation path, please change it to your Julia installation path)  
JULIALIBS = -L'/turquoise/users/ltang/Install/julia-1.4.0/lib' -Wl,--export-dynamic -Wl,-rpath,'/turquoise/users/ltang/Install/julia-1.4.0/lib' -Wl,-rpath,'/turquoise/users/ltang/Install/julia1.4.0/lib/julia' -ljulia  
$(EXEC_SE): $(OBJS) $(ULIBDEP) $(CSMSHARELIB) $(MCTLIBS) $(PIOLIB) $(GPTLLIB)  
$(LD) -o $(EXEC_SE) $(OBJS) $(CLIBS) $(ULIBS) $(SLIBS) $(MLIBS) $(F90_LDFLAGS) $(JULIALIBS)  
4. Change the software versions for Grizzly in cime/config/e3sm/machines/config_machines.xml, try to find the following configuration in config_machines.xml and then only make changes in the RED lines. You might also need to delete <modules><command name="load">parallel-netcdf/1.5.0</command></modules>. It works with parallel-netcdf/1.5.0  
<machine MACH="grizzly">  
<DESC>LANL Linux Cluster, 36 pes/node, batch system slurm</DESC>  
<NODENAME_REGEX>gr-fe.*.lanl.gov</NODENAME_REGEX>  
<OS>LINUX</OS>  
<COMPILERS>gnu,intel</COMPILERS>  
<MPILIBS>mvapich,openmpi</MPILIBS>  
<PROJECT>e3sm</PROJECT>  
<CIME_OUTPUT_ROOT>/lustre/scratch3/turquoise/$ENV{USER}/E3SM/scratch</CIME_OUTPUT_ROOT>  
<DIN_LOC_ROOT>/lustre/scratch3/turquoise/$ENV{USER}/E3SM/input_data</DIN_LOC_ROOT>  
<DIN_LOC_ROOT_CLMFORC>/lustre/scratch3/turquoise/$ENV{USER}/E3SM/input_data/atm/datm7</DIN_LOC_ROOT_CLMFORC>  
<DOUT_S_ROOT>/lustre/scratch3/turquoise/$ENV{USER}/E3SM/archive/$CASE</DOUT_S_ROOT>  
<BASELINE_ROOT>/lustre/scratch3/turquoise/$ENV{USER}/E3SM/input_data/ccsm_baselines/$COMPILER</BASELINE_ROOT>  
<CCSM_CPRNC>/turquoise/usr/projects/climate/SHARED_CLIMATE/software/wolf/cprnc/v0.40/cprnc</CCSM_CPRNC>  
<GMAKE_J>4</GMAKE_J>  
<TESTS>e3sm_developer</TESTS>  
<BATCH_SYSTEM>slurm</BATCH_SYSTEM>  
<SUPPORTED_BY>luke.vanroekel @ gmail.com</SUPPORTED_BY>  
<MAX_TASKS_PER_NODE>36</MAX_TASKS_PER_NODE>  
<MAX_MPITASKS_PER_NODE>32</MAX_MPITASKS_PER_NODE>  
<PROJECT_REQUIRED>TRUE</PROJECT_REQUIRED>  
<mpirun mpilib="default">  
<executable>mpirun</executable>  
<arguments>  
<arg name="num_tasks"> -n {{ total_tasks }}</arg>  
</arguments>  
</mpirun>  
<mpirun mpilib="mvapich">  
<executable>srun</executable>  
<arguments>  
<arg name="num_tasks"> -n {{ total_tasks }}</arg>  
</arguments>  
</mpirun>  
<mpirun mpilib="openmpi">  
<executable>mpirun</executable>  
<arguments>  
<arg name="num_tasks"> -n {{ total_tasks }}</arg>  
</arguments>  
</mpirun>  
<mpirun mpilib="mpi-serial">  
<executable/>  
</mpirun>  
<module_system type="module">  
<init_path lang="perl">/usr/share/Modules/init/perl.pm</init_path>  
<init_path lang="python">/usr/share/Modules/init/python.py</init_path>  
<init_path lang="sh">/etc/profile.d/z00_lmod.sh</init_path>  
<init_path lang="csh">/etc/profile.d/z00_lmod.csh</init_path>  
<cmd_path lang="perl">/usr/share/lmod/lmod/libexec/lmod perl</cmd_path>  
<cmd_path lang="python">/usr/share/lmod/lmod/libexec/lmod python</cmd_path>  
<cmd_path lang="sh">module</cmd_path>  
<cmd_path lang="csh">module</cmd_path>  
<modules>  
<command name="purge"/>  
<command name="use">/usr/projects/climate/SHARED_CLIMATE/modulefiles/all</command>  
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
 <command name="load">parallel-netcdf/1.5.0</command>  
 </modules>  
<modules>  
<command name="load">mkl</command>  
<command name="load">cmake</command>  
</modules>  
</module_system>  
<RUNDIR>/lustre/scratch3/turquoise/$ENV{USER}/E3SM/cases/$CASE/run</RUNDIR>  
<EXEROOT>/lustre/scratch3/turquoise/$ENV{USER}/E3SM/cases/$CASE/bld</EXEROOT>  
<environment_variables>  
<envname="PNETCDF_HINTS">romio_ds_write=disable;romio_ds_read=disable;romio_cb_write=enable;romio_cb_read=enable</env>  
</environment_variables>  
<environment_variables compiler="gnu">  
<env name="MKLROOT">/opt/intel/17.0/mkl</env>  
<env name="NETCDF_C_PATH">$ENV{NETCDF}</env>  
<env name="NETCDF_FORTRAN_PATH">$ENV{NETCDF}</env>  
<env name="NETCDF_PATH">$ENV{NETCDF}</env>  
<env name="NETCDFF">$ENV{NETCDF}</env>  
</environment_variables>  
</machine>  
  
## STEP 3. Run the E3SM+Julia example on Grizzly  
=====================================================================  
1. Add the following declaration in subroutine stepon_run3 of stepon.F90,  
 real(r8) :: temp_to_julia(nlev)  
 byte :: res_julia  
 byte,external :: jl_eval  
 byte,external :: jl_call_1d_double_array  
 byte,external :: jl_call_subroutine  
2. and put the following code between “#endif” and “subroutine stepon_run3”  
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
3. copy the interface.c file in the cam folder E3SM/components/cam/src/dynamics/se  
4. copy the Julia file insitu.jl to the case run folder (e.g., /lustre/scratch3/turquoise/sdutta/E3SM/e3sm_case_1/run)  
5. submit the job  
  
  
