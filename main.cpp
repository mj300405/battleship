#include "SDL.h"
#undef main
#include<iostream>
#include <cstdlib>
#include <vector>
#include <queue>
#include <iostream>
#include <math.h>

#include <string>
#include <SDL_ttf.h>

//#include "Window.h"
#include "Board.h"
#include "Ship.h"
#include "Game.h"
#include <memory>
//#include "TextMessage.h"


using namespace std;

int main()
{

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
    }

    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        std::cerr << "TTF_Init: " << TTF_GetError() << std::endl;
    }

    // bool koniec_koniec = false;
    // // std::unique_ptr<Game> game = nullptr;
    // Game* game = nullptr;
    // while(!koniec_koniec){
    //     // game = std::make_unique<Game>("Battleship");
    //     if(game != nullptr){
    //         delete game;
    //     }
    //     game = new Game("Battleship");
    //     // Run the game loop
    //     koniec_koniec = game->run_game_loop();
    // }
    // Instantiate the game
    try{
        Game game("Battleship");

        // Run the game loop
        game.run_game_loop();

    } catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }

    SDL_Quit();
    TTF_Quit();
    // delete game;
    return 0;
    
}