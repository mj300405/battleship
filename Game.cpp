#include "SDL.h"
#include "Game.h"
#include "Board.h"
#include "Player.h"
#include "Message.h"
#include <thread>
#include <chrono>
#include <regex>

SDL_Texture* render_text(const std::string& message, TTF_Font* font, SDL_Color color, SDL_Renderer* renderer) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, message.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

Game::Game(const std::string& title) :
    game_running(true), game_phase(GamePhase::MENU), is_multiplayer(false) {

    int grid_cell_size = 20;
    int grid_width = 10;
    int grid_height = 10;
    this->window_width = 2.5 * (grid_width * grid_cell_size) + 1;
    this->window_height = 1.5 * (grid_height * grid_cell_size) + 1;

    window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Error creating window: " << SDL_GetError() << std::endl;
        // handle error
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Error creating renderer: " << SDL_GetError() << std::endl;
        // handle error
    }

    Board* player_board_1 = new Board(0, 0, true);
    Board* player_board_2 = new Board(window_width - grid_width * grid_cell_size - 1, 0, false);

    player1 = Player(player_board_1);
    player2 = Player(player_board_2);
    ai = AI(&player2, player2.get_board());

    current_player = &player1;

    std::string initial_message = "Welcome to Battleships!";
    SDL_Color color = { 255, 255, 255, 255 }; // white color 
    font = TTF_OpenFont("Montserrat-Black.ttf", 18); // 18 is the font size
    if (!font) {
        std::cerr << "Error loading font: " << TTF_GetError() << std::endl;
        // handle error
    }

    gameMessage = new GameMessage(renderer, font, color);
    setup_game_menu();

    //auto handler = std::bind(&Game::handle_incoming_message, this, std::placeholders::_1);

    conn = new Connection("localhost", "8000");
    game_id = "";
    event = new SDL_Event;
    setup_game_over_menu();
    ultimate_end = false;
    std::cout<<"Konstruktor Game"<<std::endl;
    setup_multiplayer_menu();

    is_data_ready = false;
}

void Game::setup_game_over_menu() {
    game_over_menu = new Menu();

    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Color black = { 0, 0, 0, 255 };

    int button_width = window_width / 2;
    int button_height = window_height / 10;
    int button_y_spacing = button_height / 2;

    SDL_Rect buttonRectSinglePlayer = { window_width / 2 - button_width / 2, window_height / 4, button_width, button_height };
    SDL_Texture* buttonTextSinglePlayer = render_text("Main Menu", font, black, renderer);
    game_over_menu->add_button(new Button(buttonRectSinglePlayer, white, buttonTextSinglePlayer, [this]() {
        this->game_phase = GamePhase::MENU;
        ultimate_end = false;
        }));

    SDL_Rect buttonRectMultiPlayer = { window_width / 2 - button_width / 2, buttonRectSinglePlayer.y + button_height + button_y_spacing, button_width, button_height };
    SDL_Texture* buttonTextMultiPlayer = render_text("Exit", font, black, renderer);
    game_over_menu->add_button(new Button(buttonRectMultiPlayer, white, buttonTextMultiPlayer, [this]() {
        this->exit_game();
        ultimate_end = true;
        }));
}

Game::~Game() {
    delete game_over_menu;
    delete gameMenu;
    delete gameMessage;
    delete conn;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    std::cout<<"Destruktor Game"<<std::endl;
}

bool Game::run_game_loop() {
    while (game_running) {
        SDL_PollEvent(event);
        if (event->type == SDL_QUIT) {
            game_running = false;
            ultimate_end = true;
        }
        switch (game_phase) {
        case GamePhase::MENU:
            handle_phase_menu();

            break;
        case GamePhase::SETUP:
            setup_phase_mouse_click();
            handle_phase_setup();
            break;
        case GamePhase::PLAY:
            handle_phase_play();
            break;
        case GamePhase::WAITING:
            game_phase = GamePhase::PLAY;
            break;
        case GamePhase::GAME_OVER:
            handle_phase_game_over();
            break;
        }

        // Update screen and handle events here
        render();
    }
    return ultimate_end;
}

//legit
void Game::handle_phase_menu() {
        if (event->type == SDL_MOUSEBUTTONDOWN) {
            int x, y;
            SDL_GetMouseState(&x, &y);
            if(gameMenu->is_visible())
                gameMenu->handle_click(x, y);
            if(multiplayer_menu->is_visible())
                multiplayer_menu->handle_click(x, y);
        }
        if (multiplayer_menu->is_visible()) {
            multiplayer_menu->get_buttons()[0]->handle_event(event);
		}
}

//legit
void Game::start_single_player_game() {
    is_multiplayer = false;
    game_phase = GamePhase::SETUP;
    // set any specific game configuration here
}

//legit
void Game::start_multi_player_game() {
    gameMenu->hide();
    multiplayer_menu->show();
    is_multiplayer = true;
    conn->Connect();

    // std::thread receiveThread([&client]() {
	// 	while (true) {
	// 		client.Receive([](const std::string& message) {
    //             // 
	// 			std::cout << "Received message: " << message << std::endl;
	// 		});
	// 	}
	// });

	// std::thread sendThread([&client]() {
	// 	while (true) {
	// 		std::unique_lock ulock(client.send_mutex);
	// 		client.send_data_var.wait(
	// 			ulock, [&client]() { return client.message.size() != 0; });
	// 		client.Send(client.message);
	// 		client.message = "";
	// 	}
	// });



}

//legit
void Game::exit_game() {
    game_running = false;
}

//legit
void Game::handle_phase_setup() {
    if (player1.get_board()->all_ships_placed()) {
        gameMessage->update_message("Player 1, place your ships!");
        if (is_multiplayer)
        {
            nlohmann::json j;
            j["game_id"] = game_id;
            j["type"] = "place_ships";
            for (auto ship : player1.get_board()->ships)
            {
                j["ships"].push_back(ship.to_json());
			}   
			// conn->send_message(j);
            conn->Send(j.dump());
            //
            // conn->receive_message([this](nlohmann::json data){
            //     std::cout << "HALO" << std::endl;
            //     player2.get_board()->ships = json_to_ships_vector(data);
            //     for (auto ship : player2.get_board()->ships)
            //     {
			// 	    player2.get_board()->place_ship(ship);
			//     }
            //     //game_phase = GamePhase::PLAY;
            // });
            conn->Receive([this](const std::string& message) {
                std::cout << "HALO" << std::endl;
                nlohmann::json data = nlohmann::json::parse(message);
                player2.get_board()->ships = json_to_ships_vector(data);
                for (auto ship : player2.get_board()->ships)
                {
                    player2.get_board()->place_ship(ship);
                }
                });
            if(is_joining) current_player = &player2;
			else current_player = &player1;
            
            game_phase = GamePhase::WAITING;
            
        }

    }
    else if (!player2.get_board()->all_ships_placed()) {
        gameMessage->update_message("Player 2, place your ships!");
        if (!is_multiplayer)
        {
            //
            ai.place_ships();

		}
        // else
    }
    if(player1.get_board()->all_ships_placed() && player2.get_board()->all_ships_placed()) {
        // Once both players have placed all their ships, move to the PLAY phase
        gameMessage->update_message("Battle Begins!");
        game_phase = GamePhase::PLAY;
    }
}


std::vector<Ship> Game::json_to_ships_vector(const nlohmann::json& j) {
    std::vector<Ship> ships;
    for (auto& ship_json : j["ships"]) {
        Ship ship;
        ship.from_json(ship_json);
        ships.push_back(ship);
    }
    return ships;
}

void Game::handle_phase_play() {
    // Check if any player has all their ships sunk
    if (player1.get_board()->all_ships_sunk()) {
        gameMessage->update_message("Player 2 wins!");
        game_phase = GamePhase::GAME_OVER;
    }
    else if (player2.get_board()->all_ships_sunk()) {
        gameMessage->update_message("Player 1 wins!");
        game_phase = GamePhase::GAME_OVER;
    }
    else if (!is_multiplayer) {
        // If in single-player mode, the AI makes a move when it's their turn
        if (current_player == &player1 && event->type == SDL_MOUSEBUTTONDOWN) {
            if(player2.get_board()->shoot(event))
                if(!player2.get_board()->was_last_shot_hit())
                    current_player = &player2;
        }
        else if (current_player == &player2) {
            std::pair<int, int> ai_move = ai.make_move();
            int a = ai_move.first;
            int b = ai_move.second;
            player1.get_board()->shoot(a, b);
            if(!player1.get_board()->was_last_shot_hit())
                current_player = &player1;
        }
    }  
    else if (is_multiplayer) {
        if (current_player == &player1 && event->type == SDL_MOUSEBUTTONDOWN) {
            // If it's player 1's turn and the player clicked somewhere
            if (player2.get_board()->shoot(event)) {
                // If the shot is valid, send this move to the server
                nlohmann::json requestMsg;
                requestMsg["type"] = "game_update";
                requestMsg["game_id"] = game_id;
                requestMsg["shot"] = {   player2.get_board()->get_last_shot().first, player2.get_board()->get_last_shot().second };
                // conn->send_message(requestMsg);
                conn->Send(requestMsg.dump());

                // Check if the shot was a hit
                if (!player2.get_board()->was_last_shot_hit()) {
                    // If the shot was not a hit, switch the current player to player 2
                    current_player = &player2;
                }
            }
        }
        else if (current_player == &player2) {
            // If it's player 2's turn, wait for the server to send player 2's move
            // conn->receive_message([this](nlohmann::json data){
            //     std::cout << "HALO 2" << std::endl;

            //     if (data["type"] == "game_update") {
            //         // Extract the move from the response
            //         std::pair<int, int> move = { data["shot"][0].get<int>(), data["shot"][1].get<int>()};

            //         int a = move.first;
            //         int b = move.second;
            //         player1.get_board()->shoot(a, b);

            //         // Check if the shot was a hit
            //         if (!player1.get_board()->was_last_shot_hit()) {
            //             // If the shot was not a hit, switch the current player to player 1
            //             current_player = &player1;
            //         }
            //     }
            //     //game_phase = GamePhase::PLAY;
            // });
            conn->Receive([this](const std::string& mes){
                std::cout << mes << std::endl;
                nlohmann::json data = nlohmann::json::parse(mes);
                if (data["event"] == "game_update") {
                    // Extract the move from the response
                    std::pair<int, int> move = { data["shot"][0].get<int>(), data["shot"][1].get<int>()};

                    int a = move.first;
                    int b = move.second;
                    std::cout << a << ',' << b<<std::endl; 
                    player1.get_board()->shoot(a, b);

                    // Check if the shot was a hit
                    if (!player1.get_board()->was_last_shot_hit()) {
                        // If the shot was not a hit, switch the current player to player 1
                        current_player = &player1;
                    }
                }
                //game_phase = GamePhase::PLAY;
            });
            game_phase = GamePhase::WAITING;

            
        }
    }
}


void Game::render() {
    SDL_SetRenderDrawColor(renderer, 22, 22, 22, 255);
    SDL_RenderClear(renderer);

    switch (game_phase) {
    case GamePhase::MENU:
        gameMenu->render(renderer);
        multiplayer_menu->render(renderer);
        break;

    case GamePhase::SETUP:
    case GamePhase::PLAY:
        // Draw player1's board with their ships
        player1.get_board()->draw_ships(renderer);
        player1.get_board()->draw(renderer);
        player1.get_board()->draw_cursor(renderer, &cursor_ghost1);

        // Draw player2's board with player1's shots
        player2.get_board()->render_enemy_grid(renderer);
        player2.get_board()->draw(renderer);
        player2.get_board()->draw_cursor(renderer, &cursor_ghost2);

        gameMessage->display(50, 250);
        break;
    case GamePhase::GAME_OVER:
        	game_over_menu->render(renderer);
		break;
    }

    SDL_RenderPresent(renderer);
}


//legit
void Game::setup_game_menu() {
    gameMenu = new Menu();

    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Color black = { 0, 0, 0, 255 };

    int button_width = window_width / 2;
    int button_height = window_height / 10;
    int button_y_spacing = button_height / 2;

    SDL_Rect buttonRectSinglePlayer = { window_width / 2 - button_width / 2, window_height / 4, button_width, button_height };
    SDL_Texture* buttonTextSinglePlayer = render_text("Single Player", font, black, renderer);
    gameMenu->add_button(new Button(buttonRectSinglePlayer, white, buttonTextSinglePlayer, [this]() {
        this->start_single_player_game();
        }));

    SDL_Rect buttonRectMultiPlayer = { window_width / 2 - button_width / 2, buttonRectSinglePlayer.y + button_height + button_y_spacing, button_width, button_height };
    SDL_Texture* buttonTextMultiPlayer = render_text("Multi Player", font, black, renderer);
    gameMenu->add_button(new Button(buttonRectMultiPlayer, white, buttonTextMultiPlayer, [this]() {
        this->start_multi_player_game();
        }));

    SDL_Rect buttonRectExit = { window_width / 2 - button_width / 2, buttonRectMultiPlayer.y + button_height + button_y_spacing, button_width, button_height };
    SDL_Texture* buttonTextExit = render_text("Exit", font, black, renderer);
    gameMenu->add_button(new Button(buttonRectExit, white, buttonTextExit, [this]() {
        this->exit_game();
        }));
}

void Game::setup_multiplayer_menu() {
    multiplayer_menu = new Menu();
    multiplayer_menu->hide();
    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Color black = { 0, 0, 0, 255 };

    int button_width = window_width / 2;
    int button_height = window_height / 10;
    int button_y_spacing = button_height / 2;

    SDL_Rect buttonRectSinglePlayer = { window_width / 2 - button_width / 2, window_height / 4, button_width, button_height };
    SDL_Texture* buttonTextSinglePlayer = render_text("", font, black, renderer);
    multiplayer_menu->add_button(new TextField(buttonRectSinglePlayer, white, buttonTextSinglePlayer, "", font, black));

    SDL_Rect buttonRectMultiPlayer = { window_width / 2 - button_width / 2, buttonRectSinglePlayer.y + button_height + button_y_spacing, button_width, button_height };
    SDL_Texture* buttonTextMultiPlayer = render_text("Create game", font, black, renderer);
    multiplayer_menu->add_button(new Button(buttonRectMultiPlayer, white, buttonTextMultiPlayer, [this]() {
            // send create game json
        std::regex reg("^\\d+$");
        if (!std::regex_match(multiplayer_menu->get_buttons()[0]->get_text(), reg))
			return;
        game_id = multiplayer_menu->get_buttons()[0]->get_text();
        conn->create_game(game_id);
        std::cout << "create game\n"<< multiplayer_menu->get_buttons()[0]->get_text() << std::endl;
        game_phase = GamePhase::SETUP;
        is_joining = false;
        }));

    SDL_Rect buttonRectExit = { window_width / 2 - button_width / 2, buttonRectMultiPlayer.y + button_height + button_y_spacing, button_width, button_height };
    SDL_Texture* buttonTextExit = render_text("Join game", font, black, renderer);
    multiplayer_menu->add_button(new Button(buttonRectExit, white, buttonTextExit, [this]() {
            // send join game json
        std::regex reg("^\\d+$");
        if (!std::regex_match(multiplayer_menu->get_buttons()[0]->get_text(), reg))
            return;
        game_id = multiplayer_menu->get_buttons()[0]->get_text();
        conn->join_game(game_id);
        std::cout << "join game\n"<< multiplayer_menu->get_buttons()[0]->get_text()<<std::endl;
        game_phase = GamePhase::SETUP;
        is_joining = true;
        }));

   
}

//legit
void Game::setup_phase_mouse_click() {
    Board& active_board = *(current_player->get_board());

    if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        int row = (event->button.y - active_board.get_grid_offset_y()) / active_board.get_grid_cell_size();
        int col = (event->button.x - active_board.get_grid_offset_x()) / active_board.get_grid_cell_size();

        if (active_board.selected_ship != nullptr) {
            active_board.rotate_ship(event, *active_board.selected_ship);
        }
        else {
            for (Ship& ship : active_board.ships) {
                if (ship.is_placed && ship.start_row == row && ship.start_col == col) {
                    active_board.rotate_ship(event, ship);
                    break;
                }
            }
        }

        Ship* rotated_ship = active_board.get_ship_at(row, col);
        if (active_board.selected_ship == nullptr && (rotated_ship == nullptr || !rotated_ship->is_placed)) {
            for (Ship& ship : active_board.ships) {
                if (!ship.is_placed) {
                    active_board.selected_ship = &ship;
                    break;
                }
            }
        }

        if (active_board.selected_ship != nullptr && (rotated_ship == nullptr || !rotated_ship->is_placed)) {
            if (!active_board.place_ship(event, *active_board.selected_ship)) {
                std::cout << "Ship placement failed!" << std::endl;
                active_board.selected_ship->start_row = -1;
                active_board.selected_ship->start_col = -1;
                active_board.selected_ship = nullptr;
            }
            else {
                active_board.selected_ship = nullptr;
            }
        }
    }
    else if (event->button.button == SDL_BUTTON_RIGHT) {
        active_board.remove_ship(event);
    }

    
}

void Game::handle_phase_game_over() {
    game_over_menu->show();
    if (event->type == SDL_MOUSEBUTTONDOWN) {
            int x, y;
            SDL_GetMouseState(&x, &y);
            game_over_menu->handle_click(x, y);
    }
}

void Game::handle_incoming_message(const std::string& message) {
    nlohmann::json jsonMessage = nlohmann::json::parse(message);

    std::string event = jsonMessage["game_state"]["event"];
    if (event == "ship_placed") {
        int start_row = jsonMessage["game_state"]["ship_data"]["start_row"];
        int start_col = jsonMessage["game_state"]["ship_data"]["start_col"];
        int size = jsonMessage["game_state"]["ship_data"]["size"];
        bool is_horizontal = jsonMessage["game_state"]["ship_data"]["is_horizontal"];
        // Now use these values to update your game state
    }
    else if (event == "shot_fired") {
        int row = jsonMessage["game_state"]["shot_data"]["row"];
        int col = jsonMessage["game_state"]["shot_data"]["col"];

        if (current_player == &player1) {
            player1.get_board()->shoot(row, col);
        }
        else if (current_player == &player2) {
            player2.get_board()->shoot(row, col);
        }
    }
}

void Game::handle_phase_waiting(){
    // is_data_ready = false - nic sie nie dzieje
    std::lock_guard<std::mutex> lg(bool_mutex);
    if(is_data_ready){
        is_data_ready = false;
        game_phase = GamePhase::PLAY;
    }
}