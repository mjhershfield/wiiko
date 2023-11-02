use std::{collections::HashMap, convert::Infallible, sync::Arc, time::Duration};
use tokio::{sync::{mpsc, Mutex}, time::sleep};
use warp::{ws::Message, Filter, Rejection};

mod handlers;
mod ws;
mod gamestate;

#[derive(Debug, Clone)]
pub struct Client {
    pub client_id: String,
    pub sender: Option<mpsc::UnboundedSender<std::result::Result<Message, warp::Error>>>,
}

type Clients = Arc<Mutex<HashMap<String, Client>>>;
type Result<T> = std::result::Result<T, Rejection>;

#[tokio::main]
async fn main() {
    let clients: Clients = Arc::new(Mutex::new(HashMap::new()));

    println!("Configuring websocket route");
    let ws_route = warp::path("ws")
        .and(warp::ws())
        .and(with_clients(clients.clone()))
        .and_then(handlers::ws_handler);

    let routes = ws_route.with(warp::cors().allow_any_origin());

    let clients_ref = clients.clone();

    tokio::spawn(async move {
        let mut i: u32 = 0;
        loop {
            println!("{}", i);
            for client in clients_ref.lock().await.iter() {
                match &client.1.sender {
                    None => {}
                    Some(s) => {
                        let _ = s.send(Ok(Message::text(format!("{}", i))));
                    }
                }
            }
            i += 1;
            sleep(Duration::from_millis(1000)).await;
        }
    });

    println!("Starting server");
    warp::serve(routes).run(([127, 0, 0, 1], 8000)).await;
}

fn with_clients(clients: Clients) -> impl Filter<Extract = (Clients,), Error = Infallible> + Clone {
    warp::any().map(move || clients.clone())
}