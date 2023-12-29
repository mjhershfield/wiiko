use std::str::FromStr;
use itertools::Itertools;
use rand::{seq::SliceRandom, thread_rng};
use rocket::{serde::{json::Json, Serialize, Deserialize}, State};
use uuid::Uuid;

use crate::{player::{PlayerList, Wii}, gamestate::{self, SharedGameState, Started, GameState}, uuid::ClientUuid, shirt::{CurrentRoundDrawings, CurrentRoundQuotes, FinalizedDrawings, FinalizedQuotes, CurrentRoundShirtOptions, ShirtOptions, CurrentRoundShirts, CurrentShirtBracket, FinalRoundShirts}};

use super::GenericResponse;

#[derive(Serialize)]
#[serde(crate = "rocket::serde")]
pub struct StateResponse {
    success: bool,
    reason: Option<String>,
    state: Option<gamestate::GameState>
}

#[get("/game/state")]
pub async fn get_game_state(uuid: ClientUuid, player_list: &State<PlayerList>, wii: &State<Wii>, current_state: &State<SharedGameState>) -> Json<StateResponse> {

    //TODO: Check if uuid belongs to Wii
    let parsed_uuid: Uuid = uuid.into();
    if *wii.read().await != parsed_uuid && !player_list.read().await.contains_key(&parsed_uuid) {
        return Json(StateResponse { 
            success: false, 
            reason: Some(String::from_str("Requested player does not exist").unwrap()), 
            state: None
        });
    }

    Json(StateResponse { 
        success: true, 
        reason: None, 
        state: Some(current_state.read().await.get_state())
    })
}

#[derive(Deserialize)]
#[serde(crate = "rocket::serde")]
pub struct StateUpdateRequest {
    state: gamestate::GameState
}

// Wii can post state to change it
#[post("/game/state", data = "<new_state>")]
pub async fn post_game_state(
    new_state: Json<StateUpdateRequest>, 
    uuid: ClientUuid, 
    wii: &State<Wii>, 
    current_state: &State<SharedGameState>,
    player_list: &State<PlayerList>,
    current_drawings: &State<CurrentRoundDrawings>,
    finalized_drawings: &State<FinalizedDrawings>,
    current_quotes: &State<CurrentRoundQuotes>,
    finalized_quotes: &State<FinalizedQuotes>,
    shirt_options: &State<CurrentRoundShirtOptions>,
    current_shirts: &State<CurrentRoundShirts>,
    current_bracket: &State<CurrentShirtBracket>,
    final_shirts: &State<FinalRoundShirts>
) -> Json<GenericResponse> {
    if *wii.read().await != uuid.into() {
        return Json(GenericResponse { 
            success: false, 
            reason: Some(String::from_str("You're not the wii >:0").unwrap()), 
        });
    }

    // Update the state
    let state_transition = current_state.write().await.set_state(new_state.state);

    // Check if the current state is different from the previous state (ignoring transistions to Wait)
    if state_transition {
        // Run new state entry code
        match new_state.state {
            GameState::Lobby => {
                // Clear all player generated data from previous runs (shirts, quotes, drawings)
                // Also clear all players
                player_list.write().await.clear();
                current_drawings.write().await.clear();
                finalized_drawings.write().await.clear();
                current_quotes.write().await.clear();
                finalized_quotes.write().await.clear();
                shirt_options.write().await.clear();
                current_shirts.write().await.clear();
                current_bracket.write().await.clear();
                final_shirts.write().await.clear();
            },
            GameState::Shirt => {
                // Clear previous round shirt options
                shirt_options.write().await.clear();

                // Finalize all shirts and quotes from the current round
                for drawing in current_drawings.write().await.drain(..) {
                    finalized_drawings.write().await.insert(Uuid::new_v4(), drawing);
                }

                for quote in current_quotes.write().await.drain(..) {
                    finalized_quotes.write().await.insert(Uuid::new_v4(), quote);
                }


                let mut rand_drawings = finalized_drawings
                    .read()
                    .await
                    .iter()
                    .map(|(uuid, _)| uuid.clone())
                    .collect_vec();

                let mut rand_quotes = finalized_quotes
                    .read()
                    .await
                    .iter()
                    .map(|(uuid, _)| uuid.clone())
                    .collect_vec();

                let all_player_uuids = player_list
                    .read()
                    .await
                    .iter()
                    .map(|(uuid, _)| uuid.clone())
                    .collect_vec();

                let drawings_per_player = rand_drawings.len() / all_player_uuids.len();
                let quotes_per_player = rand_quotes.len() / all_player_uuids.len();
                let mut locked_shirt_options = shirt_options.write().await;

                // shuffle rand_drawings, rand_quotes
                let mut rng = thread_rng();
                rand_drawings.shuffle(&mut rng);
                rand_quotes.shuffle(&mut rng);

                // TODO: what if 0 drawings/quotes per player?
                for player_uuid in all_player_uuids {
                    let mut shirt_option = ShirtOptions::new();

                    for _ in 0..drawings_per_player {
                        shirt_option.images.push(rand_drawings.pop().unwrap())
                    }

                    for _ in 0..quotes_per_player {
                        shirt_option.quotes.push(rand_quotes.pop().unwrap())
                    }
                    locked_shirt_options.insert(player_uuid, shirt_option);
                }
            },
            GameState::Battle => {
                
                let mut locked_bracket = current_bracket.write().await;

                // clear the old bracket
                locked_bracket.clear();

                // Insert current round shirts into bracket
                for (uuid, _) in current_shirts.read().await.iter() {
                    locked_bracket.push_back(uuid.clone());
                }

                // Shuffle the shirt order to create the tournament
                let mut rng = thread_rng();
                locked_bracket.make_contiguous().shuffle(&mut rng);
            },
            GameState::FinalBattle => {
                // Move the final battle shirts into the battle shirts and shuffle
                let mut locked_current_round_shirts = current_shirts.write().await;
                locked_current_round_shirts.clear();

                for shirt in final_shirts.write().await.drain(..) {
                    locked_current_round_shirts.insert(Uuid::new_v4(), shirt);
                }

                let mut locked_bracket = current_bracket.write().await;

                // clear the old bracket
                locked_bracket.clear();

                // Insert current round shirts into bracket
                for (uuid, _) in locked_current_round_shirts.iter() {
                    locked_bracket.push_back(uuid.clone());
                }

                // Shuffle the shirt order to create the tournament
                let mut rng = thread_rng();
                locked_bracket.make_contiguous().shuffle(&mut rng);

            },
            _ => {}
        }
    }

    return Json(GenericResponse { 
        success: true, 
        reason: None, 
    })
}

#[derive(Deserialize)]
#[serde(crate = "rocket::serde")]
pub struct GameStartRequest {
    starting: bool
}

#[post("/game/start", data = "<starting>")]
pub async fn post_game_start<'a>(starting: Json<GameStartRequest>, uuid: ClientUuid, player_list: &State<PlayerList>, started: &State<Started>, current_state: &State<SharedGameState>) -> Json<GenericResponse> {
    
    match player_list.read().await.get(&uuid.into()) {
        Some(p) if p.admin => {

            if *current_state.read().await != GameState::Lobby {
                return Json(GenericResponse {
                    success: false,
                    reason: Some(String::from_str("Game has already started!").unwrap()),
                })
            }

            *started.write().await = starting.starting;
            return Json(GenericResponse {
                success: true,
                reason: None,
            })
        },
        Some(_) => return Json(GenericResponse { 
            success: false, 
            reason: Some(String::from_str("Only admins can start the game").unwrap()),
        }),
        None => return Json(GenericResponse { 
            success: false, 
            reason: Some(String::from_str("Requested player does not exist").unwrap()),
        })
    }
}

#[derive(Serialize)]
#[serde(crate = "rocket::serde")]
pub struct GameStartResponse {
    success: bool,
    reason: Option<String>,
    starting: Option<bool>
}

#[get("/game/start")]
pub async fn get_game_start<'a>(uuid: ClientUuid, wii: &State<Wii>, started: &State<Started>) -> Json<GameStartResponse> {
    
    if *wii.read().await != uuid.into() {
        return Json(GameStartResponse { success: false, reason: Some(String::from_str("Only the wii can get game start status").unwrap()), starting: None });
    }

    Json(GameStartResponse {
        success: true,
        reason: None,
        starting: Some(*started.read().await)
    })
}