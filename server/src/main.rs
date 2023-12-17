use std::{collections::HashMap, convert::Infallible, sync::Arc};
use game::types::MpscMessage;
use tokio::sync::{mpsc::{self, UnboundedSender}, Mutex};
use warp::{ws::Message, Filter, Rejection, reply::Reply};

use crate::game::game_manager_task;

mod ws;
mod game;

#[derive(Debug, Clone)]
pub struct Client {
    // pub associated_player_id: Option<String>,
    pub sender: Option<mpsc::UnboundedSender<std::result::Result<Message, warp::Error>>>,
}

type Clients = Arc<Mutex<HashMap<String, Client>>>;
type Result<T> = std::result::Result<T, Rejection>;

#[tokio::main]
async fn main() {
    // Global variables that are used
    let clients: Clients = Arc::new(Mutex::new(HashMap::new()));
    let (gamestate_tx, gamestate_rx) = tokio::sync::mpsc::unbounded_channel();

    println!("Starting game manager task");
    tokio::spawn(game_manager_task(clients.clone(), gamestate_rx));

    println!("Configuring websocket route");
    let ws_route = warp::path("ws")
        .and(warp::ws())
        .and(with_clients(clients.clone()))
        .and(with_gamestate_tx(gamestate_tx))
        .and_then(ws_route_handler);

    let routes = ws_route.with(warp::cors().allow_any_origin());

    println!("Starting server");
    warp::serve(routes).run(([127, 0, 0, 1], 8000)).await;
}

fn with_clients(clients: Clients) -> impl Filter<Extract = (Clients,), Error = Infallible> + Clone {
    warp::any().map(move || clients.clone())
}

fn with_gamestate_tx(tx: UnboundedSender<MpscMessage>) -> impl Filter<Extract = (UnboundedSender<MpscMessage>,), Error = Infallible> + Clone {
    warp::any().map(move || tx.clone())
}

async fn ws_route_handler(ws: warp::ws::Ws, clients: Clients, gamestate_tx: UnboundedSender<MpscMessage>) -> Result<impl Reply> {
    println!("ws_handler");

    Ok(ws.on_upgrade(move |socket| ws::client_connection(socket, clients, gamestate_tx)))
}