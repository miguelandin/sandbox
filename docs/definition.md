### Definición general
La herramienta que se desarrollará (la cual de momento se llamará _sandbox_) en esta asignatura, será un programa que ejecutará binarios en un entorno controlado [sandbox](https://en.wikipedia.org/wiki/Sandbox_(computer_security))

Esta será una herramienta completamente `cli` (por lo que se usará en consola) y exclusiva de Linux.

_Sandbox_ ejecutará el binario en un proceso controlado restringiendo el acceso a internet, abrir, editar, crear archivos y demás. Si el proceso intenta hacer alguna acción restringida está será advertida en pantalla y se guardará en logs.

También se podrán usar flags para dar permisos al proceso:
- `-i`: acceso a internet.
- `-r`: lectura de archivos.
- `-w`: edición y creación de archivos.
- `-x`: ejecución de otros binarios.

Aunque se permita el acceso, de todas formas se guardará en los logs los usos de estos permisos. Para que no advierta de ciertos accesos habrá que usar la flag `-S` (silent).

---
### Listado y descripción de las investigaciones o desarrollos similares al planteado
A continuación se enumeraran varios desarrollos parecidos a la idea planteada:
1. **Herramientas de trazado([strace](https://strace.io/) o [gdb](https://en.wikipedia.org/wiki/GNU_Debugger))**: Son herramientas que utilizan la tecnología [ptrace](https://en.wikipedia.org/wiki/Ptrace) para interceptar llamadas al sistema. A diferencia de _sandbox_, estas herramientas están pensadas para depuración y no actúan como una capa de seguridad que bloquee acciones maliciosas. _Sandbox_ sustituye estas herramientas en el ámbito de la ciberseguridad.

2. **Tecnologías de contenedores**([Docker](https://en.wikipedia.org/wiki/Docker_(software))): Utiliza tecnologías de aislamiento como los [Linux Namespaces](https://en.wikipedia.org/wiki/Linux_namespaces). Mientras que Docker se usa para el despliegue de aplicaciones, _sandbox_ utilizará esta misma base tecnológica para crear un _sandbox_ de análisis de malware.

3. **Herramientas defensivas (Antivirus)**: Son soluciones completas que monitorizan el sistema. _Sandbox_ se podría usar de forma complementaria, presentándose como una utilidad de línea de comandos ágil para aislar y probar binarios específicos de forma controlada. O incluso podría ser una alternativa a los propios antivirus si buscas estar seguro de que los binarios que instalas son seguros pero sin la necesidad de tener un antivirus constantemente corriendo por detrás en tu sistema.

---
### Tipo de planteamiento elegido
- El desarrollo consistirá en una herramienta convencional desarrollada con el apoyo de IA.
- La IA aportará valor ayudando en la investigación y documentación técnica necesaria.
- Su uso se justifica por la alta complejidad de entender e implementar características del kernel de Linux además de la gran utilidad a la hora redactar documentos de forma profesional.
---
### Estrategia de uso de la IA en el desarrollo
- **Asistentes**: se utilizarán LLMs, en este caso Gemini, como apoyo durante el proceso de **investigación**.

- **Flujo de trabajo**:  La IA intervendrá en la comprensión de las APIs de bajo nivel (como la [POSIX](https://en.wikipedia.org/wiki/POSIX) API, `ptrace` o [seccomp](https://en.wikipedia.org/wiki/Seccomp)) y en la estructuración de la arquitectura. **Todo el código `C++` será programado a mano por el alumno**.

- **Validación**: El código creado y la configuración del entorno se validarán programando pequeños programas en C++ (malware inofensivo) que intenten realizar acciones prohibidas para comprobar que el _sandbox_ los detecta y bloquea correctamente. Más tarde se podría testear la herramienta con malware real obtenido de herramientas de Kali o internet.
---
### En caso de integrar la IA en la herramienta
No aplica. La herramienta final será  un comando de Linux programado a mano en C++ y no integrará IA en su funcionamiento interno. El único caso en el que podría integrarse la IA sería si una vez terminada la herramienta base, se quiera integrar una IA que lea los logs producidos y te haga un análisis de lo sucedido, pero como digo, la herramienta base, que es el comando, no integrará absolutamente nada de código generado por IA.

---
### Diagrama de flujo que seguiría la herramienta
1. **Inicio y configuración**: El usuario ejecuta por consola el comando con el siguiente formato:
```bash
sandbox -flags ./binario
```

(por ejemplo, con flags como `-i`, `-r`, `-w`, `-x` para conceder permisos o `-S` para activar el modo silencioso).

Veamos un ejemplo usando las flags:
```bash
sandbox -i ./binario
```

En este caso la herramienta está otorgando acceso a internet y además puede leer un archivo de configuración opcional (que estará como siempre se hace en `/etc/sandbox/`) el cual contiene reglas específicas como una lista de IPs bloqueadas, o lo contrario, una whitelist de IPs permitidas.

También se podrían poner normas de otros tipos, no solo de internet, como listas de archivos y directorios permitidos y demás.

2. **Creación de proceso**: El proceso padre (la herramienta _sandbox_) utiliza `fork()` y `execve()` para ejecutar el binario sospechoso como proceso hijo.

3. **Aislamiento y Limitación**: Se aplican Linux Namespaces para aislar recursos (red, sistema de archivos). De forma opcional, se usarán `cgroups` para limitar el consumo de RAM y CPU.

4. **Intercepción**: El proceso padre utiliza `ptrace` para interceptar las llamadas al sistema (`syscalls`) del proceso hijo.

5. **Filtrado**: Mediante reglas de `seccomp`, el kernel bloquea o mata al proceso si intenta ejecutar acciones no permitidas.

6. **Registro (logging)**: Los intentos de acciones bloqueadas se notifican y se guardan en un log nativo que puede integrarse con `journalctl`.

7. **Intervención de la IA**: No hay intervención de la IA en tiempo de ejecución.

---
### Planteamiento de hitos y estructura
##### Fase 1: Motor de intercepción (15/10/26 – 26/10/26)
- **15/10 – 18/10:** Implementación de la creación de procesos en Linux usando las llamadas `fork()` y `execve()`. El objetivo es que la herramienta pueda recibir un comando por consola y ejecutar el binario sospechoso como proceso hijo.

- **19/10 – 22/10**: Integración de la tecnología `ptrace` en el proceso padre para engancharse al hijo e interceptar sus llamadas al sistema (`syscalls`).

- **23/10 – 26/10**: Traducción e identificación de las `syscalls` interceptadas. El objetivo es que el programa detecte e imprima cuándo el binario intenta realizar acciones específicas, como abrir archivos o iniciar conexiones.
##### Fase 2: Aislamiento, seguridad y filtrado selectivo (27/10/26 – 07/11/26)
- **27/10 – 30/10**: implementación de _Linux Namespaces_ para aislar los recursos del proceso. Se aplicará aislamiento de red (ej. `CLONE_NEWNET`) y del sistema de archivos para que el binario corra en un entorno restringido.

- **31/10 – 03/11**: Integración de filtros de seguridad mediante reglas `seccomp-bpf`. Se creará un filtro estricto a nivel de kernel que bloquee o mate al proceso hijo inmediatamente si intenta ejecutar acciones destructivas no permitidas.

- **04/11 – 07/11** _(Hito opcional y refinamiento)_: Pruebas de integración de la fase. Si el tiempo lo permite:
  - Lectura de archivos de configuración para la lógica de filtrado selectivo de IPs, interceptando las llamadas de red.
  - Limitación del consumo de RAM y CPU del proceso hijo mediante `cgroups`.
##### Fase 3: Refinamiento de los Logs (08/11/26 – 15/11/26)
- **08/11 – 15/11**: Modificación del código en C++ para estructurar las notificaciones de los bloqueos. Las acciones restringidas detectadas dejarán de imprimirse únicamente por consola y se enviarán de manera limpia y estructurada al sistema o a `journalctl`.
##### Fase 4: Pruebas de fuego y validación (16/11/26 – 23/11/26)
- **16/11 – 19/11**: Desarrollo de pequeños scripts o programas C++ que actúen como "malware inofensivo", los cuales intentarán de forma deliberada realizar acciones prohibidas (como leer contraseñas, editar archivos, abrir puertos...).

- **20/11 – 23/11**: Ejecución de la herramienta contra estos programas maliciosos para validar de forma práctica que el _sandbox_ los detecta, bloquea las acciones correctamente y lo registra en los logs.
##### Fase 5: Documentación y Entrega (24/11/26 – 29/11/26)
- **24/11 – 26/11**: Pulido final del código, creación del archivo `README.md` y gestión de errores (por ejemplo, que la herramienta verifique si el usuario tiene los privilegios de root necesarios para ejecutarse o leer en `/etc/`).

- **27/11 – 29/11**: Redacción de la memoria final del proyecto, documentando la arquitectura y resultados para su entrega en la asignatura. **Entrega: 29/11/26.**
---
### Detalles de las tecnologías y lenguajes
- El lenguaje de programación utilizado será exclusivamente C++ y se usarán las librerías estándar (`std`) y las librerías de la API POSIX.
- Se utilizarán tecnologías nativas de Linux: API POSIX, `ptrace`, `seccomp` y Linux Namespaces, y de forma opcional `cgroups`.
- Se usará Github para la subida de código, control de versiones y distribución del código en [este repositorio](https://github.com/miguelandin/sandbox.git).

---
### Hardware necesario
- Máquina física o virtual con sistema operativo Linux. En este caso se usará un Arch Linux, aunque debería de funcionar en cualquier Linux.
- Al ser la herramienta totalmente local, no se incurre en costes de uso de APIs para su funcionamiento y la IA de apoyo puede utilizarse de forma externa y gratuita a través de asistentes estándar.
