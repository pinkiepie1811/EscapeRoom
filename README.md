# EscapeRoom

## Introduction
In the current time of the pandemic world, making things accessible online or in other formats that maintain the safety of everyone has become crucial. This idea is especially prevalent for those who are immunocompromised and cannot afford to live life the way it was before COVID-19 without severe health risks. Hence, we strive to make one aspect of entertainment, that was at first almost exclusively only accessible in person, attainable for all. Namely, we created virtual escape room. 

## How to play
After compiling the program using the provided Makefile, the players can begin connected. Each must open a half-screen or full-screen terminal. The first player can then start the game using `./player-one`. After that, the second player can connect to the port that player one is listening to (which will be displayed in the UI of player one) with `./player-two <server name> <port>`.

Upon sucessful connections, players proceed by the instructions prompted in the game. Generally, input is done by typing with keyboard characters or by moving elements on screen with the arrow-keys. To quit the game, enter `:q` or `:quit`.

## Notes
An important rule of this game is that players would strictly follow the instructions given by the narrator of the game, especially when giving commands (inputs starting with `:` or '[' and ending with ']'). Breaking this rule can have unexpected outcomes, and players might never be able to escape the dangerous room!!!!


### SOLUTIONS/WALKTHROUGH (Do not read unless you intend to cheat!)

1. Start programs and connect users
2. Expect about 30 seconds of narration. Then, users are prompted to send a message to each other
3. When both players have sent any message terminating with the enter key, the messages will be exchanged and a timer will appear.
4. Narration will continue.From this point, gameplay for Player 1 and Player 2 diverges.
    - Player 1:
        - Type `:enter` as prompted
        - Player 1 will appear in a box. Their position is labeled by the '@' symbol, and the outer bounds are shown with `*`, and invisible walls lurk. They must navigate through the maze with arrow keys and instructions from Player 2 to get to the exit (`E`). A solution is:
            1. Down 3
            2. Left 2
            3. Down 4
            4. Right 5
            5. Down 2
            6. Right 4
            7. Up 3
            8. Right 6
            9. Down 7
            10. Left 4
            11. Down 6
    - Player 2:
        - Type `:pull` as prompted.
        - A map of the maze player 1 is in will appear. Assist player 1 in getting through the maze through text 
5. Having escaped the maze, players will then go to the next puzzle
    - Player 1:
        - Type `:view` as prompted.
        - Player 1 must solve the mathgames to give a code to player 2.
    - Player 2:
        - Type `:door` as prompted.
        - Use the left and right arrowkeys to change the focused digit on the keypad, then use up and down arrow keys to adjust the value of that digit. Switch the numbers on the keypad to the password. The solution is 0542.
6. Players have solved the math puzzle, they must then continue.
    - Type `:open` as prompted.
    - Type the solved anagram into the terminal:
        - Player 1: '[pmosera]'
        - Player 1: '[charliecurtsinger]'
7. Players have solved the anagram, move onto the final boss by typing ':fight'
8. Players must fight the giant octopus by using the arrow keys to avoid the flying lazers. The player will always see themselves as the '1' on the screen, and the other player as '2'. The players must attack the octopus a total of 10 times by getting to the octopus. When they move in the same space as the octopis, the octopus will take damage and the player will be thrown to the back. 
9. Upon defeating the octopus, players will almost be done. Type ':exit' to complete the program.
10. (addendum): If, at any point, the timer reaches 0, the players will both be thrown out of the game as they die.
