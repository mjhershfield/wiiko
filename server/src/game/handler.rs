use std::str::FromStr;
use warp::filters::ws::Message;

use uuid::Uuid;

use crate::Client;
use super::types::*;

pub fn handle_message(gamestate: &mut GameState, client: &Client, msg: WsMessage) {
    match msg {
        WsMessage::Join { name, magic } => handle_join(gamestate, client, name, magic),
        WsMessage::WiiJoin { magic } => handle_wii_join(gamestate, client, magic),
        WsMessage::Result { success, reason } => handle_result(gamestate, client, success, reason),
        _ => send_message_to_client(client, WsMessage::Result { success: false, reason: String::from_str("Invalid Request type").unwrap() })
    };    
}

fn send_message_to_client(client: &Client, msg: WsMessage) {
    if let Some(sender) = client.sender.clone() {
        println!("sending response: {:?}", msg);
        let _ = sender.send(Ok(Message::text(serde_json::to_string(&msg).unwrap())));
    }
}

fn handle_join(gamestate: &mut GameState, client: &Client, name: String, magic: u32) {

    if gamestate.players_can_join == false {
        send_message_to_client(client, WsMessage::Result { success: false, reason: String::from_str("Players cannot join yet!").unwrap() });
        return;
    }

    // Check if magic is correct for player
    if magic != gamestate.player_join_magic {
        send_message_to_client(client, WsMessage::Result { success: false, reason: String::from_str("Invalid join code").unwrap() });
        return;
    }

    // Check if a player with that name already exists in the game
    for player in &gamestate.players {
        if player.name == name {
            send_message_to_client(client, WsMessage::Result { success: false, reason: String::from_str("Player with same name already exists").unwrap() });
            return;
        }
    }

    let new_player_uuid = Uuid::new_v4().to_string();

    // Player doesn't already exist, add them in.
    let new_player: Player = Player { id: new_player_uuid.clone(), name: name, character: Character::Undecided, victory_quote: String::new() };

    gamestate.players.push(new_player);

    // Check if player should be an admin
    // TODO have these handler functions send responses from within the handler instead of returning
    // Also make a function to return a "Result"?

    // Send them a success response
    send_message_to_client(client, WsMessage::Result { success: true, reason: String::new() });

    // Send them a UUID
    send_message_to_client(client, WsMessage::Uuid { uuid: new_player_uuid.clone() });

    // Send them if they are an admin
    let mut is_admin = false;
    if gamestate.admin_player == None {
        gamestate.admin_player = Some(new_player_uuid.clone());
        is_admin = true;
    }
    send_message_to_client(client, WsMessage::Admin { is_admin });

    // Send current state
    send_message_to_client(client, WsMessage::CurrentState { state: gamestate.state.clone() })
}

fn handle_wii_join(gamestate: &mut GameState, client: &Client, magic: u32) {
    if gamestate.wii != None {
        send_message_to_client(client, WsMessage::Result { success: false, reason: String::from_str("Wii already associated with this game").unwrap() });
        return;
    }

    const WII_MAGIC: u32 = 0xDEADBEEF;
    if magic != WII_MAGIC {
        send_message_to_client(client, WsMessage::Result { success: false, reason: String::from_str("Invalid join code").unwrap() });
        return;
    }

    // Set player join code
    gamestate.player_join_magic = 1234;

    // Allow players to join
    gamestate.players_can_join = true;
}

fn handle_result(_gamestate: &mut GameState, client: &Client, success: bool, reason: String) {
    send_message_to_client(client, WsMessage::Result { success, reason });
}