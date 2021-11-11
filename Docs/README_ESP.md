---
author: Rick de Bondt
fontfamily: dejavu
fontsize: 8pt
geometry: margin=3cm
urlcolor: blue
output: pdf_document
---
# XLink Handheld Assistant - Guía de configuración y uso
Para XLink Handheld Assistant, hay algunos modos en los que el asistente se puede ejecutar, tales como:

- Modo Monitor (sólo en Linux)
- Modo Plugin (sólo para PSP)

Estos modos tienen sus propias ventajas y desventajas que se explicarán a continuación.

- Modo Monitor: Sólo se puede usar en Linux, y generalmente es menos seguro que el modo Plugin, su eficacia depende en gran medida a la tarjeta WiFi utilizada y de lo ocupado que esté el espacio del WiFi en general. La ventaja del modo Monitor, es que también funciona en juegos de Playstation Vita y, con suerte, también con más dispositivos en el futuro.
- Modo Plugin: Se puede usar tanto en Windows como en Linux, sin embargo, sólo funciona en juegos de PSP (PSP o Vita con Adrenaline), y se necesita habilitar un plugin en la consola respectiva, por lo que esta misma debe modificarse (CFW / HEN).

\
Realice alguna de las siguientes guías, según el método y el sistema operativo de su preferencia:

- [Linux - Modo Monitor][]
- [Linux - Modo Plugin][]
- [Windows - Modo Plugin][]

Para obtener una descripción general de todas las funciones de XLink Handheld Assistant, consulte [Descripción general][].

\newpage
## Linux - Modo Monitor
1. Configure la PSP en el canal 1, yendo a: \
   Configuración -> Configuración de red -> Modo Ad-Hoc -> Canal 1.

2. Inicie un juego en la consola.

3. Ejecute lo siguiente para configurar la interfaz WiFi en el modo Monitor. \
\
**NOTA**: Esto deshabilita NetworkManager, si usted usa otra conexión WiFi para conectarse a Internet, sería mejor que NetworkManager ignore el adaptador WiFi usado para XLHA, y edite el script "set_interface_monitor" para no deshabilitar NetworkManager. \


    ```bash
    sudo chmod 775 ./linux_scripts/set_interface_monitor.sh \
    && ./linux_scripts/set_interface_monitor.sh "Nombre del adaptador WiFi"
    ```

4. Inicie [XLink Kai](http://teamxlink.co.uk/) en su ordenador.
    1. Siga los pasos de [XLink Kai Guía para Debian](https://repo.teamxlink.co.uk/) u obtenga el código binario desde la [página de descarga](https://www.teamxlink.co.uk/go?c=download), después de descargar (solo cuando no esté usando la guía para Debian): 
    
       ```bash
       tar xvf kaiEngine-*.tar.gz \
       && cd "$(ls -f | grep kaiEngine | grep -v *.tar.gz)" \
       && sudo chmod 755 ./kaiengine \
       && sudo setcap cap_net_admin,cap_net_raw=eip ./kaiengine \
       && ./kaiengine
       ```
       
5. Inicie XLink Handheld Assistant 
   ```bash
   sudo chmod 755 ./xlinkhandheldassistant \
   && sudo ./xlinkhandheldassistant
   ```
   
   Disculpas por los permisos sudo requeridos en el modo monitor, pero es imposible hacer esto a menos que seas un usuario root, por lo que veo.
   
   Si recibe un error sobre el tamaño de la terminal en este punto, asegúrese de que su terminal tenga al menos un tamaño de 80x24. 
   Ejemplo: Tamaño de terminal demasiado pequeño: 46x18, asegúrese de que el terminal sea de al menos 80x24.
   
6. En el asistente, seleccione "Monitor Device" y luego use las flechas de su teclado, para bajar hasta el casillero "Next", y posteriormente seleccionada dicha opción, presione Enter.
7. Pulse la barra espaciadora de su teclado en "Automatically connect to PSP/Vita networks".
8. Baje hasta la opción "Acknowledge data packets", intente habilitar esta opción primero, pero deshabilítela si tiene algún problema. Si posee un chip WiFi de Intel, definitivamente deje esta opción desactivada.
9. Desplácese hacia abajo, hasta hallar el adaptador de red correcto, con las flechas de su teclado, y presione la barra espaciadora en el adaptador WiFi que se utilizará para XLHA.
10. Vaya al botón "Next" y presione Enter.
11. Presione Enter en el botón "Next" nuevamente.
12. En el Panel de Control, si está alojando una partida, presione la barra espaciadora en el apartado de "Hosting"; de lo contrario, diríjase al siguiente paso.
13. Presione Enter en "Start Engine".
14. Ingrese a la arena de XLink Kai en la que desee jugar, desde [WebUI](http://127.0.0.1:34522/)
15. Por el lado de la consola, ve al menú multijugador Ad-Hoc, y comienza a hospedar o unirte a una partida del juego de tu preferencia.
16. En la esquina superior derecha del Panel de Control, debería aparecer "Connected to PSP_GameID_...." después de 5-30 segundos.
17. En XLink Kai, haga clic en las métricas y desplácese hacia abajo, debería ver lo siguiente:
![](screenshots/XLinkMetrics.PNG){ width=100% }
(Hay una consola en XLHA, en el apartado de "Connected Aplications", el PSP aparece en el apartado de "Found Consoles" y usted deberá de apreciar "Broadcast Traffic out").
18. ¡Disfruta del juego!
19. Para cerrar XLHA, simplemente presione 'q' en la interfaz.
20. Ejecute el siguiente código para restaurar su tarjeta WiFi a su modo normal:

    ```bash
    sudo chmod 775 ./linux_scripts/restore_managed.sh \
    && ./linux_scripts/restore_managed.sh "Nombre del adaptador WiFi"
    ```

\
Si desea rehacer estos pasos o elegir otro método de conexión, vaya a "Options" -> "Reconfigure the application"

\newpage
## Linux - Modo Plugin
1. Copie [AdHocRedirectorWiFi.prx](./Plugin/AdHocRedirectorWiFi.prx) hacia la dirección /SEPLUGINS en la PSP.
2. Cree o abra el archivo /SEPLUGINS/GAME.TXT.
3. Añada: 

   ```ms0:/seplugins/AdHocRedirectorWiFi.prx 1``` (para PSP 1000/2000/3000) 

   o 

   ``` ef0:/seplugins/AdHocRedirectorWiFi.prx 1``` (para PSP GO)
\
   ```bash
   touch /media/username/nameofPSP/SEPLUGINS/GAME.TXT \
   && echo "ms0:/seplugins/AdHocRedirectorWiFi.prx 1" \
   >> /media/username/nameofPSP/SEPLUGINS/GAME.TXT
   ```
   
4. Inicie un juego en la consola.
5. Continúe con los siguiente pasos, para configurar la interfaz WiFi con el Ad-Hoc \
\
**NOTA:** Esto deshabilita NetworkManager, si usted usa otra conexión WiFi para conectarse a Internet, sería mejor hacer que NetworkManager ignore el adaptador WiFi usado para XLHA y edite el script "set_interface_bss" para no deshabilitar NetworkManager. \

    ```bash
    sudo chmod 775 ./linux_scripts/set_interface_bss.sh \
    && ./linux_scripts/set_interface_bss.sh "Nombre del adaptador WiFi"
    ```

6. Inicie [XLink Kai](http://teamxlink.co.uk/) en su ordenador
    1. Siga los pasos en [XLink Kai Guía para Debián](https://repo.teamxlink.co.uk/) u obtenga el código binario desde la [página de descarga](https://www.teamxlink.co.uk/go?c=download), después de descargar (solo cuando no esté usando la guía para Debian): 
    
       ```bash
       tar xvf kaiEngine-*.tar.gz \
       && cd "$(ls -f | grep kaiEngine | grep -v *.tar.gz)" \
       && sudo chmod 755 ./kaiengine \
       && sudo setcap cap_net_admin,cap_net_raw=eip ./kaiengine \
       && ./kaiengine
       ```
7. Inicie XLink Handheld Assistant 
   ```bash
   sudo chmod 755 ./xlinkhandheldassistant \
   && sudo setcap cap_net_admin,cap_net_raw=eip ./xlinkhandheldassistant \
   && ./xlinkhandheldassistant
   ```
   
   Si recibe un error sobre el tamaño de la terminal en este punto, asegúrese de que su terminal tenga al menos un tamaño de 80x24. 
   Ejemplo: tamaño de terminal demasiado pequeño: 46x18, asegúrese de que el terminal sea de al menos 80x24.
   
8. En el asistente, seleccione "Plugin Device" y luego use las flechas en su teclado, para bajar hasta el casillero "Next", y posteriormente seleccionada dicha opción, presione Enter.
9. Pulse la barra espaciadora de su teclado en "Automatically connect to PSP/Vita networks".
10. Desplácese hacia abajo, hasta hallar el adaptador de red correcto, con las flechas de su teclado, y presione la barra espaciadora en el adaptador WiFi que se utilizará para XLHA.
11. Vaya al botón "Next" y presione Enter.
12. Presione Enter en el botón "Next" nuevamente.
13. En la interfaz, si está alojando una partida, presione la barra espaciadora en el apartado de "Hosting"; de lo contrario, diríjase al siguiente paso.
14. Presiona la tecla Enter en "Start Engine".
15. Ingrese a la arena de XLink Kai en la que desee jugar, desde [WebUI](http://127.0.0.1:34522/)
16. Por el lado de la consola, ve al menú multijugador Ad-Hoc, y comienza a hospedar o unirte a una partida del juego de tu preferencia.
17. En la esquina superior derecha del Panel de Control, debería aparecer "Connected to PSP_GameID_...." después de 5-30 segundos.
18. En XLink Kai, haga clic en las métricas y desplácese hacia abajo, debería ver lo siguiente:
![](screenshots/XLinkMetrics.PNG){ width=100% }
(Hay una consola en XLHA, en el apartado de "Connected Aplications", el PSP aparece en el apartado de "Found Consoles" y usted deberá de apreciar "Broadcast Traffic out").
19. ¡Disfruta del juego!
20. Para cerrar XLHA, simplemente presione 'q' en la interfaz.
21. Ejecute el siguiente código para restaurar la tarjeta WiFi a su modo normal:

    ```bash
    sudo chmod 775 ./linux_scripts/restore_managed.sh \
    && ./linux_scripts/restore_managed.sh "Nombre del adaptador WiFi"
    ```

\
Si desea rehacer estos pasos o elegir otro método de conexión, vaya a "Options" -> "Reconfigure the application"

\newpage
## Windows - Modo Plugin
**NOTA:** En Windows 10, hay bastantes tarjetas WiFi que solo funcionarán con Windows 7 o con drivers antiguos, si el programa no se conecta a las redes después de seguir estos pasos, intente cambiar el driver, por uno de Windows 7. También puede verificar si la tarjeta WiFi es compatible con el modo Ad-Hoc \

1. Copie [AdHocRedirectorWiFi.prx](./Plugin/AdHocRedirectorWiFi.prx) hacia la dirección /SEPLUGINS on the PSP.

2. Cree o abra el archivo /SEPLUGINS/GAME.TXT.

3. Añada 

   ```ms0:/seplugins/AdHocRedirectorWiFi.prx 1``` (for PSP 1000/2000/3000) 

   o 

   ``` ef0:/seplugins/AdHocRedirectorWiFi.prx 1``` (for PSP GO)
   
4. Inicie un juego en la consola.
5. Inicie [Start XLink Kai](http://teamxlink.co.uk/) en su ordenador.
    1. Descárguelo [aquí](https://www.teamxlink.co.uk/go?c=download)
    2. Instale XLink Kai ejecutando el archivo .exe descargado.
    3. Ejecute XLink Kai, presionando el botón de Inicio y buscando 'Start XLink Kai'
6. Inicie XLink Handheld Assistant; si recibe un error sobre el tamaño de la terminal en este punto, asegúrese de que el símbolo del sistema tenga al menos un tamaño de 80x24. Ejemplo: Tamaño de terminal demasiado pequeño: 46x18, asegúrese de que el terminal sea de al menos 80x24.
7. En el asistente, seleccione "Plugin Device" y luego use las flechas en su teclado, para bajar hasta el casillero "Next", y posteriormente seleccionada dicha opción, presione la tecla Enter.
8. Pulse la barra espaciadora de su teclado en "Automatically connect to PSP/Vita networks".
9. Desplácese hacia abajo, hasta hallar el adaptador de red correcto, con las flechas de su teclado, y presione la barra espaciadora en el adaptador WiFi que se utilizará para XLHA.
10. Vaya al botón "Next" y presione Enter.
11. Presione Enter en el botón "Next" nuevamente.
12. En el Panel de Control, si está alojando una partida, presione la barra espaciadora en el apartado de "Hosting"; de lo contrario, diríjase al siguiente paso.
13. Presiona Enter en "Start Engine".
14. Ingrese a la arena de XLink Kai en la que desee jugar, desde [WebUI](http://127.0.0.1:34522/)
15. Por el lado de la consola, ve al menú multijugador Ad-Hoc, y comienza a hospedar o unirte a una partida del juego de tu preferencia.
16. En la esquina superior derecha del Panel de control, debería aparecer "Connected to PSP_GameID_...." después de 5-30 segundos.
17. En XLink Kai, haga clic en las métricas y desplácese hacia abajo, debería ver lo siguiente:
![](screenshots/XLinkMetrics.PNG){ width=100% }
(Hay una consola en XLHA, en el apartado de "Connected Aplications", el PSP aparece en el apartado de "Found Consoles" y usted deberá de apreciar "Broadcast Traffic out").
18. ¡Disfruta del juego!
19. Para cerrar XLHA, simplemente presione 'q' en la interfaz.

\
Si desea rehacer estos pasos o elegir otro método de conexión, vaya a "Options" -> "Reconfigure the application"

\newpage
## Descripción General

### Asistente

![Asistente](screenshots/Wizard.PNG)

Este es el primer paso del asistente y contiene los siguientes elementos:

- Monitor Device: Le permite utilizar el método del modo Monitor (sólo en Linux).
- Plugin Device: Le permite utilizar el método con el Plugin.
- Next: Le permite ir al siguiente paso del asistente.

\newpage
### Configuración del modo Monitor

![Configuración del modo Monitor](screenshots/Monitor.PNG)

Este apartado tiene todas las opciones para el modo Monitor, hay bastantes cosas aquí:

- Automatically connect to PSP/Vita networks: Cuando se marca, buscará las redes PSP_ y SCE_, las cuales serán utilizadas para reenviar datos a XLink Kai.
- Acknowledge data packets: Intenta reducir la latencia, ocasionando que la tarjeta WiFi envíe reconocimientos; si esto causa algún problema, desactívelo.
- Only allow packets from the following MAC-address: Si escribe una dirección MAC en este campo, sólo reenviará paquetes de esa dirección MAC, lo que hace posible escuchar solo un dispositivo Playstation Vita / PSP.
- The channel to use: Coloque aquí el canal que está configurado en el dispositivo Playstation Vita / PSP.
- Use the following adapter: Comprueba que el adaptador WiFi está en modo Monitor y que se puede usar para reenviar datos.
- Next: Vaya al siguiente paso del asistente.

\newpage
### Configuración del modo Plugin

![Configuración del modo Plugin](screenshots/Plugin.PNG)

Esto tiene todas las opciones para el modo Plugin:

- Automatically connect to PSP networks: Cuando se marca, buscará las redes de PSP_, y las utilizará para reenviar datos a XLink Kai.
- Reconnect after network has been inactive for (seconds): Cuando se marca la opción "Automatically connect", le permitirá al programa ajustar con precisión cuánto tiempo esperará cuando no haya tráfico, está configurado en 15 segundos de forma predeterminada. Si está configurado en '0, inicialmente se conectará automáticamente a una red PSP después de que se encienda el motor, pero no después de eso. Esto puede ser útil para algunos juegos que no envían nada durante mucho tiempo o dependen de una conexión para que ya esté allí.
- Use SSID from host broadcast: Si está marcado, captará a otros XLink Handheld Assistants para conocer las redes a las que conectarse.
- Use the following adapter: Comprueba que el adaptador WiFi se puede usar para reenviar datos.
- Next: Vaya al siguiente paso del asistente.

\newpage
### Opciones de XLink Kai

![Opciones de XLink Kai](screenshots/KaiOptions.PNG)

Tiene configuraciones relacionadas con la conexión a XLink Kai:

- XLink IP address: Es la dirección IP de la PC, en la que se está ejecutando XLink Kai (deje 127.0.0.1 si está en la misma PC).
- XLink Port: Es el puerto que se utilizará para la conexión XLink Kai (predeterminado: 34523).

\newpage
### Panel de Control

![Panel de Control](screenshots/Dashboard.PNG)

Este es el Tablero o HUD principal de la aplicación, le permite arrancar o detener el motor, y ver información general:

- Imagen del medio: Muestra el estado del motor, en la imagen de arriba, el motor está apagado.
- Hosting: Habilite esto cuando sea anfitrión en un juego, esto transmitirá cualquier cambio SSID potencial a otros jugadores, no lo habilite al unirse a un juego.
- Re-Connect: Cuando se enciende el motor, este botón se puede utilizar para volver a conectarse a la PSP, por ejemplo, si el tiempo de espera se estableció en 0 y ahora la PSP ha cambiado su SSID.
- Options: Lo lleva a la pantalla de opciones.
- Start Engine: Le permite arrancar el motor.
- Status: Muestra si el motor está en un estado de 'Running' (En ejecución), 'Idle' (Inactivo) o 'Error'.

\newpage
![Panel de Control con el motor en ejecución](screenshots/Dashboard_On.PNG)

Cuando el motor está funcionando, algunas cosas cambian, como:

- Imagen del medio: La imagen del medio ha cambiado para reflejar el estado de ejecución.
- Connected to: Muestra la red que la PSP está enviando a XLHA, si no se muestra nada, la PC aún no se ha conectado a una red o se deshabilitó la conexión automática.
- Stop Engine: Le permite detener el motor.

\newpage
### Opciones

![Opciones](screenshots/Options.PNG)

En la pantalla de opciones se pueden configurar algunas cosas:

- Reconfigure the application: Le permite volver al asistente para editar la configuración.
- About the application: Muestra información sobre la versión y el nombre del programa.
- Select a theme: Le permite cambiar la imagen de estado del motor en el tablero, se proporcionan algunos temas.
- Configure log level: Le permite dejar que el motor le brinde más información de registro, útil para la depuración de errores; de otro modo, déjelo en 'Info'.
- Return to the HUD: Use este botón para regresar al tablero.
- Exit the application: Hace lo mismo que habría hecho 'q' y cierra la aplicación.

\newpage
### Tema

![Selección de tema](screenshots/Themes.PNG)

Esta pantalla le permite seleccionar un tema de la carpeta Temas.

- Select Theme: Seleccione el tema que desea utilizar.
- Save selection: Guarda la selección.

