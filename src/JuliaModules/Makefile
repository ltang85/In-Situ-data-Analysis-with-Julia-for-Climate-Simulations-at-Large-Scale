#===============================================================================
#
# Common Makefile: a framework for building all CIME components and more
#
#===============================================================================

# Set up special characters
null  :=
comma := ,

# Load dependency search path.
dirs := .
dirs += $(shell cat Filepath)

cpp_dirs := $(dirs)
# Add INCROOT to path for Depends and Include
MINCROOT :=
ifdef INCROOT
  cpp_dirs += $(INCROOT)
  MINCROOT := $(INCROOT)
endif

# Expand any tildes in directory names. Change spaces to colons.
VPATH := $(foreach dir,$(cpp_dirs),$(wildcard $(dir)))
VPATH := $(subst $(space),:,$(VPATH))

RM    := rm
CP    := cp

exec_se: $(EXEC_SE)  Depends
complib: $(COMPLIB)  Depends

# Determine whether to compile threaded or not
# Set the THREADDIR for the shared build
# based on the threaded build status
ifeq ($(strip $(SMP)),TRUE)
   THREADDIR = threads
   compile_threaded = TRUE
else
   ifeq ($(strip $(SMP_PRESENT)),TRUE)
      THREADDIR = threads
      compile_threaded = TRUE
   else
      THREADDIR = nothreads
      compile_threaded = FALSE
   endif
endif

# set the debug directory based on the debug status
ifeq ($(strip $(DEBUG)),TRUE)
   DEBUGDIR = debug
else
   DEBUGDIR = nodebug
endif

ifeq ($(strip $(USE_ESMF_LIB)), TRUE)
   ESMFDIR = esmf
else
   ESMFDIR = noesmf
endif

# Determine whether any C++ code will be included in the build;
# currently, C++ code is included if and only if we're linking to the
# trilinos library or the Albany library.
USE_CXX = FALSE
ifeq ($(strip $(USE_TRILINOS)), TRUE)
   USE_CXX = TRUE
endif
ifeq ($(strip $(USE_ALBANY)), TRUE)
   USE_CXX = TRUE
endif
ifeq ($(strip $(USE_KOKKOS)), TRUE)
  USE_CXX = TRUE
endif

ifeq ($(strip $(USE_FMS)), TRUE)
  SLIBS += -lfms
endif

ifndef MOD_SUFFIX
   MOD_SUFFIX := mod
endif

#===============================================================================
# set CPP options (must use this before any flags or cflags settings)
#===============================================================================

CPPDEFS := $(USER_CPPDEFS) -D$(OS)

include $(CASEROOT)/Macros.make

# Unless DEBUG mode is enabled, use NDEBUG to turn off assert statements.
ifeq ($(strip $(DEBUG)),TRUE)
  # e3sm still has components that cannot build with -DDEBUG
  ifeq ($(CIME_MODEL),cesm)
    CPPDEFS += -DDEBUG
  endif
else
   CPPDEFS += -DNDEBUG
endif

# USE_ESMF_LIB is currently only defined in env_build.xml
ifeq ($(USE_ESMF_LIB), TRUE)
   CPPDEFS += -DUSE_ESMF_LIB
endif

ifeq ($(COMP_INTERFACE), nuopc)
   CPPDEFS += -DNUOPC_INTERFACE
else
   CPPDEFS += -DMCT_INTERFACE
endif

ifeq ($(COMPARE_TO_NUOPC), TRUE)
   CPPDEFS += -DCOMPARE_TO_NUOPC
endif

ifeq ($(strip $(MPILIB)),mpi-serial)
  CPPDEFS += -DNO_MPI2
else
  CPPDEFS += -DHAVE_MPI
endif

ifeq ($(strip $(PIO_VERSION)),1)
  CPPDEFS += -DPIO1
else
  USE_CXX = TRUE
endif

ifeq (,$(SHAREDPATH))
  SHAREDPATH = $(COMPILER)/$(MPILIB)/$(DEBUGDIR)/$(THREADDIR)/$(COMP_INTERFACE)
  INSTALL_SHAREDPATH = $(EXEROOT)/$(SHAREDPATH)
endif

ifeq ($(USE_CXX), TRUE)
  ifeq ($(SUPPORTS_CXX), FALSE)
    $(error Fatal attempt to include C++ code on a compiler/machine combo that has not been set up to support C++)
  endif
endif

# Not clear how to escape commas for libraries with their own configure
# script, and they don't need this defined anyway, so leave this out of
# FPPDEFS.
ifeq ($(HAS_F2008_CONTIGUOUS),TRUE)
  CONTIGUOUS_FLAG := -DUSE_CONTIGUOUS=contiguous,
else
  CONTIGUOUS_FLAG := -DUSE_CONTIGUOUS=
endif

ifdef CPRE
  CONTIGUOUS_FLAG := $(subst $(comma),\\$(comma),$(CONTIGUOUS_FLAG))
  CONTIGUOUS_FLAG := $(patsubst -D%,$(CPRE)%,$(CONTIGUOUS_FLAG))
endif

AR ?= ar
ARFLAGS ?= -r

ifdef NETCDF_C_PATH
  ifndef NETCDF_FORTRAN_PATH
    $(error "NETCDF_C_PATH specified without NETCDF_FORTRAN_PATH")
  endif
  NETCDF_SEPARATE:=TRUE
  ifndef INC_NETCDF_C
    INC_NETCDF_C:=$(NETCDF_C_PATH)/include
  endif
  ifndef INC_NETCDF_FORTRAN
    INC_NETCDF_FORTRAN:=$(NETCDF_FORTRAN_PATH)/include
  endif
  ifndef LIB_NETCDF_C
    LIB_NETCDF_C:=$(NETCDF_C_PATH)/lib
  endif
  ifndef LIB_NETCDF_FORTRAN
    LIB_NETCDF_FORTRAN:=$(NETCDF_FORTRAN_PATH)/lib
  endif
 else ifdef NETCDF_FORTRAN_PATH
  $(error "NETCDF_FORTRAN_PATH specified without NETCDF_C_PATH")
 else ifdef NETCDF_PATH
  NETCDF_SEPARATE:=FALSE
  ifndef INC_NETCDF
    INC_NETCDF:=$(NETCDF_PATH)/include
  endif
  ifndef LIB_NETCDF
    LIB_NETCDF:=$(NETCDF_PATH)/lib
  endif
else
  # No Netcdf is an error unless target is clean or DEP
  ifneq ($(MAKECMDGOALS), db_files)
  ifneq ($(MAKECMDGOALS), db_flags)
  ifeq (,$(findstring clean,$(MAKECMDGOALS)))
      $(error NETCDF not found: Define NETCDF_PATH or NETCDF_C_PATH and NETCDF_FORTRAN_PATH in config_machines.xml or config_compilers.xml)
  endif
  endif
  endif
endif


ifeq ($(MPILIB),mpi-serial)
  ifdef PNETCDF_PATH
    undefine PNETCDF_PATH
  endif
else
  ifdef PNETCDF_PATH
    ifndef $(INC_PNETCDF)
      INC_PNETCDF:=$(PNETCDF_PATH)/include
    endif
    ifndef LIB_PNETCDF
      LIB_PNETCDF:=$(PNETCDF_PATH)/lib
    endif
  endif
endif
# Set PETSc info if it is being used
ifeq ($(strip $(USE_PETSC)), TRUE)
  ifdef PETSC_PATH
    ifndef INC_PETSC
      INC_PETSC:=$(PETSC_PATH)/include
    endif
    ifndef LIB_PETSC
      LIB_PETSC:=$(PETSC_PATH)/lib
    endif
  else
    $(error PETSC_PATH must be defined when USE_PETSC is TRUE)
  endif

  # Get the "PETSC_LIB" list an env var
  include $(PETSC_PATH)/lib/petsc/conf/variables

endif

# Set Trilinos info if it is being used
ifeq ($(strip $(USE_TRILINOS)), TRUE)
  ifdef TRILINOS_PATH
    ifndef INC_TRILINOS
      INC_TRILINOS:=$(TRILINOS_PATH)/include
    endif
    ifndef LIB_TRILINOS
      LIB_TRILINOS:=$(TRILINOS_PATH)/lib
    endif
  else
    $(error TRILINOS_PATH must be defined when USE_TRILINOS is TRUE)
  endif

  # get a bunch of variables related to this trilinos installation;
  # these variables begin with "Trilinos_"
  include $(INC_TRILINOS)/Makefile.export.Trilinos
endif

# Set Albany info if it is being used
ifeq ($(strip $(USE_ALBANY)), TRUE)
  ifdef ALBANY_PATH
    ifndef INC_ALBANY
      INC_ALBANY:=$(ALBANY_PATH)/include
    endif
    ifndef LIB_ALBANY
      LIB_ALBANY:=$(ALBANY_PATH)/lib
    endif
  else
    $(error ALBANY_PATH must be defined when USE_ALBANY is TRUE)
  endif

  # get the "ALBANY_LINK_LIBS" list as an env var
  include $(ALBANY_PATH)/export_albany.in
endif

ifeq ($(strip $(USE_KOKKOS)), TRUE)
  include $(INSTALL_SHAREDPATH)/Makefile.kokkos
  SLIBS += $(KOKKOS_LIBS)
  CXXFLAGS += $(KOKKOS_CXXFLAGS)
  CXX_LDFLAGS += $(KOKKOS_LDFLAGS)
endif

# Set MOAB info if it is being used
ifeq ($(strip $(USE_MOAB)), TRUE)
  ifdef MOAB_PATH
    CPPDEFS += -DHAVE_MOAB
    ifndef INC_MOAB
      INC_MOAB:=$(MOAB_PATH)/include
    endif
    ifndef LIB_MOAB
      LIB_MOAB:=$(MOAB_PATH)/lib
    endif
  else
    $(error MOAB_PATH must be defined when USE_MOAB is TRUE)
  endif

  # get the "IMESH_LIBS" list as an env var
  include $(LIB_MOAB)/iMesh-Defs.inc

endif

# Set HAVE_SLASHPROC on LINUX systems which are not bluegene or Darwin (OSx)

ifeq ($(findstring -DLINUX,$(CPPDEFS)),-DLINUX)
  ifneq ($(findstring DBG,$(CPPDEFS)),DBG)
    ifneq ($(findstring Darwin,$(CPPDEFS)),Darwin)
      CPPDEFS += -DHAVE_SLASHPROC
    endif
  endif
endif

# Atleast on Titan+cray mpi, MPI_Irsends() are buggy, causing hangs during I/O
# Force PIO to use MPI_Isends instead of the default, MPI_Irsends
ifeq ($(PIO_VERSION),2)
  EXTRA_PIO_CPPDEFS = -DUSE_MPI_ISEND_FOR_FC
else
  EXTRA_PIO_CPPDEFS = -D_NO_MPI_RSEND
endif

ifdef LIB_PNETCDF
   CPPDEFS += -D_PNETCDF
   SLIBS += -L$(LIB_PNETCDF) -lpnetcdf
endif

# Set esmf.mk location with ESMF_LIBDIR having precedent over ESMFMKFILE
CIME_ESMFMKFILE := undefined_ESMFMKFILE
ifdef ESMFMKFILE
   CIME_ESMFMKFILE := $(ESMFMKFILE)
endif
ifdef ESMF_LIBDIR
   CIME_ESMFMKFILE := $(ESMF_LIBDIR)/esmf.mk
endif
# For compiling and linking with external ESMF.
# If linking to external ESMF library then include esmf.mk
# ESMF_F90COMPILEPATHS
# ESMF_F90LINKPATHS
# ESMF_F90LINKRPATHS
# ESMF_F90ESMFLINKLIBS
ifeq ($(USE_ESMF_LIB), TRUE)
  -include $(CIME_ESMFMKFILE)
  CPPDEFS += -DESMF_VERSION_MAJOR=$(ESMF_VERSION_MAJOR) -DESMF_VERSION_MINOR=$(ESMF_VERSION_MINOR)
  FFLAGS += $(ESMF_F90COMPILEPATHS)
  SLIBS  += $(ESMF_F90LINKPATHS) $(ESMF_F90LINKRPATHS) $(ESMF_F90ESMFLINKLIBS)
endif

# Stub libraries do not need to be built for nuopc driver
# so it will override these settings on the command line
ATM_PRESENT ?= TRUE
ICE_PRESENT ?= TRUE
LND_PRESENT ?= TRUE
OCN_PRESENT ?= TRUE
ROF_PRESENT ?= TRUE
GLC_PRESENT ?= TRUE
WAV_PRESENT ?= TRUE
ESP_PRESENT ?= TRUE
IAC_PRESENT ?= TRUE
ifeq ($(ULIBDEP),$(null))
  ifneq ($(LIBROOT),$(null))
    ifeq ($(ATM_PRESENT),TRUE)
      ULIBDEP += $(LIBROOT)/libatm.a
      CPPDEFS += -DATM_PRESENT
    endif
    ifeq ($(ICE_PRESENT),TRUE)
      ULIBDEP += $(LIBROOT)/libice.a
      CPPDEFS += -DICE_PRESENT
    endif
    ifeq ($(LND_PRESENT),TRUE)
      ULIBDEP += $(LNDLIBDIR)/$(LNDLIB)
      CPPDEFS += -DLND_PRESENT
    endif
    ifeq ($(OCN_PRESENT),TRUE)
      ULIBDEP += $(LIBROOT)/libocn.a
      CPPDEFS += -DOCN_PRESENT
    endif
    ifeq ($(ROF_PRESENT),TRUE)
      ULIBDEP += $(LIBROOT)/librof.a
      CPPDEFS += -DROF_PRESENT
    endif
    ifeq ($(GLC_PRESENT),TRUE)
      ULIBDEP += $(LIBROOT)/libglc.a
      CPPDEFS += -DGLC_PRESENT
    endif
    ifeq ($(WAV_PRESENT),TRUE)
      ULIBDEP += $(LIBROOT)/libwav.a
      CPPDEFS += -DWAV_PRESENT
    endif
    ifeq ($(ESP_PRESENT),TRUE)
      ULIBDEP += $(LIBROOT)/libesp.a
      CPPDEFS += -DESP_PRESENT
    endif
    ifeq ($(IAC_PRESENT),TRUE)
      ULIBDEP += $(LIBROOT)/libiac.a
    endif
  endif
endif

ifdef CPRE
  FPPDEFS := $(subst $(comma),\\$(comma),$(CPPDEFS))
  FPPDEFS := $(patsubst -D%,$(CPRE)%,$(FPPDEFS))
  EXTRA_PIO_FPPDEFS := $(subst $(comma),\\$(comma),$(EXTRA_PIO_CPPDEFS))
  EXTRA_PIO_FPPDEFS := $(patsubst -D%,$(CPRE)%,$(EXTRA_PIO_FPPDEFS))
else
  FPPDEFS := $(CPPDEFS)
  EXTRA_PIO_FPPDEFS := $(EXTRA_PIO_CPPDEFS)
endif

#===============================================================================
# Set config args for pio and mct to blank and then enable serial
#===============================================================================
ifndef CONFIG_ARGS
  CONFIG_ARGS :=
endif
ifeq  ($(findstring pio,$(MODEL)),pio)
  CONFIG_ARGS+= --enable-timing
  ifeq ($DEBUG,TRUE)
     CONFIG_ARGS+= --enable-debug
  endif
endif

#===============================================================================
# User-specified INCLDIR
#===============================================================================

INCLDIR := -I.
ifdef USER_INCLDIR
  INCLDIR += $(USER_INCLDIR)
endif

#===============================================================================
# MPI-serial library (part of MCT)
#===============================================================================

ifeq ($(strip $(MPILIB)), mpi-serial)
  CC      := $(SCC)
  FC      := $(SFC)
  CXX     := $(SCXX)
  MPIFC   := $(SFC)
  MPICC   := $(SCC)
  MPICXX  := $(SCXX)
  CONFIG_ARGS += MCT_PATH=$(SHAREDLIBROOT)/$(SHAREDPATH)/mct/mpi-serial
else
  CC  := $(MPICC)
  FC  := $(MPIFC)
  CXX := $(MPICXX)
  ifdef MPI_PATH
    INC_MPI := $(MPI_PATH)/include
    LIB_MPI := $(MPI_PATH)/lib
  endif
endif
LD := $(MPIFC)
# Decide whether to use a C++ or Fortran linker, based on whether we
# are using any C++ code and the compiler-dependent CXX_LINKER variable
ifeq ($(USE_CXX), TRUE)
  # The following is essentially an "if... elseif... else", but gmake
  # 3.80 and earlier doesn't support elseif
  ifeq ($(CXX_LINKER), CXX)
    LD := $(MPICXX)
  endif
  ifeq ($(CXX_LINKER), FORTRAN)
    LD := $(MPIFC)
  endif
endif

CSM_SHR_INCLUDE:=$(INSTALL_SHAREDPATH)/$(COMP_INTERFACE)/$(ESMFDIR)/$(NINST_VALUE)/include
# This is needed so that dependancies are found
VPATH+=$(CSM_SHR_INCLUDE)

#===============================================================================
# Set include paths (needed after override for any model specific builds below)
#===============================================================================
INCLDIR += -I$(INSTALL_SHAREDPATH)/include -I$(INSTALL_SHAREDPATH)/$(COMP_INTERFACE)/$(ESMFDIR)/$(NINST_VALUE)/include

ifeq ($(NETCDF_SEPARATE), FALSE)
  INCLDIR += -I$(INC_NETCDF)
else ifeq ($(NETCDF_SEPARATE), TRUE)
  INCLDIR += -I$(INC_NETCDF_C) -I$(INC_NETCDF_FORTRAN)
endif
ifdef MOD_NETCDF
  INCLDIR += -I$(MOD_NETCDF)
endif
ifdef INC_MPI
  INCLDIR += -I$(INC_MPI)
endif
ifdef INC_PNETCDF
  INCLDIR += -I$(INC_PNETCDF)
endif
ifdef INC_PETSC
  INCLDIR += -I$(INC_PETSC)
endif
ifdef INC_TRILINOS
  INCLDIR += -I$(INC_TRILINOS)
endif
ifdef INC_ALBANY
  INCLDIR += -I$(INC_ALBANY)
endif
ifdef INC_MOAB
  INCLDIR += -I$(INC_MOAB)
endif

ifeq ($(MODEL),driver)
  INCLDIR += -I$(EXEROOT)/atm/obj -I$(EXEROOT)/ice/obj -I$(EXEROOT)/ocn/obj -I$(EXEROOT)/glc/obj -I$(EXEROOT)/rof/obj -I$(EXEROOT)/wav/obj -I$(EXEROOT)/esp/obj -I$(EXEROOT)/iac/obj
# nagfor and gcc have incompatible LDFLAGS.
# nagfor requires the weird "-Wl,-Wl,," syntax.
# If done in config_compilers.xml, we break MCT.
  ifeq ($(strip $(COMPILER)),nag)
     ifeq ($(NETCDF_SEPARATE), false)
       SLIBS += -Wl,-Wl,,-rpath=$(LIB_NETCDF)
     else ifeq ($(NETCDF_SEPARATE), true)
       SLIBS += -Wl,-Wl,,-rpath=$(LIB_NETCDF_C)
       SLIBS += -Wl,-Wl,,-rpath=$(LIB_NETCDF_FORTRAN)
     endif
  endif
else
  ifeq ($(strip $(COMPILER)),nag)
    ifeq ($(DEBUG), TRUE)
      ifneq (,$(filter $(strip $(MACH)),hobart izumi))
       # GCC needs to be able to link to
       # nagfor runtime to get autoconf
       # tests to work.
         CFLAGS += -Wl,--as-needed,--allow-shlib-undefined
         SLIBS += -L$(COMPILER_PATH)/lib/NAG_Fortran -lf62rts
       endif
    endif
  endif
endif

ifndef MCT_LIBDIR
  MCT_LIBDIR=$(INSTALL_SHAREDPATH)/lib
endif

ifdef PIO_LIBDIR
  ifeq ($(PIO_VERSION),$(PIO_VERSION_MAJOR))
    INCLDIR += -I$(PIO_INCDIR)
    SLIBS += -L$(PIO_LIBDIR)
  else
    # If PIO_VERSION_MAJOR doesnt match, build from source
    unexport PIO_LIBDIR
  endif
endif
PIO_LIBDIR ?= $(INSTALL_SHAREDPATH)/lib

ifndef GPTL_LIBDIR
  GPTL_LIBDIR=$(INSTALL_SHAREDPATH)/lib
endif

ifndef GLC_DIR
  GLC_DIR=$(EXEROOT)/glc
endif
ifndef CISM_LIBDIR
  CISM_LIBDIR=$(GLC_DIR)/lib
endif
ifndef GLCROOT
  # Backwards compatibility
  GLCROOT=$(CIMEROOT)/../components/cism
endif

INCLDIR +=	-I$(INSTALL_SHAREDPATH)/include

#
# Use the MCT dir for the cache for all configure calls because it is the first one
#
CFLAGS+=$(CPPDEFS)
CXXFLAGS+=$(CPPDEFS)
CONFIG_ARGS +=  CC="$(CC)" FC="$(FC)" MPICC="$(MPICC)" \
                MPIFC="$(MPIFC)" FCFLAGS="$(FFLAGS) $(FREEFLAGS) $(INCLDIR)" \
                CPPDEFS="$(CPPDEFS)" CFLAGS="$(CFLAGS) -I.. $(INCLDIR)" \
                LDFLAGS="$(LDFLAGS)"

ifeq ($(NETCDF_SEPARATE), FALSE)
  CONFIG_ARGS += NETCDF_PATH=$(NETCDF_PATH)
else ifeq ($(NETCDF_SEPARATE), TRUE)
  # The mct library needs the NetCDF_C library
  CONFIG_ARGS += NETCDF_PATH=$(NETCDF_C_PATH)
endif

FFLAGS += $(FPPDEFS)
FFLAGS_NOOPT += $(FPPDEFS)

ifeq ($(findstring -cosp,$(CAM_CONFIG_OPTS)),-cosp)
  # The following is for the COSP simulator code:
  COSP_LIBDIR:=$(abspath $(EXEROOT)/atm/obj/cosp)
  ifeq ($(MODEL),driver)
    INCLDIR+=-I$(COSP_LIBDIR)
  endif
endif

ifeq ($(MODEL),cam)
  # These RRTMG files take an extraordinarily long time to compile with optimization.
  # Until mods are made to read the data from files, just remove optimization from
  # their compilation.
rrtmg_lw_k_g.o: rrtmg_lw_k_g.f90
	$(FC) -c $(FPPFLAGS) $(INCLDIR) $(INCS) $(FREEFLAGS) $(FFLAGS) $(FFLAGS_NOOPT) $<

rrtmg_sw_k_g.o: rrtmg_sw_k_g.f90
	$(FC) -c $(FPPFLAGS) $(INCLDIR) $(INCS) $(FREEFLAGS) $(FFLAGS) $(FFLAGS_NOOPT) $<


ifdef COSP_LIBDIR
INCLDIR+=-I$(COSP_LIBDIR) -I$(COSP_LIBDIR)/../ -I../$(INSTALL_SHAREDPATH)/include -I../$(CSM_SHR_INCLUDE)
$(COSP_LIBDIR)/libcosp.a: cam_abortutils.o
	$(MAKE) -C $(COSP_LIBDIR) F90='$(FC)' F90FLAGS='$(INCLDIR) $(INCS) $(FREEFLAGS) $(FFLAGS) $(FC_AUTO_R8)' \
	F90FLAGS_noauto='$(INCLDIR) $(INCS) $(FREEFLAGS) $(FFLAGS)' \
	F90FLAGS_fixed='$(INCLDIR) $(INCS) $(FIXEDFLAGS) $(FFLAGS) $(FC_AUTO_R8)'

cospsimulator_intr.o: $(COSP_LIBDIR)/libcosp.a
endif

endif


# System libraries (netcdf, mpi, pnetcdf, esmf, trilinos, etc.)
ifndef SLIBS
  ifeq ($(NETCDF_SEPARATE), FALSE)
    SLIBS := -L$(LIB_NETCDF) -lnetcdff -lnetcdf
  else ifeq ($(NETCDF_SEPARATE), TRUE)
    SLIBS := -L$(LIB_NETCDF_FORTRAN) -L$(LIB_NETCDF_C) -lnetcdff -lnetcdf
  endif
endif

ifdef LAPACK_LIBDIR
   SLIBS += -L$(LAPACK_LIBDIR) -llapack -lblas
endif
ifdef LIB_MPI
   ifndef MPI_LIB_NAME
      SLIBS += -L$(LIB_MPI) -lmpi
   else
      SLIBS += -L$(LIB_MPI) -l$(MPI_LIB_NAME)
   endif
endif

# For compiling and linking with external ESMF.
# If linking to external ESMF library then include esmf.mk
# ESMF_F90COMPILEPATHS
# ESMF_F90LINKPATHS
# ESMF_F90LINKRPATHS
# ESMF_F90ESMFLINKLIBS
ifeq ($(USE_ESMF_LIB), TRUE)
  -include $(CCSM_ESMFMKFILE)
  FFLAGS += $(ESMF_F90COMPILEPATHS)
  SLIBS  += $(ESMF_F90LINKPATHS) $(ESMF_F90LINKRPATHS) $(ESMF_F90ESMFLINKLIBS)
endif

# Add PETSc libraries
ifeq ($(strip $(USE_PETSC)), TRUE)
  SLIBS += ${PETSC_LIB}
endif

# Add trilinos libraries; too be safe, we include all libraries included in the trilinos build,
# as well as all necessary third-party libraries
ifeq ($(strip $(USE_TRILINOS)), TRUE)
  SLIBS += -L$(LIB_TRILINOS) $(Trilinos_LIBRARIES) $(Trilinos_TPL_LIBRARY_DIRS) $(Trilinos_TPL_LIBRARIES)
endif

# Add Albany libraries.  These are defined in the ALBANY_LINK_LIBS env var that was included above
ifeq ($(strip $(USE_ALBANY)), TRUE)
  SLIBS += $(ALBANY_LINK_LIBS)
endif

# Add MOAB libraries.  These are defined in the MOAB_LINK_LIBS env var that was included above
ifeq ($(strip $(USE_MOAB)), TRUE)
  SLIBS += $(IMESH_LIBS)
endif

# Add libraries and flags that we need on the link line when C++ code is included
# We need to do these additions after CONFIG_ARGS is set, because they can sometimes break configure for mct, etc.,
# if they are added to LDFLAGS in CONFIG_ARGS.
ifeq ($(USE_CXX), TRUE)
  ifdef CXX_LIBS
    SLIBS += $(CXX_LIBS)
  endif

  ifdef CXX_LDFLAGS
    LDFLAGS += $(CXX_LDFLAGS)
  endif
endif

# Remove arch flag if it exists
F90_LDFLAGS := $(filter-out -arch%,$(LDFLAGS))

# Machine stuff to appear last on the link step
ifndef MLIBS
     MLIBS  :=
endif

#------------------------------------------------------------------------------
# Drive configure scripts for support libraries (mct)
#------------------------------------------------------------------------------

$(SHAREDLIBROOT)/$(SHAREDPATH)/mct/Makefile.conf:
	@echo "SHAREDLIBROOT |$(SHAREDLIBROOT)| SHAREDPATH |$(SHAREDPATH)|"; \
	$(CONFIG_SHELL) $(CIMEROOT)/src/externals/mct/configure $(CONFIG_ARGS) --srcdir $(CIMEROOT)/src/externals/mct

$(SHAREDLIBROOT)/$(SHAREDPATH)/mct/mpi-serial/Makefile.conf:
	@echo "SHAREDLIBROOT |$(SHAREDLIBROOT)| SHAREDPATH |$(SHAREDPATH)|"; \
	$(CONFIG_SHELL) $(CIMEROOT)/src/externals/mct/mpi-serial/configure $(CONFIG_ARGS) --srcdir $(CIMEROOT)/src/externals/mct/mpi-serial

ifndef IO_LIB_SRCROOT
  ifndef PIO_SRCROOT
    PIO_SRCROOT = $(CIMEROOT)/src/externals
  endif

  ifeq ($(PIO_VERSION),2)
    PIO_SRC_DIR = $(PIO_SRCROOT)/pio2
  else
    ifneq ("$(wildcard $(PIO_SRCROOT)/pio1/pio)", "")
      PIO_SRC_DIR = $(PIO_SRCROOT)/pio1
    else
      PIO_SRC_DIR = $(PIO_SRCROOT)/pio1/pio
    endif
  endif
else
  PIO_SRC_DIR = $(IO_LIB_SRCROOT)/$(IO_LIB_v$(PIO_VERSION)_SRCDIR)
endif


ifeq ($(PIO_VERSION),2)
# This is a pio2 library
  PIOLIB = $(PIO_LIBDIR)/libpiof.a $(PIO_LIBDIR)/libpioc.a
  PIOLIBNAME = -lpiof -lpioc
else
# This is a pio1 library
  PIOLIB = $(PIO_LIBDIR)/libpio.a
  PIOLIBNAME = -lpio
endif
#endif

MCTLIBS = $(MCT_LIBDIR)/libmct.a $(MCT_LIBDIR)/libmpeu.a

GPTLLIB = $(GPTL_LIBDIR)/libgptl.a

ULIBS += -L$(INSTALL_SHAREDPATH)/$(COMP_INTERFACE)/$(ESMFDIR)/$(NINST_VALUE)/lib -lcsm_share -L$(INSTALL_SHAREDPATH)/lib $(PIOLIBNAME) -lgptl -lmct -lmpeu

#------------------------------------------------------------------------------
# Drive cmake script for cism and pio
#------------------------------------------------------------------------------

ifndef CMAKE_OPTS
  CMAKE_OPTS :=
endif
# note that the fortran flags include neither the FREEFLAGS nor the
# FIXEDFLAGS, so that both free & fixed code can be built (cmake
# doesn't seem to be able to differentiate between free & fixed
# fortran flags)
CMAKE_OPTS += -D CMAKE_Fortran_FLAGS:STRING="$(FFLAGS) $(EXTRA_PIO_FPPDEFS) $(INCLDIR)" \
              -D CMAKE_C_FLAGS:STRING="$(CFLAGS) $(EXTRA_PIO_CPPDEFS) $(INCLDIR)" \
              -D CMAKE_CXX_FLAGS:STRING="$(CXXFLAGS) $(EXTRA_PIO_CPPDEFS) $(INCLDIR)" \
              -D CMAKE_VERBOSE_MAKEFILE:BOOL=ON \
              -D GPTL_PATH:STRING=$(INSTALL_SHAREDPATH) \
              -D PIO_ENABLE_TESTS:BOOL=OFF \
              -D PIO_USE_MALLOC:BOOL=ON \
	      -D USER_CMAKE_MODULE_PATH:LIST="$(CIMEROOT)/src/CMake;$(CIMEROOT)/src/externals/pio2/cmake" \

# Allow for separate installations of the NetCDF C and Fortran libraries
ifeq ($(NETCDF_SEPARATE), FALSE)
  CMAKE_OPTS += -D NetCDF_PATH:PATH=$(NETCDF_PATH)
else ifeq ($(NETCDF_SEPARATE), TRUE)
  # NETCDF_Fortran_DIR points to the separate
  # installation of Fortran NetCDF for PIO
  CMAKE_OPTS += -D NetCDF_C_PATH:PATH=$(NETCDF_C_PATH) \
                -D NetCDF_Fortran_PATH:PATH=$(NETCDF_FORTRAN_PATH)
endif

ifdef HDF5_PATH
        CMAKE_OPTS += -D HDF5_PATH:STRING="$(HDF5_PATH)"
endif

ifdef PNETCDF_PATH
	CMAKE_OPTS += -D PnetCDF_PATH:STRING="$(PNETCDF_PATH)"
else
        CMAKE_OPTS += -D WITH_PNETCDF:LOGICAL=FALSE -D PIO_USE_MPIIO:LOGICAL=FALSE
endif
ifdef PIO_FILESYSTEM_HINTS
	CMAKE_OPTS += -D PIO_FILESYSTEM_HINTS:STRING="$(PIO_FILESYSTEM_HINTS)"
endif
ifeq ($(MPILIB),mpi-serial)
	CMAKE_OPTS += -D PIO_USE_MPISERIAL=TRUE -D MPISERIAL_PATH=$(INSTALL_SHAREDPATH)
endif

# This captures the many cism-specific options to cmake
CMAKE_OPTS += $(USER_CMAKE_OPTS)

# CMake doesn't seem to like it when you define compilers via -D
# CMAKE_C_COMPILER, etc., when you rerun cmake with an existing
# cache. So doing this via environment variables instead.
ifndef CMAKE_ENV_VARS
  CMAKE_ENV_VARS :=
endif
CMAKE_ENV_VARS += CC=$(CC) \
                  CXX=$(CXX) \
                  FC=$(FC) \
                  LDFLAGS="$(LDFLAGS)"


# We declare $(GLC_DIR)/Makefile to be a phony target so that cmake is
# always rerun whenever invoking 'make $(GLC_DIR)/Makefile'; this is
# desirable to pick up any new source files that may have been added
.PHONY: $(GLC_DIR)/Makefile
$(GLC_DIR)/Makefile:
	cd $(GLC_DIR); \
	$(CMAKE_ENV_VARS) cmake $(CMAKE_OPTS) $(GLCROOT)/source_cism

$(PIO_LIBDIR)/Makefile:
	cd $(PIO_LIBDIR); \
	$(CMAKE_ENV_VARS) cmake $(CMAKE_OPTS) $(PIO_SRC_DIR)

#-------------------------------------------------------------------------------
# Build & include dependency files
#-------------------------------------------------------------------------------

touch_filepath:
	touch Filepath

# Get list of files and build dependency file for all .o files
#   using perl scripts mkSrcfiles and mkDepends
# if a source is of form .F90.in strip the .in before creating the list of objects
SOURCES := $(shell cat Srcfiles)
BASENAMES := $(basename $(basename $(SOURCES)))
OBJS    := $(addsuffix .o, $(BASENAMES))
INCS    := $(foreach dir,$(cpp_dirs),-I$(dir))

CURDIR := $(shell pwd)

Depends: Srcfiles Deppath
	$(CASETOOLS)/mkDepends $(USER_MKDEPENDS_OPTS) Deppath Srcfiles > $@

Deppath: Filepath
	$(CP) -f Filepath $@
	@echo "$(MINCROOT)" >> $@

Srcfiles: Filepath
	$(CASETOOLS)/mkSrcfiles

Filepath:
	@echo "$(VPATH)" > $@


#-------------------------------------------------------------------------------
# echo file names, paths, compile flags, etc. used during build
#-------------------------------------------------------------------------------

db_files:
	@echo " "
	@echo "* VPATH   := $(VPATH)"
	@echo "* INCS    := $(INCS)"
	@echo "* OBJS    := $(OBJS)"
db_flags:
	@echo " "
	@echo "* cc      := $(CC)  $(CFLAGS) $(INCS) $(INCLDIR)"
	@echo "* .F.o    := $(FC)  $(FFLAGS) $(FIXEDFLAGS) $(INCS) $(INCLDIR)"
	@echo "* .F90.o  := $(FC)  $(FFLAGS) $(FREEFLAGS) $(INCS) $(INCLDIR)"
	ifeq ($(USE_CXX), TRUE)
	  @echo "* .cpp.o  := $(CXX) $(CXXFLAGS) $(INCS) $(INCLDIR)"
	endif

#-------------------------------------------------------------------------------
# Rules used for the tests run by "configure -test"
#-------------------------------------------------------------------------------

test_fc: test_fc.o
	$(LD) -o $@ test_fc.o $(F90_LDFLAGS)
ifeq ($(NETCDF_SEPARATE), FALSE)
test_nc: test_nc.o
	$(LD) -o $@ test_nc.o -L$(LIB_NETCDF) -lnetcdff -lnetcdf $(F90_LDFLAGS)
else ifeq ($(NETCDF_SEPARATE), TRUE)
test_nc: test_nc.o
	$(LD) -o $@ test_nc.o -L$(LIB_NETCDF_FORTRAN) -L$(LIB_NETCDF_C) -lnetcdff -lnetcdf $(F90_LDFLAGS)
endif
test_mpi: test_mpi.o
	$(LD) -o $@ test_mpi.o $(F90_LDFLAGS)
test_esmf: test_esmf.o
	$(LD) -o $@ test_esmf.o $(F90_LDFLAGS)

#-------------------------------------------------------------------------------
# create list of component libraries - hard-wired for current ccsm components
#-------------------------------------------------------------------------------
ifeq ($(CIME_MODEL),cesm)
  ifeq ($(COMP_LND),clm)
    USE_SHARED_CLM=TRUE
  else
    USE_SHARED_CLM=FALSE
  endif
else
  USE_SHARED_CLM=FALSE
endif
ifeq ($(USE_SHARED_CLM),FALSE)
  LNDOBJDIR = $(EXEROOT)/lnd/obj
  LNDLIBDIR=$(LIBROOT)
  ifeq ($(COMP_LND),clm)
    LNDLIB := libclm.a
  else
    LNDLIB := liblnd.a
  endif
  INCLDIR += -I$(LNDOBJDIR)
else
  LNDLIB := libclm.a
  LNDOBJDIR = $(SHAREDLIBROOT)/$(SHAREDPATH)/$(COMP_INTERFACE)/$(ESMFDIR)/clm/obj
  LNDLIBDIR = $(EXEROOT)/$(SHAREDPATH)/$(COMP_INTERFACE)/$(ESMFDIR)/lib
  INCLDIR += -I$(INSTALL_SHAREDPATH)/$(COMP_INTERFACE)/$(ESMFDIR)/include
  ifeq ($(MODEL),clm)
    INCLUDE_DIR = $(INSTALL_SHAREDPATH)/$(COMP_INTERFACE)/$(ESMFDIR)/include
  endif
endif
ifeq ($(LND_PRESENT),TRUE)
  INCLDIR += -I$(LNDOBJDIR)
endif
ifeq ($(COMP_GLC), cism)
  ULIBDEP += $(CISM_LIBDIR)/libglimmercismfortran.a
  ifeq ($(CISM_USE_TRILINOS), TRUE)
    ULIBDEP += $(CISM_LIBDIR)/libglimmercismcpp.a
  endif
endif
ifeq ($(OCN_SUBMODEL),moby)
  ULIBDEP += $(LIBROOT)/libmoby.a
endif

ifdef COSP_LIBDIR
  ifeq ($(CIME_MODEL),cesm)
    ULIBDEP += $(COSP_LIBDIR)/libcosp.a
  endif
endif


ifndef CLIBS
   ifdef ULIBDEP
     # For each occurrence of something like /path/to/foo/libbar.a in ULIBDEP,
     # CLIBS will contain -L/path/to/foo -lbar
     CLIBS := $(foreach LIBDEP,$(strip $(ULIBDEP)), -L$(dir $(LIBDEP)) $(patsubst lib%.a,-l%,$(notdir $(LIBDEP))))
   endif
endif

# libcsm_share.a is in ULIBDEP, but -lcsm_share is in ULIBS rather than CLIBS,
# so this needs to be added after creating CLIBS above
CSMSHARELIB = $(INSTALL_SHAREDPATH)/$(COMP_INTERFACE)/$(ESMFDIR)/$(NINST_VALUE)/lib/libcsm_share.a
ULIBDEP += $(CSMSHARELIB)

#-------------------------------------------------------------------------------
# build rules:
#-------------------------------------------------------------------------------

.SUFFIXES:
.SUFFIXES: .F90 .F .f90 .f .c .cpp .o .in

ifeq ($(MPILIB),mpi-serial)
  MPISERIAL = $(INSTALL_SHAREDPATH)/lib/libmpi-serial.a
  MLIBS += $(MPISERIAL)
  CMAKE_OPTS += -DMPI_C_INCLUDE_PATH=$(INSTALL_SHAREDPATH)/include \
      -DMPI_Fortran_INCLUDE_PATH=$(INSTALL_SHAREDPATH)/include \
      -DMPI_C_LIBRARIES=$(INSTALL_SHAREDPATH)/lib/libmpi-serial.a \
      -DMPI_Fortran_LIBRARIES=$(INSTALL_SHAREDPATH)/lib/libmpi-serial.a
endif

$(MCTLIBS)  : $(MPISERIAL)

$(PIOLIB) : $(MPISERIAL) $(GPTLLIB)

$(CSMSHARELIB):  $(MCTLIBS) $(PIOLIB) $(GPTLLIB)

ifneq ($(MODEL),csm_share)
  $(OBJS):  $(CSMSHARELIB)
else
  complib: install_lib
endif

install_lib: $(COMPLIB)
	$(CP) -p $(COMPLIB) $(CSMSHARELIB)
	$(CP) -p *.$(MOD_SUFFIX) *.h $(INCLUDE_DIR)

JULIALIBS = -L'/turquoise/usr/projects/w20_insituinf/ClimateSimulations/InSitu/Install/julia-1.5.4/lib' -Wl,--export-dynamic -Wl,-rpath,'/turquoise/usr/projects/w20_insituinf/ClimateSimulations/InSitu/Install/julia-1.5.4/lib' -Wl,-rpath,'/turquoise/usr/projects/w20_insituinf/ClimateSimulations/InSitu/Install/julia-1.5.4/lib/julia' -ljulia

$(EXEC_SE): $(OBJS) $(ULIBDEP) $(CSMSHARELIB) $(MCTLIBS) $(PIOLIB) $(GPTLLIB)
	$(LD) -o $(EXEC_SE) $(OBJS) $(CLIBS) $(ULIBS) $(SLIBS) $(MLIBS) $(F90_LDFLAGS) $(JULIALIBS)

$(COMPLIB): $(OBJS)
	$(AR) $(ARFLAGS) $(COMPLIB) $(OBJS)

.c.o:
	$(CC) -c $(INCLDIR) $(INCS) $(CFLAGS)  $<
.F.o:
	$(FC) -c $(INCLDIR) $(INCS) $(FFLAGS) $(FIXEDFLAGS) $<
.f.o:
	$(FC) -c $(INCLDIR) $(INCS) $(FFLAGS) $(FIXEDFLAGS) $<
.f90.o:
	$(FC) -c $(INCLDIR) $(INCS) $(FFLAGS) $(FREEFLAGS)  $<
.F90.o:
	$(FC) -c $(INCLDIR) $(INCS) $(FFLAGS) $(FREEFLAGS) $(CONTIGUOUS_FLAG) $<
.cpp.o:
	$(CXX) -c $(INCLDIR) $(INCS) $(CXXFLAGS)  $<

%.F90: %.F90.in
	$(CIMEROOT)/src/externals/genf90/genf90.pl $< > $@

clean_dependsatm:
	$(RM) -f $(EXEROOT)/atm/obj/Srcfiles

clean_dependscpl:
	$(RM) -f $(EXEROOT)/cpl/obj/Srcfiles

clean_dependsocn:
	$(RM) -f $(EXEROOT)/ocn/obj/Srcfiles

clean_dependswav:
	$(RM) -f $(EXEROOT)/wav/obj/Srcfiles

clean_dependsiac:
	$(RM) -f $(EXEROOT)/iac/obj/Srcfiles

clean_dependsglc:
	$(RM) -f $(EXEROOT)/glc/obj/Srcfiles

clean_dependsice:
	$(RM) -f $(EXEROOT)/ice/obj/Srcfiles

clean_dependsrof:
	$(RM) -f $(EXEROOT)/rof/obj/Srcfiles

clean_dependsesp:
	$(RM) -f $(EXEROOT)/esp/obj/Srcfiles

clean_dependslnd:
	$(RM) -f $(LNDOBJDIR)/Srcfiles

clean_dependscsmshare:
	$(RM) -f $(SHAREDLIBROOT)/$(SHAREDPATH)/$(COMP_INTERFACE)/$(ESMFDIR)/$(NINST_VALUE)/csm_share/Srcfiles

clean_depends: clean_dependsatm clean_dependscpl clean_dependswav clean_dependsglc clean_dependsice clean_dependsrof clean_dependslnd clean_dependscsmshare clean_dependsesp clean_dependsiac


cleanatm:
	$(RM) -f $(LIBROOT)/libatm.a
	$(RM) -fr $(EXEROOT)/atm/obj

cleancpl:
	$(RM) -fr $(EXEROOT)/cpl/obj

cleanocn:
	$(RM) -f $(LIBROOT)/libocn.a
	$(RM) -fr $(EXEROOT)/ocn/obj

cleanwav:
	$(RM) -f $(LIBROOT)/libwav.a
	$(RM) -fr $(EXEROOT)/wav/obj

cleaniac:
	$(RM) -f $(LIBROOT)/libiac.a
	$(RM) -fr $(EXEROOT)/iac/obj

cleanesp:
	$(RM) -f $(LIBROOT)/libesp.a
	$(RM) -fr $(EXEROOT)/esp/obj

cleanglc:
	$(RM) -f $(LIBROOT)/libglc.a
	$(RM) -fr $(EXEROOT)/glc

cleanice:
	$(RM) -f $(LIBROOT)/libice.a
	$(RM) -fr $(EXEROOT)/ice/obj

cleanrof:
	$(RM) -f $(LIBROOT)/librof.a
	$(RM) -fr $(EXEROOT)/rof/obj

cleanlnd:
	$(RM) -f $(LIBROOT)/$(LNDLIB)
	$(RM) -fr $(EXEROOT)/lnd/obj

cleancsmshare:
	$(RM) -f $(CSMSHARELIB)
	$(RM) -fr $(SHAREDLIBROOT)/$(SHAREDPATH)/$(COMP_INTERFACE)/$(ESMFDIR)/$(NINST_VALUE)/csm_share

cleanpio:
	$(RM) -f $(PIO_LIBDIR)/libpio*
	$(RM) -fr $(SHAREDLIBROOT)/$(SHAREDPATH)/pio

cleanmct:
	$(RM) -f $(MCTLIBS)
	$(RM) -fr $(SHAREDLIBROOT)/$(SHAREDPATH)/mct

cleangptl:
	$(RM) -f $(GPTLLIB)
	$(RM) -fr $(SHAREDLIBROOT)/$(SHAREDPATH)/gptl

clean: cleanatm cleanocn cleanwav cleanglc cleanice cleanrof cleanlnd cleanesp cleaniac

realclean: clean cleancsmshare cleanpio cleanmct cleangptl

# the if-tests prevent DEPS files from being created when they're not needed
ifneq ($(MAKECMDGOALS), db_files)
ifneq ($(MAKECMDGOALS), db_flags)
ifeq (,$(findstring clean,$(MAKECMDGOALS)))
    -include Depends $(CASEROOT)/Depends.$(COMPILER) $(CASEROOT)/Depends.$(MACH) $(CASEROOT)/Depends.$(MACH).$(COMPILER)
endif
endif
endif
ifeq ($(MODEL),csm_share)
  shr_assert_mod.mod: shr_assert_mod.o
endif
