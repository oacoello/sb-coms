# sb-coms

Native Windows/Linux push-to-talk desktop application.

## Stack

- C++20
- Qt 6
- CMake
- Opus
- Local native UDP relay

## First milestone

Prove the local push-to-talk audio pipeline before adding networking:

1. capture microphone audio
2. gate capture behind push-to-talk state
3. encode/decode with Opus
4. play decoded audio locally

Networking comes after the audio pipeline is stable.

## Local relay

Run the UDP relay locally:

```powershell
.\build-mingw\servers\relay-native\sb-coms-relay.exe
```

An experimental Go relay also exists, but Go must be installed first:

```powershell
cd servers/relay
go run .
```

Then run two desktop clients, connect both to the same channel, and hold the push-to-talk button in one client to send Opus audio through `127.0.0.1:50000` to the other client.
