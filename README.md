# sb-coms

Native Windows/Linux push-to-talk desktop application.

## Stack

- C++20
- Qt 6
- CMake
- Opus

## First milestone

Prove the local push-to-talk audio pipeline before adding networking:

1. capture microphone audio
2. gate capture behind push-to-talk state
3. encode/decode with Opus
4. play decoded audio locally

Networking comes after the audio pipeline is stable.
