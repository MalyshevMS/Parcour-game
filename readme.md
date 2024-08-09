# ParCour game
 - [My github](https://github.com/MalyshevMS)
 - [How to run it?](#how-to-run-it)
 - [Install a dedicated server](#installing-a-dedicated-server)
 - [Connect to a dedicated server](#installing-a-dedicated-server)
 - [Engine source code](https://github.com/MalyshevMS/M2-engine)

This is a simply minimalistic 2D game about parkour.

### WARNING
Project is on develop stage! Running it on your PC is only **YOUR** responsibility!\
We're not sure that launching it won't hurt your PC.

# Installing a dedicated server
Go by <a href="https://github.com/MalyshevMS/ParCour-game-dedicated-server">this link</a> and install (build) a dedicated server.

# Connect to a server
To connect to a server you need to write IP and port to [server.cfg](settings/server.cfg)\
Your addres must be in this format:
```cpp
your.server.ip.address:port;
// supports only IPv4 format
```
Then relaunch or rebuild a game

# How to run it?
 - Create a folder *build* in main directory.
 - Downdload <a href="https://cmake.org/">CMake</a>.
 - Open cmd (or bash) in engine directory and type there: 
```bash 
cd build 
cmake .. 
cmake --build . 
```
 - Open binary by path \build\bin\M2_engine (GNU/Linux) or \build\bin\Debug\M2_engine (Win32).
 - **CONGRATULATIONS!** You have started the engine!