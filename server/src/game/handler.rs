use std::str::FromStr;
use warp::filters::ws::Message;

use uuid::Uuid;

use crate::Clients;
use super::types::*;

pub async fn handle_message(gamestate: &mut GameState, clients: Clients, client_id: String, msg: WsMessage) {
    match msg {
        WsMessage::Join { name, magic } => handle_join(gamestate, clients, &client_id, name, magic).await,
        WsMessage::WiiJoin { magic } => handle_wii_join(gamestate, clients, &client_id, magic).await,
        WsMessage::Result { success, reason } => handle_result(gamestate, clients, &client_id, success, reason).await,
        _ => send_message_to_client(clients, &client_id, WsMessage::Result { success: false, reason: String::from_str("Invalid Request type").unwrap() }).await
    };    
}

async fn send_message_to_client(clients: Clients, client_id: &String, msg: WsMessage) {
    let sender = match clients.lock().await.get(client_id) {
        Some(c) => c.sender.clone(),
        None => {println!("Client disconnected before message {:?} could be sent", msg); return}
    };

    println!("sending to client: {:?}", msg);
    let _ = sender.send(Ok(Message::text(serde_json::to_string(&msg).unwrap())));
}

async fn send_message_to_wii(gamestate: &mut GameState, clients: Clients, msg: WsMessage) {
    let wii_uuid = &gamestate.wii.clone().unwrap(); // Make this a match statement :)
    match clients.lock().await
        .iter()
        .find_map(|kv: (&String, &crate::Client)|
            if kv.1.associated_player_id == Some(wii_uuid.to_owned()) {
                return Some(kv.1)
            } else {
                return None
            }) 
    {
        Some(client) => {
            let _ = client.sender.send(Ok(Message::text(serde_json::to_string(&msg).unwrap())));
        }
        None => {
            gamestate.wii = None;
            println!("Wii was disconnected :( and message {:?} could not be sent to it", msg);
        }
    }
}

async fn handle_join(gamestate: &mut GameState, clients: Clients, client_id: &String, name: String, magic: u32) {

    if gamestate.players_can_join == false {
        send_message_to_client(clients, client_id, WsMessage::Result { success: false, reason: String::from_str("Players cannot join yet!").unwrap() }).await;
        return;
    }

    // Check if magic is correct for player
    if magic != gamestate.player_join_magic {
        send_message_to_client(clients, client_id, WsMessage::Result { success: false, reason: String::from_str("Invalid join code").unwrap() }).await;
        return;
    }

    // Check if a player with that name already exists in the game
    for player in &gamestate.players {
        if player.name == name {
            send_message_to_client(clients, client_id, WsMessage::Result { success: false, reason: String::from_str("Player with same name already exists").unwrap() }).await;
            return;
        }
    }

    let new_player_uuid = Uuid::new_v4().to_string();

    // Player doesn't already exist, add them in.
    let new_player: Player = Player { id: new_player_uuid.clone(), name: name.clone(), character: Character::Undecided, victory_quote: String::new() };
    gamestate.players.push(new_player);

    // Associate player UUID with websocket connection
    clients.lock().await.get_mut(client_id).unwrap().associated_player_id = Some(new_player_uuid.clone());

    // Send them a success response
    send_message_to_client(clients.clone(), client_id, WsMessage::Result { success: true, reason: String::new() }).await;

    // Send them a UUID
    send_message_to_client(clients.clone(), client_id, WsMessage::Uuid { uuid: new_player_uuid.clone() }).await;

    // Send them if they are an admin
    let mut is_admin = false;
    if gamestate.admin_player == None {
        gamestate.admin_player = Some(new_player_uuid.clone());
        is_admin = true;
    }
    send_message_to_client(clients.clone(), client_id, WsMessage::Admin { is_admin }).await;

    // Send current state
    send_message_to_client(clients.clone(), client_id, WsMessage::CurrentState { state: gamestate.state.clone() }).await;

    // Tell Wii that a new player has joined
    send_message_to_wii(gamestate, clients.clone(), WsMessage::PlayerJoined { uuid: new_player_uuid.clone(), name: name.clone() }).await;
}

async fn handle_wii_join(gamestate: &mut GameState, clients: Clients, client_id: &String, magic: u32) {
    if gamestate.wii != None {
        send_message_to_client(clients, client_id, WsMessage::Result { success: false, reason: String::from_str("Wii already associated with this game").unwrap() }).await;
        return;
    }

    const WII_MAGIC: u32 = 0xDEADBEEF;
    if magic != WII_MAGIC {
        send_message_to_client(clients, client_id, WsMessage::Result { success: false, reason: String::from_str("Invalid join code").unwrap() }).await;
        return;
    }

    // Add Wii to game
    // TODO: put in wii client uuid, not connection UUID 
    let new_wii_uuid = Uuid::new_v4().to_string();
    clients.lock().await.get_mut(client_id).unwrap().associated_player_id = Some(new_wii_uuid.clone());
    gamestate.wii = Some(new_wii_uuid.clone());
    send_message_to_client(clients.clone(), client_id, WsMessage::Uuid { uuid: new_wii_uuid.clone() }).await;

    // Set player join code
    gamestate.player_join_magic = 1234;

    // Allow players to join
    gamestate.players_can_join = true;

    // Return success
    send_message_to_client(clients.clone(), client_id, WsMessage::Result { success: true, reason: String::new() }).await;

    // Send current state to Wii
    send_message_to_client(clients.clone(), client_id, WsMessage::CurrentState { state: gamestate.state.clone() }).await;
}

async fn handle_result(_gamestate: &mut GameState, clients: Clients, client_id: &String, success: bool, reason: String) {
    send_message_to_client(clients, client_id, WsMessage::Result { success, reason }).await;
}