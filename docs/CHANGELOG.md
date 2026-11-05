27/10/26
Se ha creado la estructura básica del proyecto, con un `.gitignore` y `Makefile`. La estructura usa la estándar usando directorios `bin/`, `obj/`, `include/` y `src/`.

28/10/26
Se ha configurado un .`gitignore` y un `Makefile` automatizado con reglas estrictas de compilación. Se ha iniciado la Fase 1 implementando el esqueleto de `main.cpp`. El proceso padre ya es capaz de usar `fork()` y `execve()` para ejecutar binarios, y se ha añadido el bucle inicial de intercepción con ptrace.

21/11/26
Se ha cambiado por completo el código original. Ahora `main.py` es mucho más simple y legible. También se ha creado una variable `*envp` para el `execve` la cual contiene el `$PATH` y el directorio `/tmp`.
Además, las flags de sandbox pueden ser depuradas correctamente para que en el futuro se puedan manejar.

Finalmente se ha investigado como funciona `ptrace` y como se complementa con `waitpid` para poder saber exactamente que acciones está haciendo el proceso hijo. Para poder implementado hay que evitar que se de lugar a una condición de carrera entre el padre e hijo. Por ello, antes de llamar a `execve` se le manda la señal `SIGSTOP` para parar el fork hijo antes de que ejecute el binario. Después de que el padre reciba la señal de `SIGSTOP` del hijo, este aplica el `ptrace` sobre el proceso hijo. Finalmente con el mismo `ptrace` le manda una señal para que continue el proceso de forma normal poniendo en el parámetro `option=nullptr`: `ptrace(PTRACE_SYSCALL, pid, nullptr , nullptr).

Con todo esto se ha dejado el bucle de intercepciones listo y comprobado para analizar el binario malicioso.

25/11/26
Hoy se ha investigado como poder traducir las llamadas traducidas por `ptrace`. Para poder hacerlo, hay que hacer los siguientes pasos:
1. Creamos una variable de tipo struct llamada `user_regs_struct`. Esta variable contiene una copia del estado de todos los registros de la CPU del proceso que estamos analizando. Esto nos servirá para numerosas acciones más adelante.
2. llamar a la función `ptrace(PTRACE_GETREGS, pid, nullptr, &regs)`. Con esto le damos valor a `regs` la cual es el struct mencionado antes.
3. Con `regs.orig_rax` podemos sacar una variable de tipo `unsigned long` con la cual podremos meter en un switch para poder depurar acciones que ha hecho el proceso.

También se ha creado una función llamada `read_string_from_process` la cual mediante `PTRACE_PEEKDATA`, nos permite sacar cadenas de texto del proceso según la dirección que se le pase (`unsigned long`). Esto nos sirve para ver que archivos está viendo el proceso, aunque seguramente nos sirva para muchas más cosas aparte de eso. La función definida usa una sintaxis de muy bajo nivel que cuesta de entender a simple vista. Lo que hace la función es lo siguiente:
1.  Se obtiene el tamaño de palabra de nuestro SO mediante `sizeof(long)`. Esto servirá para mover la memoria palabra a palabra.
2. Se calcula la variable `addr` para que sea un múltiplo de 8, evitando saltos incorrectos.
3. Dentro del while se saca de en palabra en palabra (`long`) lo que haya en la dirección de memoria que se haya pasado. Dentro de cada palabra se hace una conversión de los bytes a char y se recorre cada char de la palabra, obteniendo de este modo los strings que se encuentra en la dirección de memoria que se haya pasado del proceso.
4. Finalmente la función puede terminar de varias formas: La normal sería detectando el final del `c-string` que es con un `\0`. En caso de que pasara otra cosa como un error (para eso la variable `errno`) o si el string resultado está sobrepasando el tamaño que podría el kernel del SO contener para el proceso, evitando un bucle infinito (para eso el parámetro `max_len = PATH_MAX`).

Finalmente, el código está siendo un main demasiado largo en el que no se entiende casi nada para alguien que nunca haya visto el código por lo que para futuras sesiones voy a refactorizar el proyecto: Pondré cabeceras en el directorio `include`. Aún no tengo muy claro que haré pero, se me ocurre de primeras, crear un archivo que contenga funciones auxiliares y crear una clase llamada `tracer` que haga toda la lógica de mirar el proceso malicioso. En el main sería tan simple como hacer `tracer.run()`. También sería buena idea que este `.run()`, en vez de hacer prints, devuelva strings para luego hacer diversas cosas con ellos: Imprimirlos y guardarlos en logs.


02/11/26
Hoy se ha fragmentado en varios archivos:
- **`MemoryUtils`**:  Contendrá funciones auxiliares sobre sobre la memoria del proceso.
- **`ArchHandler`**: Es una interfaz para manejar ciertas funciones que cambian según la arquitectura del Linux.
- **`Tracer`**: Es la clase que se encargará de encapsular toda la lógica de vigilancia sobre el proceso a investigar.

De la clase **`Tracer`**, se está usando un `std::unique_ptr<ArchHandler>` aunque debería de mirar si de verdad es necesario en este caso usar un puntero o vale con una copia entera de la variable para poder emplear correctamente el polimorfismo. Además, la función `run()` se le ha puesto un template **`<typename... Handlers>`** para que al llamarla, se puedan poner funciones con las que manejen los strings que devuelve la función `run()` a la hora de identificar y manejar las `syscalls`. Así se puede imprimir por pantalla, guardar en logs, y demás, estos strings sin pausar el flujo del programa y sin tener que llamar la función varias veces.

Para la siguiente sesión debería implementar está fragmentación de código en el `main.cpp`, el cual de momento no a sido cambiado.

04/11/26
Después de investigar sobre `ptrace`, he visto que no hace falta implementar nada de polimorfismo de clases con `ArchHandler`, ya que `ptrace` ya tiene una función que saca las llamada de sistema sobre cualquier arquitectura con el mismo output: 
```c++
#include <sys/ptrace.h>
#include <linux/ptrace.h>
#include <sys/syscall.h>
struct ptrace_syscall_info sys_info;
ptrace(PTRACE_GET_SYSCALL_INFO, target_pid, sizeof(sys_info), &sys_info) != -1
```

También, ahora podemos distinguir si es una llamada de entrada y salida con `sys_info.op == PTRACE_SYSCALL_INFO_ENTRY` → no hace falta usar la variable `manage` que alternaba entre true y false.

Para manager las señales se hace accediendo al siguiente atributo: `sys_info.op` y para coger la dirección de memoria del proceso (para usarlo en `read_string_from_process`) se haría así: `sys_info.entry.args[1]`. 

Para la siguiente sesión, se eliminarán las clases de `ArchHandler` y terminará la clase `Tracer`.