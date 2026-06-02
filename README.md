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

## Segundo avance (Junio 2, 2026)
1. Cambios de interfaz, orientado al tema del proyecto Santa Barbara.
2. Se pueden realizar grabaciones de las sesiones de escucha, por canal individual y exportar archivos en WAV.
3. Visualización de forma de onda de audio para visualizar el funcionamiento de la transmisión.
4. Se pueden crear y eliminar canales de voz.
