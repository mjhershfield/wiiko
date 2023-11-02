## Websockets
Use mpsc to channel events from WS receiver tasks to the gamestate task.

All requests to a server are responded to with a success object: 
- {"success": true}
- {"success": false, "reason": ...}
Certain events have more complex success objects, these are explicitly listed when applicable

instead of JSON, use plain text with spaces to separate fields? no that seems dumb.
### Start of connection
- New players send a JSON containing specific magic to differentiate between an admin or a player
	- {"type": "create", "name": "JEFF", "magic": 11111111}
	- WS server responds with {"success": true, "uuid": 1234, "admin": true} if name is unique, game not full -> game continues
	- WS server responds with {"success": false, "reason": ...} -> player gets booted to login screen
- Players that are reconnecting send a JSON containing their UUID
	- {"type": "rejoin", "uuid": 1111111111, "name": JEFF}
	- WS server responds with {"success": true} if UUID in current game and correct name -> game continues
	- Otherwise WS server responds with {"success": false, "reason": ...} -> player gets booted to login screen

Depending on the magic, players get sorted in one of the following ways
- Wii gets its own special role: admin
- Players get sorted into the client array and get a player object created for them.

### Lobby
- Websocket server sends clients a statechange JSON. First player in the room is the admin? Or make it a button on the Wii screen?
	- {"type: state_update", "state": "LOBBY"}
- Players can update their information here
	- {"type": "character_update", "character": 0}
	- {"type": "quote_update", "quote": "HI"}
- Whenever a player changes their character, rebroadcast the taken characters to players
	- {"type: "taken_characters", "taken_characters": [0, 1, 2, 3, 4]}
- Whenever a player changes their character, update the Wii's display
	- {"type": "character_update", uuid: 00000, "character": 9}
- Stretch goal: allow players to set their character's emotions/make character blink
	- {"type": "set_emotion", "uuid": 000000, "emotion": "blink"}

### Starting the game
- The admin player can start the game or cancel the game start
	- {"type": "start_game"}
	- {"type": "cancel_start"}
- These messages are passed on to the wii, which handles the countdown timer
	- {"type": "start_game"}
	- {"type": "cancel_start"}
- When the Wii's countdown timer ends, it issues an update_state request to the server
	- {"type":  "state_update", "state": "WAIT"}
	- later, state_update to DRAW

### Drawing
- Format for players to submit their images:
	- {"type": "submit_image", "bg_color": 0x000000, "image": "base64 encoded image"}
- When a player submits, the Wii is updated
	- {"type": "image_added", "id": player uuid, "total": 4}
- Server may have to take care of scaling images to be 128x128 PNGs
- State transitions taken care of by Wii (TKO 2 style?)

### Quotes
- Format for players to submit their quotes:
	- {"type": "submit_quote", "quote": "string goes here"}
- When a player submits, the Wii is updated
	- {"type": "quote_added", "id": player uuid, "total": 4}
- State transitions taken care of by Wii

### Shirt
- Starts with server giving each player their selections
	- You get as many image/quote options as you provide to the pool
	- Permute and then assign the number required?
	- {"type": "shirt_options", "images": [{id bgcolor data}, {}], "quotes": []}
- Players choose their shirts
	- {"type": "shirt_selection", "image": image_id, "quote": quote_id}
- When a player submits, the Wii is updated
	- {"type": "shirt_submitted", "id": player uuid}
- State transitions taken care of by Wii

### Voting (bracket style)
- Server sends Wii the number of matchups
	- {"type": num_matchups, "remaining_matchups": 4}
- For each matchup, Wii sends start_matchup command
	- {"type": start_matchup}
- Server responds with matchup details (server sets players into vote state)
	- Gets sent to clients as well (or maybe just quotes and colors, no picture?)
	- {"type": matchup, shirt1: {object}, shirt2: {object}}
- Wii tells server when voting ends
	- {type: end_voting}
- After all matchups completed, server tells wii which shirts won
	- {type: winner, winner: {shirt object}}
- Server responds with voting results (server sets players into wait state)
	- {type: voting_results, shirt1: 0, shirt2: 9, remaining_matchups: 3}
- To vote, players send a vote message (sets player into wait state)
	- {type: vote, vote: 0}
- The winner from each round is saved to be in the final round (moved into winning_shirt array when all matchups have been completed)

### Final Round
- State transition sent from Wii to server is FINAL
- Works the same way as normal voting, only shirts are pulled from winning shirts array

### End
- Pretty sure we just do a state change to END
- Admin can do a reset (keep players) or a reset (all new players)

## Game State
Wii: {uuid, sender}
Clients (Active WS connections): {uuid, sender}
Players (players in game): {uuid, character, victory_quote}
Admin: uuid
Images: id: {creator, bgcolor, data}
Quotes: id: {creator, data}
Shirts: id: {creator, image_id, quote_id}
Winning Shirts: id: {creator, image_id, quote_id}
State

images/quotes are moved into shirt objects when shirt objects are inserted into Shirts array
Shirts are moved into winning shirts at end of voting rounds; shirts vector is cleared after that?