use crate::Clients;
use tokio::sync::mpsc::UnboundedReceiver;

use self::types::{MpscMessage, GameState};

pub mod types;
mod handler;

pub async fn game_manager_task(clients: Clients, mut message_queue: UnboundedReceiver<MpscMessage>) {

    let mut gamestate: GameState = GameState::new();

    while let Some(msg) = message_queue.recv().await {
        handler::handle_message(&mut gamestate, clients.clone(), msg.client_id, msg.msg).await;
    }
}