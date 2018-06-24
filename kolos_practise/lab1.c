#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <dlfcn.h>

int main() {
    //malloc
    char *m_buffer = malloc(10 * sizeof(char));
    //calloc
    char *c_buffer = calloc(10, sizeof(char));
    //realloc
    realloc(c_buffer,5*sizeof(char));
    //measure time with times
    struct tms measured_time;
    times(&measured_time);

    //dlsym like
    void(*fun_symbol)();
    void*dl_handle;
    dl_handle=dlopen("./libPropername.so",RTLD_LAZY);
    fun_symbol=dlsym(dl_handle,"function_name");
    fun_symbol(dl_handle);  //then a call, you don't have to declare how many args it takes
    dlclose(dl_handle);






    return 0;
}