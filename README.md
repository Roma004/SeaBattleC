# SeaBattle C (POSIX only)

simple console SeaBattle written in C

## Features

In current version things are really simple. You can:

* Emplace and reemplace ships into the field with keyboard
* Play game with 2 players in single screen (both set up their own field)
* Win your partner

Use the hints at the top of the screen to get help about navigating

## TODO List

If you want to help the project you could try this things:

* Divide test.c to separate files (structurize project)
* Test this game on Windows (I think, PowerShell only) and MacOS
* Implement Enemy AI game mode
* Implement peer-to-peer play
* Rewrite code so that numbers of ships and sizes of fields would be configurable
* Or something else

## Build & Run

```
git clone https://github.com/Roma004/SeaBattleC.git
cd SeaBattleC
make build
make run
```

every call of `make run` starts new game 


## Credits

If you want to help the author, pull request with new game features is the best
way to do that.

If you really want to help the author, there is no way to do that. Sorry :(
