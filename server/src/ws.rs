use std::str::FromStr;

use crate::{Client, Clients};
use crate::game::types::*;

use futures::stream::SplitStream;
use futures::{FutureExt, StreamExt};
use tokio::sync::mpsc::{self, UnboundedSender};
use tokio_stream::wrappers::UnboundedReceiverStream;
use uuid::Uuid;
use warp::ws::{Message, WebSocket};

pub async fn client_connection(ws: WebSocket, clients: Clients, gamestate_tx: UnboundedSender<MpscMessage>) {
    println!("establishing client connection... {:?}", ws);

    let (client_ws_sender, client_ws_rcv) = ws.split();
    let (client_sender, client_rcv) = mpsc::unbounded_channel();

    let client_rcv = UnboundedReceiverStream::new(client_rcv);

    tokio::task::spawn(client_rcv.forward(client_ws_sender).map(|result| {
        if let Err(e) = result {
            println!("error sending websocket msg: {}", e);
        }
    }));

    let uuid = Uuid::new_v4().simple().to_string();

    let new_client = Client {
        associated_player_id: None,
        sender: client_sender
    };

    clients.lock().await.insert(uuid.clone(), new_client);

    tokio::task::spawn(client_task(clients, client_ws_rcv, gamestate_tx, uuid));
    
}

async fn client_task(clients: Clients, mut client_ws_rcv: SplitStream<WebSocket>, gamestate_tx: UnboundedSender<MpscMessage>, uuid: String) {
    while let Some(result) = client_ws_rcv.next().await {
        let msg = match result {
            Ok(msg) => msg,
            Err(e) => {
                println!("error receiving message for id {}): {}", uuid.clone(), e);
                break;
            }
        };
        client_msg(&uuid, msg, &gamestate_tx).await;
    }

    clients.lock().await.remove(&uuid);
    println!("{} disconnected", uuid);
}

async fn client_msg(client_id: &str, msg: Message, gamestate_tx: &UnboundedSender<MpscMessage>) {

    // Pass all messages received from the client to the gamestate thread.
    let parsed_message: WsMessage = parse_client_message(msg);
    println!("Sending to gamestate_tx!");
    match gamestate_tx.send(MpscMessage {client_id: client_id.to_string(), msg: parsed_message}) {
        Ok(()) => println!("message sent successfully"),
        Err(e) => println!("Error: {}", e.to_string())
    };

}

fn parse_client_message(msg: Message) -> WsMessage {
    
    let message = match msg.to_str() {
        Ok(v) => v,
        Err(_) => return WsMessage::Result { success: false, reason: "Message is not text".to_string() },
    };

    let parsed_json: WsMessage = match serde_json::from_str(message) {
        Ok(s) => s,
        Err(e) => return WsMessage::Result { success: false, reason: String::from_str("Invalid request JSON").unwrap() },
    };

    println!("Received message: {:?}", parsed_json);
    parsed_json
}