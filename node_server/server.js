const WebSocket = require('ws');

const wss = new WebSocket.Server({ port: 8000 });

let games = {};

wss.on('connection', (ws) => {
  console.log("New connection established");
  
  ws.on('message', (message) => {
    console.log("Received message: " + message);
    const data = JSON.parse(message);

    switch(data.type) {
        case 'create_game': {
            const game_id = data.game_id;
            console.log("Trying to create a game with id: " + game_id);
            games[game_id] = [ws];
            //ws.send(JSON.stringify({"event": "game_created", "game_id": game_id}));
            console.log("Game created with id: " + game_id);
            console.log("Current games: ", games);
            break;
          }
          

      case 'join_game': {
        const game_id = data.game_id;
        console.log("Trying to join game with id: " + game_id);
        console.log("Current games: ", games);
        if (games[game_id] && games[game_id].length < 2) {
          games[game_id].push(ws);
          //ws.send(JSON.stringify({"event": "game_joined", "game_id": game_id}));
          console.log("Game joined with id: " + game_id);
        } else {
          //ws.send(JSON.stringify({"event": "error", "message": "Game not found or already full"}));
          console.log("Game not found or already full");
        }
        break;
      }
      

      case 'leave_game': {
        const game_id = data.game_id;
        const game = games[game_id];
        if (game && game.includes(ws)) {
          games[game_id] = game.filter(player => player !== ws);
          console.log("Player left game with id: " + game_id);
        }
        break;
      }

      case 'place_ships': {
        const game_id = data.game_id;
        const ships = data.ships;
        const game = games[game_id];
        if (game) {
          game.forEach(player => {
            if (player !== ws) {
            console.log("Sending ships to player in game with id: " + game_id);
              player.send(JSON.stringify({"event": "place_ships", "ships": ships}));
            }
          });
        }
        break;
      }

      case 'game_update': {
        const game_id = data.game_id;
        const shot = data.shot;
        const game = games[game_id];
        if (game) {
          game.forEach(player => {
            if (player !== ws) {
            console.log("Sending game update to player in game with id: " + game_id);
              player.send(JSON.stringify({"event": "game_update", "shot": shot}));
            }
          });
          console.log("Game state updated for game with id: " + game_id);
        }
        break;
      }
    }
  });

  ws.on('close', () => {
    // When a client disconnects, remove them from any game they're part of
    console.log("Connection closed");
    for (let game_id in games) {
      if (games[game_id].includes(ws)) {
        games[game_id] = games[game_id].filter(player => player !== ws);
        console.log("Player disconnected, removed from game with id: " + game_id);
      }
    }
  });
});

function generateId() {
  return Math.floor(Math.random() * 10000).toString();
}

console.log("Server started, waiting for connections...");
