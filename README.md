# EscapeRoom

## Introduction
In the current time of the pandemic world, making things accessible online or in other
formats that maintain the safety of everyone has become crucial. This idea is especially prevalent
for those who are immunocompromised and cannot afford to live life the way it was before
COVID-19 without severe health risks. Hence, we strive to make one aspect of entertainment,
that was at first almost exclusively only accessible in person, attainable for all. Namely, we will
be creating a virtual escape room. 

## How to play
After compiling the program using the provided Makefile, the first player can start the game using `./player-one`. After that, the second player can connect to the port that player one is listening to (which will be displayed in the UI of player one) with `./player-two <server name> <port>`. Upon sucessful connections, players proceed by the instructions prompted in the game.

### How the program work:



## Notes
An important rule of this game is that players would strictly follow the instructions given by the narrator of the game, especially when giving commands (inputs starting with `:`). Breaking this rule can have unexpected outcomes, and players might never be able to escape the dangerous room!!!!
