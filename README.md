# Spark
## Description
Fun and Competitive Game created without a game engine entirely in C
- Intended to run an a LINUX machine with CLANG
- Uses the ncurses libraries for display and socket libraries for network communication
## Controls
### Setting up
- After building run with no commandline arguments exp: ./spark
- Server will start on this device and start listening for sockets
- On a seperate device run with command line arguments name of the server device and the generated port adress exp: ./spark fraenkel 48257
### Playing
- Server computer can use w and s arrow keys to decide a map and press enter to confirm
- Both players race to the 0s using w a s d keys for movement
- first player to reach the 0s gets a point
- along the way, players can pick up Xs to gain gold
- After each round, players can spend Gold to alter their opponents map
- Use w and a arrow keys and enter to select walls are sprites and add to oponents map with space bar
- Press b to go back and select a new item
- Press n when ready to move on to next round
- first to 5 points wins
