Progreso del proyecto. A continuación se ve un diario apuntando todos los progresos en el proyecto siendo la última la más reciente.

27/10/26
Se ha creado la estructura básica del proyecto, con un .gitignore y Makefile. La estructura usa la estándar usando directorios `bin/` `obj/` `include/` `src/`.

28/10/26
Se ha configurado un .gitignore y un Makefile automatizado con reglas estrictas de compilación. Se ha iniciado la Fase 1 implementando el esqueleto de main.cpp. El proceso padre ya es capaz de usar `fork()` y `execve()` para ejecutar binarios, y se ha añadido el bucle inicial de intercepción con ptrace.

21/11/26
Se ha cambiado por completo el código original. Ahora `main.py` es mucho más simple y legible. También se ha creado una variable `*envp` para el `execve` la cual contiene el `$PATH` y el directorio `/tmp`.
Además, las flags de sandbox pueden ser depuradas correctmante para que en el futuro se puedan manejar.

Finalmente se ha investigado como funciona `ptrace` y como se complementa con `waitpid` para poder saber exactamente que acciones está haciendo el proceso hijo. Para poder implementado hay que evitar que se de lugar a una condición de carrera entre el padre e hijo. Por ello, antes de llamar a `execve` se le manda la señal `SIGSTOP` para parar el fork hijo antes de que ejecute el binario. Después de que el padre reciba la señal de SIGSTOP del hijo, este aplica el ptrace sobre el proceso hijo. Finalmente con el mismo ptrace le manda una señal para que continue el proceso de forma normal poniendo en el parámetro option=nullptr: `ptrace(PTRACE_SYSCALL, pid, nullptr , nullptr).

Con todo esto se ha dejado el bucle de intercepciones listo y comprobado para analizar el binario malicioso.
