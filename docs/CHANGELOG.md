*27*/10/26
Se ha creado la estructura básica del proyecto, con un `.gitignore` y `Makefile`. La estructura usa la estándar usando directorios `bin/`, `obj/`, `include/` y `src/`.

---
28/10/26
Se ha configurado un .`gitignore` y un `Makefile` automatizado con reglas estrictas de compilación. Se ha iniciado la Fase 1 implementando el esqueleto de `main.cpp`. El proceso padre ya es capaz de usar `fork()` y `execve()` para ejecutar binarios, y se ha añadido el bucle inicial de intercepción con ptrace.

---
21/11/26
Se ha cambiado por completo el código original. Ahora `main.py` es mucho más simple y legible. También se ha creado una variable `*envp` para el `execve` la cual contiene el `$PATH` y el directorio `/tmp`.
Además, las flags de sandbox pueden ser depuradas correctamente para que en el futuro se puedan manejar.

Finalmente se ha investigado como funciona `ptrace` y como se complementa con `waitpid` para poder saber exactamente que acciones está haciendo el proceso hijo. Para poder implementado hay que evitar que se de lugar a una condición de carrera entre el padre e hijo. Por ello, antes de llamar a `execve` se le manda la señal `SIGSTOP` para parar el fork hijo antes de que ejecute el binario. Después de que el padre reciba la señal de `SIGSTOP` del hijo, este aplica el `ptrace` sobre el proceso hijo. Finalmente con el mismo `ptrace` le manda una señal para que continue el proceso de forma normal poniendo en el parámetro `option=nullptr`: `ptrace(PTRACE_SYSCALL, pid, nullptr , nullptr).

Con todo esto se ha dejado el bucle de intercepciones listo y comprobado para analizar el binario malicioso.

---
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

---
02/11/26
Hoy se ha fragmentado en varios archivos:
- **`MemoryUtils`**:  Contendrá funciones auxiliares sobre sobre la memoria del proceso.
- **`ArchHandler`**: Es una interfaz para manejar ciertas funciones que cambian según la arquitectura del Linux.
- **`Tracer`**: Es la clase que se encargará de encapsular toda la lógica de vigilancia sobre el proceso a investigar.

De la clase **`Tracer`**, se está usando un `std::unique_ptr<ArchHandler>` aunque debería de mirar si de verdad es necesario en este caso usar un puntero o vale con una copia entera de la variable para poder emplear correctamente el polimorfismo. Además, la función `run()` se le ha puesto un template **`<typename... Handlers>`** para que al llamarla, se puedan poner funciones con las que manejen los strings que devuelve la función `run()` a la hora de identificar y manejar las `syscalls`. Así se puede imprimir por pantalla, guardar en logs, y demás, estos strings sin pausar el flujo del programa y sin tener que llamar la función varias veces.

Para la siguiente sesión debería implementar está fragmentación de código en el `main.cpp`, el cual de momento no a sido cambiado.

---
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

---
06/11/26
He estado implementado el nuevo descubrimiento de `<linux/ptrace.h>` el cual tiene el struct de `syscall_info` tan útil, pero me he topado con un problema que resulta ser bastante común: Las librerías `<sys/ptrace.h>` y `<linux/ptrace.h>`. Esto se debe a que son entidades separadas: Las librerías de `<linux/>` son de la API de Linux los cuales están mas actualizados que las librerías de `<sys>` que son del [C stándar](https://www.open-std.org/jtc1/sc22/wg14/).

Al principio, tenía pensado deshacerme por completo de `<sys/ptrace.h>` haciendo una función propia que imite a `ptrace()`:
```C++
inline long ptrace(enum __ptrace_request request, pid_t pid, void *addr, void *data)
{
	return syscall(SYS_ptrace, request, pid, addr, data);
}
```

Pero tras una exhaustiva investigación he llegado a la conclusión que lo mejor sería definir un header file que incluye `<sys/ptrace.h>` y las variables a mano que quiero usar de la API de Linux. Es mejor así porque así seguiré usando el estándar POSIX, evitando problemas de compatibilidad con otras librerías como los `Linux Namespaces` o `seccomp` en el futuro. Además, con esta otra solución, el programa será compatible con más sistemas Linux ya que prácticamente todos tienen `<sys>` mientras que la API Linux no está en tantos.

Con esto, he credo el header file `KernelAbi.hpp` donde `Abi` significa Application Binary Interface, que es como una API pero solo define variables. Del header file he copiado las variables de `ptrace_syscall_info` del propio [kernel de linux](https://github.com/torvalds/linux/blob/master/include/uapi/linux/ptrace.h) y adaptado para que funcione en C++.

Finalmente, tras estar todo el día programando esto, el `main.cpp`  es legible. Para lograr esto he hecho todos estos cambios:
1. He creado la clase `Tracer` en `Tracer.hpp` el cual maneja toda la parte de saber que acciones el proceso está ejecutando (para futuras versiones habría que simplificar en `.run()` en varias funciones). Al principio la clase se dividía en header file y `cpp`, pero resulta que el compilador tiene que saber lo que hace una función con templates en el mismo lugar donde se define por lo que tenía que juntar todo en el `.hpp`. Lo único malo de esto sería que al hacer un make, no detectará los cambias si ya fue compilado, por lo que habría que hacer un make clean y luego make de nuevo. En la siguiente sesión miraré si se puede hacer algo al respecto.

2. He creado el name space `psx` que tiene funciones auxiliares para que el manejo de funciones POSIX sea más ameno.

3. El `Makefile` lo he revertido a su versión antigua ya que no usaré nada de manejar el Arch.

4. El main ha sido simplificado: Ahora solo se maneja las flags (que podría meterlo en otra función aparte pero me parecería innecesario); se instancia la clase Tracer y se llama la función `run()` con una lambda temporal creada para ver la salida.

---
18/11/26
Hoy he refactorizado la clase `Tracer` ya que tenía varios problemas con el manejo de procesos hijos del binario malicioso:
1. En la versión antigua, si el binario malicioso ejecutaba un `fork()` o `clone()` y terminaba el proceso padre, `sandbox` daba por terminada la ejecución y estos procesos hijos seguirían ejecutándose sin supervisión.  Para solucionar esto he puesto opciones extra a `ptrace`; `PTRACE_O_TRACEFORK | PTRACE_O_TRACECLONE | PTRACE_O_TRACEVFORK`, además de que `waitpid` tiene nuevos argumentos; `waitpid(-1, &m_status, __WALL)`. Con estos parámetros el kernel analizará también los procesos hijos para que podamos vigilaros en el bucle while hasta que mueran (`ECHILD`).

2. Ahora, si matas el proceso de `sandbox`, con el nuevo option `PTRACE_O_EXITKILL`, garantizamos que al morir, también mueran todo los hijos que estaba trazando.

3. Antes `psx::resumeProcess` mandaba al proceso parado la señal 0 en el cuarto parámetro, haciendo que que olvidara cualquier señal enviada al proceso originalmente. Para solucionar esto he hecho varios cambios; Primero la propia función, ahora recibe dos parámetros, el `pid` y la señal que manda `signal=0` para que dependiendo del caso, mande 0 o las señales que queden por mandar. También en `PosixUtils` he creado una `struc enum` que enumera los 4 tipos de señales que da `waitpid` que son importantes para `ptrace`;

```C++
enum class StopKind {
Ended, /* exited or killed: never resume it */
PtraceEvent, /* fork / vfork / clone / exec / first stop of a new child */ SyscallStop, /* syscall entry or exit (SIGTRAP | 0x80) */
SignalDelivery /* a real signal for the tracee: must be re-injected }; */
```

3. Esta clase se usa en una nueva función `StopKind classifyStop(int status)` que nos hará el trabajo de parsear las 4 situaciones que se pueden dar dependiendo del resultado de `waitpid`. Estas situaciones se pasan finalmente en un switch en el cual, si se se da el caso de `psx::StopKind::SyscallStop` se ejecutará el antiguo switch de la versión anterior que manejaba el `PTRACE_SYSCALL_INFO_ENTRY`. En el caso de que se diera `SignalDelivery` (señales para el proceso que necesita) se llama a `psx::resumeProcess` con el parámetro `signal=WSTOPSIG(m_status)` para que pueda continuar su correcto funcionamiento.

Con todos estos arreglos `sandbox` es ahora capaz de detectar subprocesos del proceso malicioso. Lo siguiente que debería de hacer sería simplificar más la función `run` del `Tracer` y también tengo que mirar si debería de usar `std::uint_64t` o `usigned long` por temas de compatibilidad con distintas arquitecturas.

---
19/11/26
He simplificado un poco más la función `run` del `Tracer` dividiendo el manejo de de la `syscall` en una función privada llamada `manageSyscall(pid_t pid)`.

También he implementando por fin el uso (de forma primitiva) el uso de los Linux Namespaces. Los Linux Namespaces son una funcionalidad de Linux proporcionada por la librería `<sched.h>` la cual nos permite aislar recursos de un proceso de una forma muy sencilla y efectiva: Con tan solo usar la función `unshare(FLAGS)` podemos aislar recursos de un proceso como: Internet, el sistema de archivos, usuarios, procesos, hostname y dominio, etc.

Esta función `unshare` ha sido muy fácil de implementar, tan solo ha sido necesario cambiar la función `launchProcess`:
1. Se ha puesto un nuevo parámetro de tipo `int` para pasar las flags de `unshare`.
2. Se ha puesto la función `unshare` justo después de hacer el `fork` (en el proceso hijo, que será el que ejecutara `execve` malicioso). 

Se ha testeado si funciona con la flag `CLONE_NEWNET` la cual aísla la red y ha sido un éxito:
```java
ping: 1 
google.com: Temporary failure in name resolution1 
```

Finalmente, como se están usando Linux Namespaces a partir de ahora, no veo la necesidad de crear a mano la variable `envp` para limitar lo que el proceso puede ver. Por esto mismo he cambiado la función `execve` por `execvp` la cual es exactamente igual que este pero usar de variable de entorno `environ`, el cual el es mismo entorno que el del usuario que ejecuta sandbox, y tiene el `PATH` para que ejecutar binario sea más cómodo.

Para la siguiente sesión debería de hacer que no haga falta usar sudo para ejecutar `sandbox` con los namespaces.

---
20/11/26
Hoy he mejorado la función `launchProcess`: Antes se necesitaba root para ejecutar `sandbox` ya que el kernel pide permisos de root para crear namespaces, a no ser que uses la flag `CLONE_NEWUSER`. Esta flag hace pensar al proceso maliciosos se está ejecutando como el user nobody (65534) el cual no tiene permisos para casi nada. En principio con este cambio ya no necisitarías usar sudo para cada uso de sandbox y aisla al proceso a un usuario sin permisos pero he sería mejor que el proceso sepa que usuario lo ejecutó (uid) para que intente hacer acciones y poder investigar. De todas formas aunque se ejecuta como tu usuario, en principio no debería de poder tus archivos ya que se usaría por defecto la flag `CLONE_NEWNS`. Además, podríamos hacer que el proceso piense que se esté ejecutando como root aunque no lo sea para ver que acciones tomaría.

Para hacer que el proceso sepa con qué usuario se está ejecutando hay que editar ciertos valores de archivos en el directorio `/proc/<pid>/`. Al editar estos archivos no se está realmente guardando texto en ninguna parte del SO, en realidad está enviando señales al kernel para traducir las tablas de traducción del proceso malicioso.

He creado dos funciones nuevas en `PosixUtils` para editar estos archivos:
- `writeProcFile`: Abre cierto archivo y pone el contenido. Antes, en vez de usar esta función, usaba `std::ofstream` pero me ví limitado ya que no podía manejar errores y ser tan exacto que con las funcines POSIX de open() write(), a sí que al final he creado esta función que hacía fácil el escribir en los ficheros de proc y manejar errores.

- `mapUserNamespace`: Primero intenta deshabilitar la llamada al sistema `setgroups()` para poder editar posteriormente los ficheros `uid_map` y `gid_map` (los que hace ver el usuario y grupo al proceso). Dependiendo de si la escritura ha sido exitosa o no, devolverá un booleano que se usará para parar el problema en caso de fallo en la función `psx::launchProcess`.

Con esta sesión he aprendido bastante como funciona la comunicación de los procesos con el kernel usando ficheros. Al principio me parecia que esta solución era chapucera ya que se estaban ejecutando procesos durante la ejecución y poniendo strings a mano, pero resulta que es la forma en la que se hace estas cosas.

---
21/11/26
Resulta que existe una función llamada `std::format()` que hace básicamente lo que haría en python un string que empieza por `f""`. Menos mal que Gemini ha decidido contarle, le ha costado. Con esta función ya no se montaran más string usando operandos, lo cual me parecía muy chapucero. Además el propio `format` ya hace la conversión de tipos a string, a sí que nada más de `std::to_string()`. Finalmente, con el nuevo uso de format, el código será mucho más seguro ya que se evitan problemas de tipos en tiempo de compilación y, tiene un rendimiento mucho mayor que usar operadores ya que no tiene que crear varios strings separados y por tanto, solo reserva memoria en el heap una vez.

Hoy he creado un namespace que va a ser super util: `snd` del archivo `SandUtils.hpp`. Este namespace sirve para hacer el parseo u uso de flags mucho más cómodo.
Primero tenemos la clase Flags la cual tiene la siguiente estructura:
```c++
class Flags {
public:
  bool network = false; // -n
  bool read = false;    // -r
  bool write = false;   // -w
  bool user = false;    // -u
  bool exec = false;    // -x
  int nmflags = 0;

  Flags(std::uint8_t options = 0);
  std::string toString() const;
};
```

Con esta estructura, ver si sandbox le está otorgando algún recurso como internet es tan fácil como ver el boolean (`.network`). En el constructor he querido hacer algo más interesante, usando una máscara de bit como parámetro:

```c++
constexpr std::uint8_t NETWORK = 1u << 0;
constexpr std::uint8_t READ = 1u << 1;
constexpr std::uint8_t WRITE = 1u << 2;
constexpr std::uint8_t USER = 1u << 3;
constexpr std::uint8_t EXEC = 1u << 4;
```

No se si la verdad usar esto hace que sea más ligero o rápido instanciar la clase, pero me ha gustado implementarlo. Además, hay una variable `nmflags` el cual es el bitmap que usar el método `unshare()` para pasar sus flags. En el constructor de esta clase se crea también el bitmap de namespaces de la siguiente forma:

```c++
snd::Flags::Flags(std::uint8_t o)
    : network(o & NETWORK), read(o & READ), write(o & WRITE), user(o & USER),
      exec(o & EXEC) {
  if (!network) {
    nmflags |= CLONE_NEWNET;
  }
  if (!write) {
    nmflags |= CLONE_NEWNS;
  }
  if (!user) {
    nmflags |= CLONE_NEWUSER;
  }
  if (!exec) {
    nmflags |= CLONE_NEWPID;
  }
}
```

Para poder hacer mas legible el parseo de flags a bits en el constructor de esta clase, he creado estas dos funciones: 
- `snd::letterToBit(char letter)`: Recibe una letra (`w,r,x,n`) y devuelve su respectivo `std::uint_8` de los que hemos definido arriba.
- `std::uint8_t snd::argsToFlags(int &i, int argc, char *argv[])`: Recibe los `argv` y los pasa uno a uno por dos bucles; Uno de palabra y otro de cada letra. Los chars pasaran por la función definida anterior y se sumará a la mascara de bits.

En cuanto las flags, he puesto una nueva `-A` la cual ejecuta el proceso sin restricciones (a sí que tener cuidado usando esto) y he cambiado `-i` por `-n` ya que la palabra más apropiada sería "network", no "internet".

---
22/11/26
Hoy ha sido un día de desarrollo muy duro y una pérdida de tiempo. Intenté implementar un entorno de trabajo completamente falso para el proceso malicioso: Este estaría montado en `/tmp/sandbox_XXXXXX`, el proceso pensaría que esta viendo y editando archivos del árbol real (desde root), pero en realidad siempre que escriba un archivo, se guardaría en `/tmp/sandbox_XXXXXX`. Tras unas 16 horas de programación he tenido que tirar la toalla. Me quede bastante cerca de lograrlo pero estaba demasiado quemado y la entrega se acerca a sí que debo de cumplir con los hitos obligatorios que he prepuesto. Finalmente, probé a ver como de cerca estaba y genere con IA la solución, y funcionaba, pero el código era extremadamente inconsistente y tenía pinta de que se rompería en cualquier momento además de que el proceso podría escaparse si estuviera bien hecho.

Por todo estas razones, además de que no puedo generar código con IA, he decido dejar atrás este concepto tan interesante que casi logro. Si sigo con el proyecto en un futuro después de la entrega podría volver a intentar con esta idea ta interesante pero de momento mantendré sandbox más seguro, robusto y sobre todo, lo terminaré a tiempo sin hacer trampas.

---
24/11/26
Como ultimamente mi velocidad de desarrollo ha sido nefasta, he decido probar otro approach: Ahora que voy a implementar el `seccomp` y terminar de una vez con la fase 2, lo que haré primero, antes de escribir ni una sola línea de código, será investigar que es `seccomp`, luego, una vez tenga claro como funciona, escribiré unas check boxes con las tareas que tengo que hacer para lograr mi objeto, asegurándome de que son correctas las definiciones de las tareas. Finalmente me pondré a codear, completando tareas y debería de funcionar sin tantas complicaciones como las que he tenido desarrollando como he estado haciendo antes (como que nunca funciona, no se que estoy haciendo en el fondo, no sé como funciona la librería, el código siempre tengo que refactorizarlo luego...). Con esto claro, esto es lo que he sacado de la investigación:

- [x] Añadir a `StopKind` una variable nueva que sea `SeccompEvent`. En la función  `clasifiyStop` devolver `SeccompEvent` si `WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP && ((status >> 16) == PTRACE_EVENT_SECCOMP))`. Con estos cambios podremos saber que interrupciones son llamadas de `seccomp`.

- [x] Reemplazar por completo el `StopKind` de `SyscallStop` por el nuevo de `SeccompEvent` y el manejo antiguo, ya que con `PTRACE_GETEVENTMSG` se podrá filtrar de forma mucho más eficiente (ya que solo se para con las llamadas que bloqueamos), y con el resultado se podrá parsear que hacer con el dependiendo de que tipo de acción hizo y  así (si está permitido o no cuando pasamos las flags previamente) bloquearlo con `EPERM` (no tiene permisos para hacer) o con `EACCES` (tiene permiso de la acción, pero no de los recursos concretos).

- [x] Instanciar `seccomp` con `scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_ALLOW)`. Con esto se permite todo en principio y luego iremos poniendo en la lista negra que cosas peligrosas no puede hacer. El contexto `SCMP_ACT_ALLOW` dice que intercepte las llamadas que se pongan en las reglas pero no mate el proceso directamente.

- [x] Definir las reglas de `seccomp`. Para ello se usa por ejemplo `seccomp_rule_add(ctx, SCMP_ACT_TRACE(TAG_READ), SCMP_SYS(openat), 0);`. Hay ciertas llamadas a sistema como `OPENAT` que se usan para dos tipos de regla, como en este caso es leer y escribir, por lo que a la hora de definir la regla, se usará una etiqueta solo para instrucciones de tipo `OPEN` y luego en el switch que mira ese grupo se mirarán los argumentos para saber si se trata de lectura o de escritura.

- [x] Hacer `seccomp_load()` **en el proceso hijo**. La instancia y definición de normas se harán en el padre.

- [x] Interceptar las instrucciones de `seccomp` usando `classifyStop`→`SeccompEvent`.

- [x] Coger la información de la instrucción en CPU con `ptrace(PTRACE_GET_SYSCALL_INFO, ...)`. De este información mirar en orden: `.op`  para estar seguro de que es una instrucción de tipo `PTRACE_SYSCALL_INFO_SECCOMP`, mirar `.ret_data` para saber a que grupo de flag (conexión, lectura, escritura, ejecución, ...) pertenece la instrucción que ha saltado cierta norma de `seccomp`, mirar `.nr` para saber que llamada de sistema hizo y con esto sacar su información y guardarlo en el log.

- [x] Si las flags de  a la hora de ejecutar `sandbox` no están presentes en el grupo de regla que se ha saltado, bloquear la instrucción con:
```c++
// Solo funciona en x86_64
struct user_regs_struct regs{};
ptrace(PTRACE_GETREGS, target_pid, nullptr, &regs)
regs.orig_rax = static_cast<unsigned long long>(-1);
regs.rax = static_cast<unsigned long long>(-EPERM);
ptrace(PTRACE_SETREGS, pid, nullptr, &regs);
```

- [x] En todas las interrupción por llamada de `seccomp`, se reanudará (con bloqueo o permitido) con `ptrace(PTRACE_CONT, pid, nullptr, nullptr);`

- [x] Etiquetar primero por grupo con `.ret_data` y luego por instrucción en concreto con `.nr`.

- [x] Cambiar configuración de `ptrace` para que funcione con `seccomp`: Poner en sus opciones **`PTRACE_O_TRACESECCOMP`**.

- [x] Limpiar el objeto `seccomp` con `seccomp_release(ctx);`.

- [x] Arreglar el problema de incompatibilidad de `seccomp` bloqueando ejecutar binarios con `execve` justo al lado. Para ello, simplemente, antes de que se llame a run, en el constructor de `Tracer`, hacer algo para que reciba la primera instrucción detenida (que debe de ser la de `execve`) y la pase por alto.


---
25/11/26
A la hora de bloquear las operaciones de lecturas del proceso malicioso hay un problema: Si no puede leer nada → no puede ejecutar librerías y recursos que necesita para funcionar.

La solución va a ser que en vez de bloquear con `seccomp` las instrucciones de lectura, usaré el `CLONE_NEWNS` para montar un overlay en directorios con información delicada, como `home/`. 

Esto es lo que se hará (hacerlo en próximas sesiones):
- **Para `/home`, un overlay en vez de un volumen vacío.** El mismo overlay que ya tienes, pero con `lowerdir` apuntando a un directorio vacío en lugar del `/home` real. Con un solo montaje consigues las dos cosas: el proceso no ve nada tuyo y todo lo que escriba queda en `capture/`.
- **Volver a poner `CLONE_NEWPID` y montar un `/proc` nuevo**, sin eso, lo demás tiene un agujero enorme. Si lo quitaste por algún problema concreto, ese problema es el que hay que resolver.
- **Tapar con un tmpfs vacío** `/run/user/<uid>`, `/dev/shm`, `/media` y `/mnt`. `/tmp` y `/var/tmp` también, si tu overlay no los cubre ya.
- **`chdir` después de los montajes.**
- **seccomp solo para lo que los namespaces no pueden hacer:** io_uring, `open_by_handle_at`, `ptrace` y la red como segunda capa.

- **La flag `-r` cambia de significado.** Pasa de bloquear la lectura a ocultar los datos personales. Me parece mejor diseño, pero es un cambio respecto a `definition.md` y tiene que aparecer en la tabla de cambios de la memoria final.
- **El calendario.**  El overlay con lower vacío, `NEWPID` con `/proc` nuevo y el `chdir` son cambios pequeños sobre código que ya tienes. El resto, si no llega, lo puedes documentar como limitación conocida.

Hoy, a aparte de ese prompt de arriba que tengo guardado para saber como bloquear el acceso del proceso malicioso a directorios con información delicada, sin que pierda acceso a recursos necesarios para que funcione, he cumplido con todas las tareas que definí y la verdad es que el desarrollo ha ido bastante bien. Ha sido un acierto cambiar este método de desarrollo.

Para ir previniendo la implementación de `journalctl`, he mirado un poco como funciona y va a ser muy útil que lo handlers de `Tracer.run` reciban, en vez de un simple `std::string`, un struct que contenga varios datos sobre la llamada a sistema, haciendo que sean fáciles de manejar por los handlers (en principio el que guardará los logs). Este `struct` está definido en el nuevo header file 

Finalmente para rellenar el campo target de este nuevo tipo, hay una limitación según Claude que sería necesaria documentar:
**Limitación de TOCTOU.** Lees la ruta o el `sockaddr` en la parada de `seccomp`, antes de que el kernel los copie. Un malware con varios hilos podría cambiar esa memoria justo después de tu lectura: tú registras `/tmp/inocente` y el kernel abre `/etc/shadow`. Es un problema conocido de todos los sandboxes basados en `ptrace`.

---
26/11/26

He cambiado la estructura general del código un poco: Antes tenía una mascara de bits `std::uint8_t` que solo servía como parámetro para el constructor de `snd::Flags`. También tenía un `struct` `enum` llamado Groups que hacía practicamente lo mismo que la máscara  de bits pero más seguro ya que es un `enum`. Lo que he hecho ha sido mover `psy::Sample` a `SandUtils.hpp`, el header file `Autopsy.hpp` lo he borrado porque se quedó vacío. Ahora siempre que se usa variable de Grupo, tanto como el constructor de `Flags` como para leer `.ret_data`, son siempre del tipo `Group` definido en `SandUtils.hpp`. Además de todo esto, para que funcione correctamente el `enum` como una máscara de bits, he hecho las funciones de operadores `|, &, |=`, además de que se pueden pasar a string con su función `group_toString` que se usa en la re-definición de `std::format`.

He implementado todas las funciones de lectura de memoria RAM en `PosixUtils` las cuales se usan en `Tracer` para rellenar el parámetro `target` de `sample`.

Para que `execve` funcione sin bloquearse de inmediato por `Tracer` con el `seccomp`, he puesto un bool `m_launched` el cual está inicializado en false. Cuando se llega a un `Stopkind::PtraceEvent` y su bit 16 es igual `PTRACE_EVENT_EXEC` (evento de que se ejecutó `execve`) a la vez que el `m_pid` (`pid` del proceso malicioso) es igual al  `m_cPid` `pid` del proceso detenido (con esto comprobamos que no es un proceso hijo del binario malicioso), pasamos `m_launched` a true. Con el método privado `isLaunch` se comprueba si el grupo `EXEC` ejecutado es del proceso original `execve`.

Sobre las funciones `networkCall(), readCall(), writeCall() ...`  básicamente se está mirando las posibles `sys calls` (mirando `.nr`) y según el tipo, sacar la información para `.target` de la memoria, mirando en distintos argumentos, ya que dependiendo de la instrucción a sistema, la dirección que nos interesa puede estar en una `arg` o en otro.

Tengo que mirar como quita el warning del case cuando invento valores de Group usando `Group | Group`. También voy a cambiar como se manejan las operaciones de lectura: Ahora mismo si no está permitido la lectura se bloquean directamente con la función `blockSeccompProcess`. Esto es un problema porque como mencioné en previos logs, los binarios necesitan leer de librerías y recursos para funcionar. Se me ocurren dos cosas: Compilarlo estáticamente para que los recursos los tengo en el propio código, o como mencioné el otro día montar overlays en `/home` y `/tmp`.

Otro cambio nuevo, es que ahora para que se puedan leer las IPs que está intentado acceder el binario malicioso sin que tenga permisos de red u o usuario dándole acceso al puerto `ICMP` mediante la función `allowSocketICMP`.

Tras investigar sobre el tema de lectura, he llegado al siguiente plan de trabajo. En vez de usar `overlays`, usaré `mounts` como `tmpfs`. Debería ser fácil de implementar:

#### Fase 0. Preparación
- [x] Hacer commit de lo que tienes ahora, para poder volver atrás.
- [x] Crear en `/home/<tu usuario>/prueba/` un binario de prueba dinámico (un "hola mundo" vale).
- [x] Crear en ese mismo directorio un fichero `secreto.txt` con cualquier contenido.
- [x] Crear en ese mismo directorio un script `prueba.sh` con `#!/bin/sh` y un `ls`, y darle permiso de ejecución.
- [x] Crear un segundo binario de prueba que intente abrir `secreto.txt` por ruta relativa y por ruta absoluta, e imprima si lo consigue.
#### Fase 1. Namespace de montaje también cuando falta `-r`
Archivo: `src/SandUtils.cpp`, constructor de `Flags`.
- [x] Localizar el `if (!write)` que añade `CLONE_NEWNS` a `nmflags`.
- [x] Cambiar la condición para que se cumpla cuando falte `-w` **o** falte `-r`.
Archivo: `src/PosixUtils.cpp`, función `launchProcess`.
- [x] Localizar el `if (!flags.write)` que llama a `isolateMnt()`.
- [x] Cambiar la condición a la misma de arriba: falta `-w` o falta `-r`.
- [x] Compilar y comprobar que `./bin/sandbox -w <binario>` sigue arrancando igual que antes.
#### Fase 2. Función que oculta los directorios
Archivo: `include/PosixUtils.hpp`.
- [x] Añadir `#include <filesystem>`.
- [x] Declarar una función nueva en `psx` que reciba la ruta del binario (`const char *`) y devuelva `bool` (por ejemplo, `hideUserDirs`).
Archivo: `src/PosixUtils.cpp`. Dentro de la función nueva, en este orden:

**2.1. Guardar lo que hace falta antes de tapar nada**
- [x] Añadir `#include <system_error>`.
- [x] Declarar un `std::error_code` para todas las llamadas de `std::filesystem` (así no lanzan excepciones).
- [x] Guardar el directorio de trabajo actual con `std::filesystem::current_path(ec)`.
- [x] Calcular la ruta absoluta del binario con `std::filesystem::canonical(binaryPath, ec)`.
- [x] Guardar en un `bool` si `canonical` ha funcionado. Si falla, el argumento es un nombre suelto (`ping`) y se buscará en el `PATH`.
- [x] Comprobar si esa ruta absoluta empieza por `/home/`, `/root/` o `/run/user/`. Guardarlo en otro `bool`.
- [x] Solo si ambos `bool` son verdaderos: abrir el binario con `open(ruta, O_PATH | O_CLOEXEC)` y guardar el descriptor.
- [x] Si ese `open` devuelve -1: `perror` y devolver `false`.

**2.2. Tapar los directorios**
- [x] Montar el tmpfs sobre `/home`: `mount("tmpfs", "/home", "tmpfs", MS_NOSUID | MS_NODEV, "size=1m,mode=755")`.
- [x] Si falla con un error distinto de `ENOENT`: `perror` y devolver `false`.
- [x] Repetir el montaje y la comprobación para `/root`.
- [x] Repetir el montaje y la comprobación para `/run/user`.

**2.3. Volver a hacer visible el binario**

Solo si se abrió el descriptor en 2.1:

- [x] Recrear los directorios padre del binario dentro del tmpfs: `std::filesystem::create_directories(ruta.parent_path(), ec)`.
- [x] Crear un fichero vacío con el nombre del binario: `open(ruta, O_WRONLY | O_CREAT, 0755)` y cerrarlo enseguida.
- [x] Construir el origen del bind mount: `std::format("/proc/self/fd/{}", descriptor)`.
- [x] Hacer el bind mount: `mount(origen, ruta, nullptr, MS_BIND, nullptr)`.
- [x] Si falla: `perror` y devolver `false`.
- [x] Remontarlo en solo lectura: `mount(nullptr, ruta, nullptr, MS_BIND | MS_REMOUNT | MS_RDONLY | MS_NOSUID | MS_NODEV, nullptr)`.
- [x] Si falla: `perror` y devolver `false`.
- [x] Cerrar el descriptor con `close`.

**2.4. Cerrar la fuga del directorio de trabajo**

- [x] Recrear el directorio de trabajo guardado: `std::filesystem::create_directories(cwd, ec)`.
- [x] Hacer `chdir(cwd.c_str())`.
- [x] Si el `chdir` falla: hacer `chdir("/")` como último recurso. Nunca continuar sin haber hecho un `chdir` con éxito.
- [x] Devolver `true`.

#### Fase 3. Llamar a la función desde el hijo
Archivo: `src/PosixUtils.cpp`, función `launchProcess`, dentro del `if (pid == 0)`.
- [x] Colocar la llamada después de `isolateMnt()` y antes de `sec::loadSeccomp()`.
- [x] Llamarla solo cuando falte `-r` (`!flags.read`).
- [x] Si devuelve `false`: `_exit(1)`. Si no se puede ocultar, el binario no se ejecuta.
- [x] Comprobar que el orden final del hijo es: `unshare`, `raise(SIGSTOP)`, `isolateMnt`, ocultar directorios, `loadSeccomp`, `execvp`.
- [x] Dejar el `execvp` tal como está, con el argumento original.

#### Fase 4. Dejar de bloquear las lecturas
Archivo: `src/Tracer.cpp`, función `manageSyscall`.
- [x] Localizar la línea que calcula `sample.is_allowed`.
- [x] Añadir un caso: si el grupo es exactamente `READ`, la llamada se permite siempre.
- [x] Añadir otro caso: si el grupo es `READ | WRITE` (abrir con `O_RDWR`), se permite solo si el usuario dio `-w`. Si no lo tratas aparte, seguirá exigiendo también `-r`.
- [x] Dejar el resto de grupos como están.
- [x] Comprobar que las lecturas siguen llegando al handler: deben seguir apareciendo en el log.

Archivo: `src/Warden.cpp`. No hay que tocar nada: las reglas de lectura se quedan para que el log siga funcionando.
#### Fase 5. Pruebas
Lanzando siempre desde `/home/<tu usuario>/prueba/` y sin `sudo`:
- [x] `sandbox ./binario` (sin flags): el binario dinámico arranca.
- [x] `sandbox ./prueba.sh`: el script se ejecuta y su `ls` solo muestra `prueba.sh`.
- [x] `sandbox ./lector`: `secreto.txt` no se puede abrir ni por ruta relativa ni por absoluta.
- [x] `sandbox -r ./lector`: `secreto.txt` sí se puede abrir.
- [x] `sandbox true`: un nombre suelto se sigue encontrando en `/usr/bin`.
- [x] `sandbox -w ./binario`: arranca, y lo que escriba en `/home` no aparece en tu `/home` real al terminar.
- [x] `ls /home/<tu usuario>` en otra terminal mientras el sandbox corre: tus ficheros siguen ahí. El montaje no ha salido del namespace.
- [x] Lanzar desde `/tmp` un binario que esté en `/tmp`: arranca y no da error en el `chdir`.
- [x] `make` sin avisos nuevos.

---
27/11/26
Tras cumplir todos los checkbox, y completar los tests, `sandbox` ahora funciona sin usar `-r`.  Luego explico como funciona todo, por ahora hay que destacar las limitaciones de este nuevo `sandbox`:
- Un binario que sea un **symlink** dentro de `/home` no queda visible en el tmpfs.
- Sin `-w`, las aperturas de `/dev/tty` en lectura-escritura se bloquean, así que los programas interactivos pierden la terminal.
- El aislamiento de lectura cubre `/home`, `/root` y `/run/user`, no todo el sistema. El resto (`/etc`, `/usr`…) sigue siendo legible, y es a propósito, para que los binarios puedan cargar sus librerías.
- La traza de rutas es **informativa**: la protección real la hace el kernel con los montajes, porque comprobar rutas desde el tracer es vulnerable a `TOCTOU`.

Además de todo esto, y para terminar la sesión de hoy, he creado la clase (sin terminar) Logger, la cual será la que se encargará de mostrar los eventos de `sandbox`. Los eventos se pueden mostrar de tres formas: Por consola con `.print()`, en un archivo con `.save()` y en los logs del sistema con `log()`.

Para decidir que opción usar a la hora de usar sandbox, he hecho un par de cambios en la clase flags: Ahora tiene el booleano help y silent además de un `string_view` que contiene el path del archivo donde queramos guardar los eventos, y a la vez hace de booleano ya que se puede comprobar con `.empty()` si se le paso la opción de guardar salida. Para poder hacer que esto funcione se ha creado una nueva función `parseArgs` que remplaza a `argsToGroups` ya que ahora hay argumentos que no forman parte del tipo `Groups` y no tendría sentido que formaran, como silent, o help.

---
28/11/26
Hoy he terminado con los logs. Para ello he creado la clase `Logger` que mencioné ayer, y con el operador `()` de ejecutan las tres funciones (si puede) de escritura por terminal `print`, escritura en archivo de salida `save` y escritura en los logs del sistema `log`.

Además, las salidas de la terminal aparecen con colores gracias a la ayuda del nuevo header file `Color.hpp` en el cual se define un `struct enum` de colores, una función que convierte el `struct enum` ah `std::string_view` para usarlos en los prints, y una función `paint` que le pases un string del texto que quieras y un color (lo típico).

Como las salidas aparecían con mucho ruido (eventos del Tracer que se repetían mucho), he añadido una flag nueva `verbose -v` que muestra todos los eventos de `Tracer`.  En caso por defecto, solo aparecen las señales que se han bloqueado. Para mostrar las señales bloqueadas, se ha cambiado un poco la lógica de cómo se da valor al bool `sample.is_allowed`. Ahora, en caso de que es de lectura (o `read_write` en algunos casos), no se bloque como antes pero además no se pone  en false el `sample.is_allowed`, para que el usuario entienda que aunque se haya ejecutado la instrucción de lectura, en realidad no está leyendo el directorio real, sino un mount de `tmpfs` encima del original, por lo que se ha bloqueado técnicamente.

Para comprobar que el directorio que se ha bloqueado forma parte de los que se han montado, he creado una función llamada `isProtected` que comprueba que el path que le pases esté en la nueva lista creada de directorios protegidos llamada `PROTECTED_DIRS`.

Esta lista es un buen cambio para el principio dry, ya que estaba repitiendo muchas veces la misma definición de los strings para los directorios que quiero proteger de lectura (`/home`, `/root`, `/run/user`). Ahora si quisiera añadir un nuevo directorio para proteger, sería tan fácil como poner un nuevo string en el array `PROTECTED_DIRS`.

También, para terminar con el proyecto, se han hecho los tests. En este caso, como los binarios maliciosos de prueba no forman parte del propio sandbox, me he tomado la libertad (ya que voy muy mal de tiempo) de generarlos con IA (mirar el prompt en [prompts.txt](prompts.txt)). Luego en el `Makefile` he añadido una nueva flag `test` que compila los ficheros `.cpp` que se encuentran en [tests](../tests) y ejecutan cada binario comprobando el caso de que se aisle (printea CONTAINED) y el caso de que no lo logre aislar (BREACHED).

Finalmente para terminar de una vez con este proyecto, he completado los `//TODO`  que habían en `Tracer.hpp`: El segundo simplemente no lo he implementado por que me parecía innecesario mostrar que `pid` nueva se ha creado, ya que el tipo `sample` ya mostraría eso; En cuanto al primero `TODO`, el caso de que termine mande la señal de terminado el `m_cPid`, primero comprueba que se trate del proceso malicioso original, luego si se da el caso mira si el proceso termino por su propio cuenta con `WEXISTATUS`, si es el caso se devuelva la señal con la que terminó el proceso por su cuenta. Si no terminó el proceso por su cuenta, con `WIFSIGNALED` vemos si le mandaron alguna señal como `sigkill`, si se da el caso, devuelve la señal que se envío al proceso pero sumando 128 (es la convención de shell).

Ahora para terminar faltaría hacer la documentación para la cual usaré el CHANGELOG.md para que la IA no alucine a la hora de redactar como ha sido el desarrollo de sandbox.