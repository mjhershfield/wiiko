use crate::Clients;
use tokio::sync::mpsc::UnboundedReceiver;

use self::types::{MpscMessage, GameState};

pub mod types;
mod handler;

pub async fn game_manager_task(clients: Clients, mut message_queue: UnboundedReceiver<MpscMessage>) {

    let mut gamestate: GameState = GameState::new();

    while let Some(msg) = message_queue.recv().await {
        match clients.lock().await.get(&msg.client_id) {
            Some(client) => {
                if let Some(_sender) = &client.sender {
                    println!("Received a message in the gamestate thread: {:?}", msg.msg);
                    handler::handle_message(&mut gamestate, &client, msg.msg)
                }
            }
            None => println!("Client no longer exists :("),
        }
    }
}