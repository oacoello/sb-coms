# Comns Santa Barbara (sb-coms)
App nativa para Windows y Linux de comunicación por "pulsar para hablar" para misiones de artillería.

## Tecnologías
- C++20
- Qt 6
- CMake
- Opus
- Relay local nativo por UDP.

## Primer avance (Junio 1, 2026)
Comprobar que la comunicación local push-to-talk funciona antes de añadir movimiento de redes:
1. Capturar el audio del micrófono.
2. Capturar transmisión por botón de pulso para hablar.
3. Codificar y decodificar utilizando Opus Encode.
4. Reproducir audio codificado de manera local
5. <img width="842" height="620" alt="image" src="https://github.com/user-attachments/assets/2f837721-fa20-40da-9c67-b6d88ce6b180" />


## Segundo avance (Junio 2, 2026)
1. Cambios de interfaz, orientado al tema del proyecto Santa Barbara.
2. Se pueden realizar grabaciones de las sesiones de escucha, por canal individual y exportar archivos en WAV.
3. Visualización de forma de onda de audio para visualizar el funcionamiento de la transmisión.
4. Se pueden crear y eliminar canales de voz.
<img width="872" height="554" alt="image" src="https://github.com/user-attachments/assets/aaed35af-8346-4f06-a427-aac6c7183628" />

