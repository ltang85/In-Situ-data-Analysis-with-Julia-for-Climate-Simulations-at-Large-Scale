#include<stdio.h>
#include<stdint.h>
#include<julia.h>
#include "mpi.h"

void jl_init_(){
  jl_init__threading();
}

char* nullterminated(char* input, int strlen){
  char *str = malloc(strlen+1);
  strncpy(str, input, strlen);
  str[strlen] = 0;
  return str;
}

uint8_t jl_call_2d_double_array_(char* fstr, double *arr, int *rows, int *cols, int strlen){
  char *str = nullterminated(fstr, strlen);
  jl_function_t *f = jl_get_function(jl_main_module, str);
  if(f){
    jl_value_t* array_type = jl_apply_array_type((jl_value_t*)jl_float64_type, 2);
    jl_value_t *types[] = {(jl_value_t*)jl_long_type, (jl_value_t*)jl_long_type};
    jl_tupletype_t *tt = jl_apply_tuple_type_v(types, 2);
    typedef struct {
      ssize_t r;
      ssize_t c;
    } size2d;
    size2d *s = (size2d*)jl_new_struct_uninit(tt);
    JL_GC_PUSH1(&s);
    s->r = *rows;
    s->c = *cols;
    jl_array_t *x = jl_ptr_to_array(array_type, arr, (jl_value_t*)s, 0);
    JL_GC_POP();
    jl_call1(f, (jl_value_t*)x);
    jl_value_t* res = jl_exception_occurred();
    if(res){
      fprintf(stderr, "julia evaluation of %s failed with exception %s\n", str, jl_typeof_str(res));
      free(str);
      return 1;
    }
  } else {
    fprintf(stderr, "couldn't find julia function %s\n", str);
    free(str);
    return 1;
  }
  free(str);
  return 0;
}

uint8_t jl_call_1d_double_array_(char* fstr, double *arr, int *arrlen, int *nstep, int strlen){
  char *str = nullterminated(fstr, strlen);
  printf("INTERFACE=== arrlen %d nstep %d str %s\n",*arrlen,*nstep,str);
  jl_function_t *f = jl_get_function(jl_main_module, str);
  if(f){
    jl_value_t* array_type = jl_apply_array_type((jl_value_t*)jl_float64_type, 1);
    jl_array_t *x = jl_ptr_to_array_1d(array_type, arr, *arrlen, 0);
    jl_call1(f, (jl_value_t*)x);
    jl_value_t* res = jl_exception_occurred();
    if(res){
      fprintf(stderr, "julia evaluation of %s failed with exception %s\n", str, jl_typeof_str(res));
      free(str);
      return 1;
    }
  } else {
    fprintf(stderr, "couldn't find julia function %s\n", str);
    free(str);
    return 1;
  }
  free(str);
  return 0;
}

uint8_t jl_call_subroutine_(char* fstr, double *arr, int* arrlen, int* iam, int* nstep, MPI_Fint *Fcomm, int strlen){
  char *str = nullterminated(fstr, strlen);
  jl_value_t *res;

//  printf("FROM INTERFACE Name: %s temp: %f %f %f %f len: %d iam: %d nstep: %d",str,arr[0],arr[1],arr[2],arr[3],*arrlen,*iam,*nstep);

  if(*nstep==0) {
    res = jl_eval_string("Base.include(Main, \"insitu.jl\")");
    if (!res) printf("Base.include failed\n");
    res = jl_eval_string("using Main.mymodule");
    if (!res) printf("using Main.mymodule failed\n");
  }
  jl_module_t *mod_name = (jl_module_t*)jl_eval_string("Main.mymodule");
  jl_function_t *f = jl_get_function(mod_name, str);
  if(f){
    jl_value_t *array_type = jl_apply_array_type((jl_value_t*)jl_float64_type, 1);
    jl_array_t *x = jl_ptr_to_array_1d(array_type, arr, *arrlen, 0);
    jl_value_t *jl_iam = jl_box_int32(*iam);
    jl_value_t *jl_nstep = jl_box_int32(*nstep);
    jl_value_t *jl_comm = jl_box_int32(*Fcomm);
    jl_call3(f, (jl_value_t*)x, jl_nstep, jl_comm);
    res = jl_exception_occurred();
    if(res){
      fprintf(stderr, "julia evaluation of %s failed with exception %s\n", str, jl_typeof_str(res));
      free(str);
      return 1;
    }
  } else {
    fprintf(stderr, "couldn't find subroutine %s\n", str);
    free(str);
    return 1;
  }
  free(str);
  return 0;
}

uint8_t jl_call_ssw_(char* fstr, double *arr, int *arrlen, int *iam, int *nstep, int strlen, MPI_Fint *Fcomm){
  char *str = nullterminated(fstr, strlen);
  if(*nstep==0) {
    jl_eval_string("Base.include(Main, \"insitu.jl\")");
    jl_eval_string("using Main.mymodule");
  }
  jl_module_t *mod_name = (jl_module_t*)jl_eval_string("Main.mymodule");
  jl_function_t *f = jl_get_function(mod_name, "computeSSW");
  if(f)
  {
    jl_value_t* array_type = jl_apply_array_type((jl_value_t*)jl_float64_type, 1);
    jl_array_t *data = jl_ptr_to_array_1d(array_type, arr, *arrlen, 0);
    jl_value_t *jl_iam = jl_box_int32(*iam);
    jl_value_t *jl_nstep = jl_box_int32(*nstep);
    jl_value_t *jl_comm = jl_box_int32(*Fcomm);
    jl_call3(f, (jl_value_t*)data, jl_comm, jl_nstep);

    jl_value_t* res = jl_exception_occurred();
    if(res){
      fprintf(stderr, "julia evaluation of %s failed with exception %s\n", str, jl_typeof_str(res));
      free(str);
      return 1;
    }
  } else {
    fprintf(stderr, "couldn't find subroutine %s\n", str);
    free(str);
    return 1;
  }
  free(str);
  return 0;
}

uint8_t jl_eval_(char* fstr, int strlen){
  char *str = nullterminated(fstr, strlen);
  jl_eval_string(str);
  jl_value_t* res = jl_exception_occurred();
  if(res){
    fprintf(stderr, "julia evaluation of %s failed with exception %s\n", str, jl_typeof_str(res));
    return 1;
  }
  free(str);
  return 0;
}

void jl_exit_(){
  jl_atexit_hook(0);
}

