from flask import Flask
from flask_sockets import Sockets
from collections import defaultdict
import json
import uuid

app = Flask(__name__)
sockets = Sockets(app)

games = defaultdict(list)

@app.route('/')
def index():
    return "Battleship Game Server"

def process_message(ws, message):
    print("Received message: " + message)
    data = json.loads(message)

    if data['type'] == 'create_game':
        game_id = str(uuid.uuid4())
        games[game_id].append(ws)
        ws.send(json.dumps({"event": "game_created", "game_id": game_id}))
        print("Game created with id: " + game_id)

    elif data['type'] == 'join_game':
        game_id = data['game_id']
        if game_id in games and len(games[game_id]) < 2:
            games[game_id].append(ws)
            ws.send(json.dumps({"event": "game_joined", "game_id": game_id}))
            print("Game joined with id: " + game_id)
        else:
            ws.send(json.dumps({"event": "error", "message": "Game not found or already full"}))
            print("Game not found or already full")

    elif data['type'] == 'leave_game':
        game_id = data['game_id']
        if ws in games[game_id]:
            games[game_id].remove(ws)

    elif data['type'] == 'game_update':
        game_id = data['game_id']
        game_state = data['game_state']
        # Forward the game state to the other player(s)
        for player_ws in games[game_id]:
            if player_ws != ws:
                player_ws.send(json.dumps({"event": "game_update", "game_state": game_state}))

@sockets.route('/socket')
def game_socket(ws):
    while not ws.closed:
        message = ws.receive()
        process_message(ws, message)
    # If the WebSocket is closed, remove the client from their current game session
    for game_id, players in games.items():
        if ws in players:
            players.remove(ws)


            
 # gunicorn -k geventwebsocket.gunicorn.workers.GeventWebSocketWorker -w 1 server:app