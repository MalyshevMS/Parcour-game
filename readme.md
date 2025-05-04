# ParCour game
 - [My github](https://github.com/MalyshevMS)
 - [How to run it?](#how-to-run-it)
 - [Connect to a server](#connect-to-a-server)
 - [Latest release](https://github.com/MalyshevMS/Parcour-game/releases/tag/v1.0.0)

This is a simply minimalistic 2D game about parkour.

### WARNING
Project is on develop stage! Running it on your PC is only **YOUR** responsibility!\
We're not sure that launching it won't hurt your PC.


# Connect to a server
To connect to a server you need to write IP and port to [server.json](settings/server.json)\
Your addres must be in this format:
```json
{
    "ip": "Server IP address (IPv4 only)",
    "port": 1111
}
```
Then relaunch or rebuild a game

# How to run it?
Go by [this link](https://github.com/MalyshevMS/Parcour-game/releases/tag/v1.0.0), scroll down and then download zip archive. Then unarhive this and launch Parcour-game.exe

### If you don't have access to latest release or wanna change source code and build it yourself, follow this steps:
 - Create a folder *build* in main directory.
 - Downdload <a href="https://cmake.org/">CMake</a>.
 - Open cmd (or bash) in engine directory and type there:
```bash 
cd build 
cmake .. 
cmake --build . 
```
 - Open binary by path \build\bin\Parcour-game\Parcour-game.exe
 - **CONGRATULATIONS!** You have started the game!
