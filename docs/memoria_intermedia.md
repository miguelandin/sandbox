# Memoria intermedia — Proyecto _sandbox_

|                                |                                                                                                                                                                                                                                   |
| ------------------------------ | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Proyecto**                   | _sandbox_ — ejecución y monitorización de binarios en un entorno controlado (CLI, Linux)                                                                                                                                          |
| **Autor**                      | miguelandin                                                                                                                                                                                                                       |
| **Repositorio**                | https://github.com/miguelandin/sandbox                                                                                                                                                                                            |
| **Documentación del proyecto** | [`docs/definition.md`](https://github.com/miguelandin/sandbox/blob/main/docs/definition.md) (entrega preliminar) · [`docs/CHANGELOG.md`](https://github.com/miguelandin/sandbox/blob/main/docs/CHANGELOG.md) (diario de sesiones) |
| **Demostración**               | [`assets/setup-and-usage.gif`](https://github.com/miguelandin/sandbox/blob/main/assets/setup-and-usage.gif) (enlazado también desde el [README](https://github.com/miguelandin/sandbox/blob/main/README.md))                      |
| **Fecha de la memoria**        | 25 de octubre de 2026                                                                                                                                                                                                             |
| **Estado global**              | Fase 1 (motor de intercepción) **completada en plazo**. Siguiente paso: refactorización + Fase 2                                                                                                                                  |

---

## 1. Resumen

_Sandbox_ es una herramienta de línea de comandos para Linux, escrita íntegramente en C++, que lanza un binario sospechoso como proceso hijo y vigila lo que hace. A fecha de esta entrega, la **Fase 1 (motor de intercepción) está completa**: la herramienta crea el proceso con `fork()` + `execve()`, se engancha a él con `ptrace`, intercepta cada llamada al sistema y traduce las más relevantes para el análisis (apertura de ficheros, conexiones de red y ejecución de binarios), llegando incluso a extraer de la memoria del proceso hijo la ruta de cada fichero que abre.

Todo el código del repositorio ha sido programado a mano por el alumno. El único fragmento generado por IA que ha existido en el proyecto (un esqueleto inicial de `fork`/`execve`/`waitpid` producido con Gemini antes de arrancar la Fase 1) fue evaluado, descartado por completo y reescrito desde cero; en el apartado 4.4 se documenta exactamente qué tenía mal. La IA (Gemini en un ~95 % de las consultas, Claude en el ~5 % restante) ha actuado como tutor técnico para entender las APIs de bajo nivel del kernel, nunca como generador del código entregado.

---

## 2. Estado del desarrollo

Tabla de funcionalidades según se plantearon en la entrega preliminar (`docs/definition.md`). La columna **Autoría** distingue entre desarrollo propio del alumno, generación con apoyo de IA o cobertura mediante software existente.

| #   | Funcionalidad planteada                                                                                      | Fase                 | Estado                                                                                                   | Autoría                                                                               |
| --- | ------------------------------------------------------------------------------------------------------------ | -------------------- | -------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------- |
| 1   | Estructura del proyecto (`src/`, `include/`, `obj/`, `bin/`), `Makefile` con reglas estrictas y `.gitignore` | 0                    | ✅ Completada                                                                                             | Alumno                                                                                |
| 2   | Creación del proceso hijo con `fork()` + `execve()` y entorno controlado (`envp` con `PATH` y `HOME=/tmp`)   | 1                    | ✅ Completada                                                                                             | Alumno (el primer esqueleto, generado con IA, se descartó y se reescribió; ver 4.4)   |
| 3   | Parseo de flags (`-i`, `-r`, `-w`, `-x`, `-S`) desde la línea de comandos                                    | 1                    | 🟡 Parcial: las flags se reconocen y se separan correctamente del binario, pero todavía no tienen efecto | Alumno                                                                                |
| 4   | Enganche al proceso hijo con `ptrace` y sincronización padre/hijo sin condiciones de carrera                 | 1                    | ✅ Completada                                                                                             | Alumno                                                                                |
| 5   | Bucle de intercepción de llamadas al sistema (`PTRACE_SYSCALL` + `waitpid`)                                  | 1                    | ✅ Completada                                                                                             | Alumno                                                                                |
| 6   | Traducción e identificación de syscalls (ficheros, red, ejecución)                                           | 1                    | ✅ Completada (`open`, `openat`, `connect`, `execve`)                                                     | Alumno                                                                                |
| 7   | Lectura de cadenas desde la memoria del hijo (`PTRACE_PEEKDATA`) para mostrar rutas de ficheros              | 1                    | ✅ Completada                                                                                             | Alumno, a partir del análisis línea a línea de un ejemplo público de internet (ver 8) |
| 8   | Aislamiento con Linux Namespaces (`CLONE_NEWNET`, sistema de archivos)                                       | 2                    | ⏳ Pendiente                                                                                              | —                                                                                     |
| 9   | Filtrado a nivel de kernel con `seccomp-bpf` (bloquear/matar al proceso)                                     | 2                    | ⏳ Pendiente                                                                                              | —                                                                                     |
| 10  | Aplicación real de los permisos de las flags sobre el proceso                                                | 2                    | ⏳ Pendiente (depende de 8 y 9)                                                                           | —                                                                                     |
| 11  | Fichero de configuración en `/etc/sandbox/` (listas de IPs y rutas)                                          | 2 (opcional)         | ⏳ Pendiente                                                                                              | —                                                                                     |
| 12  | Limitación de CPU/RAM con `cgroups`                                                                          | 2 (opcional)         | ⏳ Pendiente                                                                                              | —                                                                                     |
| 13  | Logs estructurados e integración con `journalctl`                                                            | 3                    | ⏳ Pendiente                                                                                              | —                                                                                     |
| 14  | "Malware inofensivo" de prueba en C++ y validación de la herramienta contra él                               | 4                    | ⏳ Pendiente                                                                                              | —                                                                                     |
| 15  | `README.md` con instrucciones de instalación y uso                                                           | 5                    | ✅ Completada (adelantada; incluye gif de demostración)                                                   | Alumno                                                                                |
| 16  | Gestión de errores (p. ej. comprobación de privilegios de root) y memoria final                              | 5                    | ⏳ Pendiente                                                                                              | —                                                                                     |
| 17  | Publicación del paquete en el AUR de Arch Linux (`PKGBUILD` + `.SRCINFO`)                                    | Extra (post-entrega) | ⏳ Pendiente (nuevo, ver apartados 6 y 7)                                                                 | —                                                                                     |

> **Software existente utilizado**: únicamente las herramientas de compilación (`g++`, `make`) y las APIs nativas del sistema (biblioteca estándar de C++, API POSIX y la interfaz `ptrace` del kernel). No se ha incorporado ninguna biblioteca ni herramienta de terceros para cubrir funcionalidad del proyecto.

---

## 3. Arquitectura actual del código

Hoy toda la lógica vive en `src/main.cpp` (unas 130 líneas). Es deliberado: la Fase 1 se ha centrado en **entender y hacer funcionar** la cadena `fork` → `SIGSTOP` → `PTRACE_SEIZE` → bucle de intercepción → `PTRACE_GETREGS` → `PTRACE_PEEKDATA`, y una vez validada esa cadena es cuando toca estructurarla (ver apartado 6, hito intermedio de refactorización).

### 3.1 Creación del proceso y sincronización con el padre

El hijo se detiene a sí mismo con `raise(SIGSTOP)` **antes** de ejecutar `execve`. Así el padre tiene garantizado que el hijo sigue siendo el programa _sandbox_ (y no el binario sospechoso) cuando se engancha a él. Sin esto habría una condición de carrera: el binario podría ejecutar sus primeras syscalls antes de que `ptrace` estuviera activo.

```cpp
pid_t pid = fork();

if (pid == 0) {
  raise(SIGSTOP);                 // wait for the parent to attach
  execve(argv[i], argv + i, envp);

  std::perror("execve");          // only reached if execve fails
  _exit(127);
}

int status;
waitpid(pid, &status, WUNTRACED); // block until the child is stopped
if (!WIFSTOPPED(status)) {
  std::cerr << "Child did not stop succesfully" << std::endl;
}
```

Se evaluó la alternativa de sincronizar con un `pipe()`; se optó por `SIGSTOP` por simplicidad, legibilidad y porque encaja de forma natural con `waitpid(..., WUNTRACED)`.

### 3.2 Enganche con `PTRACE_SEIZE`

Se usa `PTRACE_SEIZE` (disponible desde Linux 3.4) en lugar del clásico `PTRACE_TRACEME`/`PTRACE_ATTACH`, porque no inyecta una parada adicional y permite pasar las opciones en la propia llamada. Dos opciones son clave:

- `PTRACE_O_TRACESYSGOOD`: hace que las paradas por syscall lleguen como `SIGTRAP | 0x80`, lo que permite distinguirlas de un `SIGTRAP` normal.
- `PTRACE_O_TRACEEXEC`: notifica el `execve` del binario.

```cpp
int options = PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEEXEC;
ptrace(PTRACE_SEIZE, pid, nullptr, options);
ptrace(PTRACE_SYSCALL, pid, nullptr, nullptr); // resume until next syscall
```

### 3.3 Bucle de intercepción

El bucle **no itera continuamente**: `waitpid` bloquea hasta que el hijo cambia de estado, y `ptrace(PTRACE_SYSCALL, ...)` al final de cada vuelta lo reanuda hasta la siguiente syscall. Cada llamada al sistema provoca **dos paradas** (entrada y salida), por lo que se usa un flag `manage` que alterna en cada parada para procesar únicamente la entrada y eliminar el ruido duplicado.

```cpp
bool manage = true;
while (true) { // Interception loop
  waitpid(pid, &status, 0);

  if (WIFEXITED(status) || WIFSIGNALED(status)) {
    break;                          // child finished
  }
  if (!WIFSTOPPED(status)) {
    continue;
  }

  int signal = WSTOPSIG(status);

  if (signal == (SIGTRAP | 0x80)) { // syscall stop (thanks to TRACESYSGOOD)
    if (manage) {
      struct user_regs_struct regs;
      ptrace(PTRACE_GETREGS, pid, nullptr, &regs);
      auto syscall_id = regs.orig_rax;

      switch (syscall_id) {
      case SYS_open:
        std::cout << "FILE OPENED " << syscall_id << std::endl;
        std::cout << read_string_from_process(pid, regs.rdi) << std::endl;
        break;
      case SYS_openat:
        std::cout << "FILE OPENED " << syscall_id << std::endl;
        std::cout << read_string_from_process(pid, regs.rsi) << std::endl;
        break;
      case SYS_connect:
        std::cout << "NETWORK CONNECTION " << syscall_id << std::endl;
        break;
      case SYS_execve:
        std::cout << "BINARY EXECUTION " << syscall_id << std::endl;
        break;
      default:
        break;
      }
    }
    manage = !manage;               // enter / exit toggle
  }

  ptrace(PTRACE_SYSCALL, pid, nullptr, nullptr);
}
```

`regs.orig_rax` contiene el número de la syscall (en x86_64 se guarda ahí porque `rax` se sobrescribe con el valor de retorno). Los argumentos siguen el convenio de llamada del kernel: `rdi`, `rsi`, `rdx`... De ahí que para `open` la ruta esté en `rdi` y para `openat` (cuyo primer argumento es el descriptor del directorio) esté en `rsi`.

### 3.4 Lectura de cadenas desde la memoria del hijo

`PTRACE_PEEKDATA` solo devuelve una palabra de máquina (8 bytes en x86_64) por llamada, y la dirección debe estar alineada. La función alinea la dirección de inicio, lee palabra a palabra, desempaqueta los bytes y se detiene al encontrar el `'\0'` del C-string, al producirse un error (`errno`) o al superar `PATH_MAX` (protección contra bucles infinitos).

```cpp
std::string read_string_from_process(pid_t pid, unsigned long address,
                                     std::size_t max_len = PATH_MAX) {
  constexpr std::size_t word_size = sizeof(long);

  std::size_t offset = address % word_size; // alignment correction
  auto addr = address - offset;

  std::string result;
  while (true) {
    errno = 0;
    long word = ptrace(PTRACE_PEEKDATA, pid,
                       reinterpret_cast<void *>(addr), nullptr);
    if (errno != 0) {
      return "";
    }

    const char *bytes = reinterpret_cast<const char *>(&word);
    for (std::size_t i = offset; i < word_size; ++i) {
      if (bytes[i] == '\0') {
        return result;
      }
      if (result.size() == max_len) {
        return "";
      }
      result += bytes[i];
    }
    offset = 0;
    addr += word_size;
  }
}
```

---

## 4. Pruebas de las funcionalidades desarrolladas

### 4.1 Compilación

El proyecto compila limpio con el `Makefile` del repositorio, usando flags estrictas (`-O3 -std=c++23 -Wall -Wextra -Iinclude`). La secuencia completa (`make` y ejecución) está grabada en el gif de demostración: [`assets/setup-and-usage.gif`](https://github.com/miguelandin/sandbox/blob/main/assets/setup-and-usage.gif).

```
❯ make
mkdir -p obj
g++ -O3 -std=c++23 -Wall -Wextra -Iinclude -c src/main.cpp -o obj/main.o
```

### 4.2 Prueba principal: intercepción de `/bin/ls`

Se ejecuta la herramienta sobre un binario real del sistema. Salida obtenida (transcrita del gif):

```
❯ ./bin/sandbox /bin/ls
BINARY EXECUTION 59
FILE OPENED 257
/etc/ld.so.cache
FILE OPENED 257
/usr/lib/libcap.so.2
FILE OPENED 257
/usr/lib/libc.so.6
FILE OPENED 257
.
CHANGELOG.md  LICENSE  Makefile  README.md  bin  obj  src
Child process terminated
```

Interpretación, contrastada con la tabla de syscalls de x86_64:

| Línea | Syscall | Nº | Qué demuestra |
|---|---|---|---|
| `BINARY EXECUTION 59` | `execve` | 59 | El enganche está activo **antes** de que el binario arranque: se captura su propio `execve`. |
| `FILE OPENED 257` + `/etc/ld.so.cache` | `openat` | 257 | El cargador dinámico abre la caché de bibliotecas. La ruta se ha extraído correctamente de la memoria del hijo. |
| `/usr/lib/libcap.so.2`, `/usr/lib/libc.so.6` | `openat` | 257 | Se detectan las bibliotecas compartidas que `ls` carga. |
| `.` | `openat` | 257 | `ls` abre el directorio actual para listarlo. |
| `Child process terminated` | — | — | El bucle detecta `WIFEXITED` y termina limpiamente; la salida de `ls` llega intacta al terminal. |

Resultado: **correcto**. Cada syscall aparece una sola vez (sin el duplicado entrada/salida), con su número correcto y, en el caso de los ficheros, con la ruta completa que recibe el kernel.

### 4.3 Comportamientos incorrectos detectados durante la Fase 1 y correcciones aplicadas

Estos problemas surgieron al ejecutar versiones intermedias del código (no de código generado por IA, sino del propio desarrollo) y se resolvieron iterando:

| Problema observado | Causa | Corrección |
|---|---|---|
| Al principio la salida era "desconcertante": aparecían paradas y señales que no se correspondían con syscalls. | Sin `PTRACE_O_TRACESYSGOOD`, una parada por syscall y un `SIGTRAP` ordinario son indistinguibles. | Activar `TRACESYSGOOD` y filtrar por `signal == (SIGTRAP \| 0x80)`; las paradas no reconocidas se ignoran (`default` vacío). |
| **Todas las syscalls se imprimían dos veces.** | `PTRACE_SYSCALL` detiene al hijo a la entrada **y** a la salida de cada syscall. | Flag `manage` que alterna en cada parada, procesando solo la entrada. |
| El proceso hijo se quedaba sin interceptar tras el primer `waitpid`. | Hacía falta un `PTRACE_SYSCALL` inicial (antes del bucle) además del que reanuda al final de cada iteración. | Doble `PTRACE_SYSCALL`: uno tras el `SEIZE` y otro al cierre de cada vuelta. |
| Cadenas corruptas al leer rutas: la dirección de inicio parecía "incorrecta". | `PEEKDATA` lee palabras de 8 bytes alineadas; la dirección del string rara vez está alineada. | Calcular `offset = address % word_size` y empezar a copiar desde ese byte dentro de la primera palabra. |
| Al abrir un fichero por ruta relativa (`file.txt`) se muestra solo el nombre, no la ruta absoluta. | `openat` recibe exactamente la cadena que pasó el programa; resolverla exige combinar `dirfd` (o `AT_FDCWD`) con el directorio de trabajo del hijo. | **Pendiente** (limitación conocida; se abordará en la refactorización / Fase 2). |

### 4.4 Validación del código generado por IA

El único código generado con IA que ha existido en el proyecto es el **`src/main.cpp` del commit `754040c` (28/09/2026)**, un esqueleto pedido a Gemini para ver cómo se creaba un proceso hijo: `fork()`, `execve()` y un `waitpid()` que informaba del código de salida. Se puede consultar íntegro en el historial del repositorio (`git show 754040c:src/main.cpp`).

Al revisarlo con criterio propio antes de arrancar la Fase 1 se detectaron estos problemas:

| Defecto del código generado | Por qué es un problema | Corrección en la versión manual |
|---|---|---|
| `execve(argv[1], argv + 1, nullptr)`: entorno **vacío** para el hijo. | El binario no tiene `PATH` ni `HOME`; muchos programas fallan o se comportan de forma distinta a la real. Además impide controlar qué entorno ve el proceso. | Se construye un `envp` explícito y controlado: `PATH` heredado y `HOME=/tmp`. |
| Si `execve` falla, el hijo imprime un error **y continúa ejecutando el resto de `main`** (no hay `_exit`). | Quedan dos copias del programa corriendo; el hijo acaba ejecutando código del padre. | `std::perror("execve")` + `_exit(127)` (el código de salida convencional para "no se pudo ejecutar"). |
| No contempla flags: asume que `argv[1]` es siempre el binario. | Incompatible con la interfaz definida (`sandbox -flags ./binario`). | Bucle que salta los argumentos que empiezan por `-` y toma el primero que no lo hace como binario. |
| Sin ninguna preparación para `ptrace` (sin parada previa del hijo, `waitpid` sin `WUNTRACED`). | La sincronización padre/hijo es la base de todo el motor y el esqueleto no la facilitaba. | Diseño propio: `raise(SIGSTOP)` en el hijo y `waitpid(..., WUNTRACED)` en el padre. |
| Mensajes decorativos (`[Sandbox] initializing...`) y comprobaciones redundantes que no aportan. | Ruido que dificulta leer el código y la salida. | Eliminados. |

Decisión: **descartar el fichero completo y reescribirlo a mano** (commit `44cfb5a`, 21/10/2026, "manage flags && not AI crap"). Desde ese momento no ha vuelto a entrar código generado por IA en el repositorio.

### 4.5 Validación de las explicaciones de la IA

Como la IA se ha usado como tutor y no como generador, la validación se ha hecho sobre sus **explicaciones**, contrastándolas con documentación (`man ptrace`, `man waitpid`, cabeceras del sistema) y, sobre todo, ejecutando. Casos relevantes (el listado completo de consultas está en el apartado 8):

- **Tipo de `orig_rax`**: la IA trabajaba con `long`; el alumno comprobó en `<sys/user.h>` que `user_regs_struct::orig_rax` es `unsigned long long` y adoptó `auto syscall_id = regs.orig_rax;` para no depender del tipo.
- **`PTRACE_SEIZE` vs. enganche clásico**: fue el alumno, a partir de la documentación online, quien planteó usar `PTRACE_SEIZE` (Linux ≥ 3.4) y lo contrastó con la IA antes de adoptarlo.
- **Estilo impuesto por la IA**: insistía en `for(;;)` y en patrones menos legibles; se cuestionó cada uno y se mantuvo `while (true)` por legibilidad. Igualmente se sustituyeron `#define` por `constexpr` y se justificó el uso de `std::size_t`.
- **Explicación confusa de la relación `ptrace`/`waitpid`**: hicieron falta varias rondas de preguntas hasta tener un modelo mental correcto (el bucle no itera: avanza cuando `waitpid` recibe un cambio de estado). El modelo final se validó con la ejecución del apartado 4.2.
- **Duplicado entrada/salida de syscalls**: se pidió a la IA que explicara el fenómeno **sin mostrar código**, y la solución (el toggle) se diseñó e implementó a mano.

### 4.6 Limitaciones conocidas a cierre de la Fase 1

- Solo se traducen cuatro syscalls (`open`, `openat`, `connect`, `execve`); el resto se ignoran silenciosamente.
- Para `connect` y `execve` todavía no se extraen los argumentos (dirección IP / ruta del binario).
- Las rutas relativas no se resuelven a absolutas (ver 4.3).
- La herramienta **observa pero no bloquea**: el bloqueo llega con `seccomp` en la Fase 2.
- Las flags se reconocen pero no se almacenan ni aplican.

---

## 5. Grado de avance respecto al planteamiento inicial y a los hitos

| Hito (fechas corregidas) | Previsto | Real (según commits) | Estado |
|---|---|---|---|
| Fase 1 · `fork()` + `execve()` | 15/10 – 18/10 | Base ya existente desde el 28/09; reescrito a mano el 21/10 | ✅ |
| Fase 1 · Integración de `ptrace` | 19/10 – 22/10 | 21/10 (`e3b2d50`, "ptrace implemented") | ✅ En plazo |
| Fase 1 · Traducción de syscalls | 23/10 – 26/10 | 25/10 (`99bbe84`, "primitive version of reading system calls && reading routes") | ✅ En plazo |
| README con instalación y uso (Fase 5) | 24/11 – 26/11 | 25/10 (`cb08e98`) | ✅ Adelantado |
| Fase 2 · Namespaces | 27/10 – 30/10 | — | ⏳ Comienza según calendario |

**Avance**: la Fase 1 se ha completado íntegramente dentro de su ventana (cierre previsto el 26/10) y se ha adelantado una tarea de la Fase 5 (el README). No hay funcionalidad prevista para la Fase 1 que quede sin hacer.

**Desviaciones y su motivo**:

1. **Corrección del calendario de hitos.** La primera versión de la definición del proyecto tenía unas fechas de hitos erróneas que se extendían más allá de la fecha de entrega (29/11). Se corrigieron en `docs/definition.md` y, desde entonces, el ritmo de trabajo se ajusta a las nuevas fechas. Por ese motivo el diario `docs/CHANGELOG.md` muestra un aparente "salto" de un mes: sus entradas fueron fechadas con el calendario antiguo. Las fechas reales de cada sesión son las de los commits correspondientes (28/09, 21/10 y 25/10).
2. **Refactorización no prevista inicialmente.** El `main.cpp` ha crecido hasta ser difícil de leer para alguien ajeno al proyecto. Antes de añadir namespaces y seccomp conviene reestructurarlo; se añade como hito intermedio (apartado 6) sin desplazar los demás.
3. **Alcance de la traducción de syscalls.** El hito preveía "abrir archivos o iniciar conexiones"; se cubren ambos, pero la extracción de argumentos de `connect` queda para cuando se trabaje el aislamiento de red en la Fase 2, donde tendrá sentido junto a las listas de IPs.

---

## 6. Planificación actualizada de los hitos restantes

Las fechas de la entrega preliminar **se mantienen**: el proyecto va al ritmo previsto. Se añade un hito intermedio de refactorización al arranque de la Fase 2 (sin mover ningún otro hito) y se reserva una segunda pasada de refactorización en la Fase 3, cuando el sistema de logs obligue a cambiar la forma en que el motor comunica lo que detecta.

##### Fase 2: Aislamiento, seguridad y filtrado selectivo (27/10 – 07/11)
- **27/10 – 28/10 · Hito intermedio (nuevo): refactorización.**
  - Mover declaraciones a `include/`.
  - Crear un fichero de funciones auxiliares (lectura de memoria del hijo, utilidades de `waitpid`).
  - Encapsular el motor en una clase `tracer`, de modo que `main` quede reducido a parsear argumentos y llamar a `tracer.run()`.
  - Hacer que `run()` devuelva/emita eventos como strings en lugar de imprimir directamente, preparando el terreno para los logs de la Fase 3.
  - Guardar las flags parseadas en una estructura de configuración.
- **27/10 – 30/10**: Linux Namespaces (`CLONE_NEWNET` y aislamiento del sistema de archivos). Comienza en paralelo con el cierre de la refactorización.
- **31/10 – 03/11**: filtros `seccomp-bpf` que bloqueen o maten al hijo ante acciones no permitidas; conexión de las flags con los permisos reales.
- **04/11 – 07/11** _(opcional)_: fichero de configuración en `/etc/sandbox/` (listas de IPs/rutas), extracción de argumentos de `connect`, resolución de rutas relativas y `cgroups`.

##### Fase 3: Refinamiento de los logs (08/11 – 15/11)
- **08/11 – 15/11**: logs estructurados y envío a `journalctl`. Incluye la **segunda refactorización** prevista: separar la generación de eventos (motor) de su destino (consola, fichero, journal).

##### Fase 4: Pruebas de fuego y validación (16/11 – 23/11)
- **16/11 – 19/11**: programas C++ de "malware inofensivo" (lectura de ficheros sensibles, escritura, apertura de puertos, ejecución de otros binarios).
- **20/11 – 23/11**: ejecución de _sandbox_ contra ellos; verificación de detección, bloqueo y registro.

##### Fase 5: Documentación y entrega (24/11 – 29/11)
- **24/11 – 26/11**: pulido del código, gestión de errores (comprobación de privilegios de root, validación de argumentos), actualización del README.
- **27/11 – 29/11**: memoria final. **Entrega: 29/11/2026.**

##### Hito extra (post-entrega): publicación en el AUR de Arch Linux
- **A partir del 30/11, una vez terminado y entregado el proyecto.** No afecta a ninguna fecha anterior ni a la entrega.
- Objetivo: que _sandbox_ se pueda instalar en cualquier Arch Linux con un helper de AUR (`yay -S sandbox` o similar), cerrando el círculo de "herramienta CLI nativa de Linux".
- Tareas:
  - Etiquetar una versión estable en GitHub (`v1.0.0`) y generar el tarball de release.
  - Escribir el `PKGBUILD` (dependencias: `gcc`, `make`; `build()` con `make`, `package()` instalando `bin/sandbox` en `/usr/bin/` y, si existe, el fichero de configuración de ejemplo en `/etc/sandbox/`) y generar el `.SRCINFO` con `makepkg --printsrcinfo`.
  - Validar el paquete en limpio con `makepkg -si` y `namcap` antes de publicar.
  - Comprobar la disponibilidad del nombre en el AUR (si `sandbox` está ocupado, usar un nombre alternativo o la variante `-git`), crear la cuenta de AUR con clave SSH y hacer el primer `git push` al repositorio del paquete.
  - Actualizar el README con la instrucción de instalación desde el AUR.

---

## 7. Cambios respecto a la entrega preliminar

| Ámbito | Cambio | Justificación |
|---|---|---|
| **Planificación** | Fechas de hitos corregidas (la primera versión sobrepasaba la fecha de entrega). | Error de calendario detectado al revisar la definición; las nuevas fechas son las que se están cumpliendo. |
| **Planificación** | Nuevo hito intermedio de refactorización (27/10 – 28/10) y segunda refactorización dentro de la Fase 3. | El `main.cpp` monolítico ya es difícil de leer; añadir namespaces y seccomp encima lo haría inmantenible. No desplaza ningún hito. |
| **Alcance** | Todas las funcionalidades planteadas siguen vigentes. Se añade un hito extra, posterior a la entrega, para publicar la herramienta en el AUR de Arch Linux. | Es la forma natural de distribuir un comando nativo de Linux desarrollado en Arch, y en la definición ya se preveía la distribución del código vía GitHub. Al situarse después del 29/11 no compromete la entrega. |
| **Arquitectura** | De un `main.cpp` monolítico a cabeceras en `include/`, utilidades separadas y una clase `tracer` cuyo `run()` devuelve eventos en vez de imprimir. | Legibilidad, reutilización del motor desde los logs de la Fase 3 y facilidad de prueba. |
| **Tecnologías** | Sin cambios de base: C++, POSIX, `ptrace`, namespaces, `seccomp`. Decisión de detalle: usar `PTRACE_SEIZE` con `PTRACE_O_TRACESYSGOOD \| PTRACE_O_TRACEEXEC` en lugar de `PTRACE_TRACEME`, y sincronizar con `raise(SIGSTOP)` en vez de con un `pipe()`. | `SEIZE` evita paradas espurias y permite fijar opciones en el enganche; `SIGSTOP` es más simple y legible que un pipe para esta sincronización. |
| **Estrategia de IA** | Endurecida: a partir de ahora, al investigar una librería o API, se exige a la IA que muestre **solo código de ejemplo**, nunca la solución a implementar en el proyecto. | El único código generado con IA (28/09) resultó de baja calidad y hubo que descartarlo (4.4). Además, la IA tiende a empujar hacia su propio estilo (`for(;;)`, `long` genérico) y a dar explicaciones que necesitan varias rondas para ser precisas (4.5). Como tutor funciona bien; como programador, no aporta. |
| **Estrategia de IA** | Sin herramientas descartadas; Gemini sigue siendo el asistente principal y Claude entra de forma puntual (documentación). | No ha hecho falta cambiar de asistente, solo cambiar la forma de usarlo. |

---

## 8. Registro del uso de la IA en el desarrollo

### 8.1 Herramientas y agentes

| Herramienta | Peso aproximado | Uso |
|---|---|---|
| **Gemini** (chat) | ~95 % | Investigación de APIs de bajo nivel, resolución de dudas conceptuales, revisión de salidas de ejecución, explicación de código ajeno. |
| **Claude** (chat / proyecto) | ~5 % | Apoyo puntual de investigación y en la redacción de documentación (incluida esta memoria, elaborada a partir de `definition.md`, `CHANGELOG.md`, el historial de commits, el código del repositorio y el listado de consultas del alumno). |

No se ha utilizado ningún agente de desarrollo autónomo ni asistentes integrados en el editor para generar código.

### 8.2 Tareas y grado de automatización

| Tarea | Herramienta | Grado de automatización | Resultado |
|---|---|---|---|
| Esqueleto inicial `fork`/`execve`/`waitpid` (28/09) | Gemini | **Generación de código** (única vez) | Descartado y reescrito a mano (4.4). |
| Entender `ptrace`: `PTRACE_SEIZE`, opciones, `PTRACE_SYSCALL`, paradas | Gemini | Asistencia puntual (consulta conceptual, sin código de solución) | Implementado a mano. |
| Entender `waitpid`: parámetros, `WUNTRACED`, bloqueo, semántica de `pid` | Gemini | Asistencia puntual | Implementado a mano. |
| Decidir la sincronización padre/hijo (`pipe()` vs `raise(SIGSTOP)`) | Gemini | Consulta comparativa (rendimiento, legibilidad, consistencia, seguridad); decisión del alumno | `SIGSTOP`. |
| Interpretar las salidas de ejecución (señales desconocidas, duplicados) | Gemini | Asistencia puntual, explícitamente **sin código** | Correcciones diseñadas por el alumno (4.3). |
| Leer registros (`user_regs_struct`, `orig_rax`) y mapear números de syscall | Gemini | Asistencia puntual | `switch` sobre `SYS_*` escrito a mano. |
| Entender un ejemplo público de lectura de memoria con `PEEKDATA` (alineación, `reinterpret_cast`, `constexpr`, `std::size_t`) | Gemini | Explicación línea a línea de código **de internet**, no generado por la IA | `read_string_from_process` escrita a mano adaptando la idea. |
| Dudas de C++ moderno (`constexpr` vs `static`, `reinterpret_cast` vs `static_cast`, `auto`) | Gemini | Asistencia puntual | Aplicado con criterio propio. |
| Redacción de documentación (esta memoria) | Claude | Asistencia en redacción a partir de material del alumno | Revisada y validada por el alumno. |

### 8.3 Listado de consultas realizadas

Las consultas a la IA durante la Fase 1, en orden cronológico y agrupadas por tema (el fichero íntegro se incluye en `docs/` del repositorio junto a esta memoria):

**Enganche con `ptrace` (19/10 – 22/10)**
- Cómo abordar la integración de `ptrace` en el padre, pidiendo explícitamente una base para empezar y **no el código**.
- Opinión sobre `PTRACE_SEIZE` (Linux ≥ 3.4) frente al comportamiento clásico de parada por señal.
- `pipe()` vs `raise(SIGSTOP)` para que el hijo espere al enganche: ventajas en rendimiento, legibilidad, consistencia y seguridad.
- Semántica de `waitpid`: si bloquea, parámetros, confusión con `pid == 0` en el hijo, efecto de `WUNTRACED`.
- Dónde colocar `raise(SIGSTOP)` respecto a `execve`, y cómo reanudar al hijo una vez enganchado.
- Uso de `PTRACE_SEIZE`; qué aporta `ptrace` dentro del bucle y su relación con `waitpid`; opciones del cuarto parámetro.
- Qué señal de `waitpid` comprobar para verificar que funciona con `ls`.
- Por qué un `while (true)`, por qué un `PTRACE_SYSCALL` al final del bucle si se usa `SEIZE`, cuándo se detiene el hijo, y por qué `waitpid` con `0` en vez de `WUNTRACED` dentro del bucle.

**Traducción de syscalls (25/10)**
- Explicación línea a línea de la lectura de registros; por qué `long`, `0x80` como `#define`, uso de `||` en la detección de apertura.
- Corrección del tipo de `orig_rax` (`unsigned long long`) y propuesta de `auto syscall_id = regs.orig_rax;`.
- `switch` vs `else if`; primera ejecución con salida desconcertante; dónde consultar la tabla de números de syscall.
- Qué señales eran las desconocidas en la salida de `ls`; a qué corresponde cada línea de la salida.
- Por qué cada syscall aparece dos veces y cómo evitarlo (**sin mostrar código**); por qué hacen falta dos `PTRACE_SYSCALL`.
- Revisión de una salida que parecía incorrecta; cómo saber qué bibliotecas abre el proceso.

**Lectura de memoria del hijo (25/10)**
- Explicación línea a línea de un ejemplo de internet de lectura de strings con `PEEKDATA`.
- `constexpr` vs `static`; `std::size_t` vs `size_t`; por qué la dirección inicial parece incorrecta (alineación); `for(;;)` vs `while(true)`; `reinterpret_cast` vs `static_cast`; límite de 8 bytes por `PEEKDATA`.
- Por qué `file.txt` aparece sin ruta absoluta y si se puede obtener siempre la ruta completa.
- Contenido de `user_regs_struct`.

---

## 9. Instalación y ejecución

**Requisitos**: Linux (desarrollado en Arch Linux, debería funcionar en cualquier distribución), `g++` con soporte C++23, `make`, cabeceras POSIX.

```bash
git clone https://github.com/miguelandin/sandbox.git
cd sandbox/
make
```

```bash
# From the root dir of the project
./bin/sandbox <flags> <bin_route>

# Example
./bin/sandbox /bin/ls
```

`make clean` elimina `obj/` y `bin/`; `make run ARGS="/bin/ls"` compila y ejecuta en un paso. El repositorio no contiene credenciales ni claves de ningún tipo (la herramienta es completamente local).

---

## Anexo A. Histórico de commits

| Fecha | Commit | Descripción |
|---|---|---|
| 27/09/2026 | `6495ff5`, `5f17150` | Commit inicial y licencia. |
| 28/09/2026 | `807e3a7` | `Makefile` y estructura básica del proyecto. |
| 28/09/2026 | `754040c` | `main.cpp` primitivo (esqueleto generado con Gemini, posteriormente descartado). |
| 15/10/2026 | `338f6bf` | README temporal con la definición del proyecto y los hitos. |
| 18/10/2026 | `32262fc` | Diario de sesiones (`CHANGELOG.md`). |
| 21/10/2026 | `44cfb5a` | Parseo de flags y reescritura manual completa del `main.cpp` ("not AI crap"). |
| 21/10/2026 | `e3b2d50` | `ptrace` implementado (hito 19/10 – 22/10). |
| 25/10/2026 | `99bbe84` | Lectura de syscalls y de rutas de ficheros (hito 23/10 – 26/10). |
| 25/10/2026 | `cb08e98` | Nuevo README de instalación y uso con gif de demostración; documentación movida a `docs/`. |
