// Wii gets all player info

use std::str::FromStr;

use rocket::serde::Deserialize;
use rocket::serde::json::Json;
use rocket::{serde::Serialize, State};
use uuid::Uuid;

use crate::gamestate::{GameState, SharedGameState};
use crate::player::{Character, PlayerList, Wii, Emotion};
use crate::uuid::ClientUuid;

use super::GenericResponse;

#[derive(Serialize)]
#[serde(crate = "rocket::serde")]
pub struct PlayerData {
    uuid: Uuid,
    name: String,
    character: Character,
    emotion: Emotion
}

#[derive(Serialize)]
#[serde(crate = "rocket::serde")]
pub struct PlayerInfoResponse {
    success: bool,
    reason: Option<String>,
    players: Option<Vec<PlayerData>>
}

#[get("/players")]
pub async fn get_players<'a>(uuid: ClientUuid, player_list:  &State<PlayerList>, wii: &State<Wii>) -> Json<PlayerInfoResponse> {
    if *wii.read().await != uuid.into() {
        return Json(PlayerInfoResponse {
                success: false,
                reason: Some(String::from_str("Must be Wii to get player data").unwrap()),
                players: None
            });
    }

    let locked_player_list = player_list.read().await;
    let player_data = locked_player_list.iter().map(|player|
        PlayerData {uuid: player.0.clone(), name: player.1.name.clone(), emotion: player.1.emotion, character: player.1.character}
    ).collect();

    Json(PlayerInfoResponse { 
            success: true, 
            reason: None, 
            players: Some(player_data)
    })
}

#[derive(Deserialize)]
#[serde(crate = "rocket::serde")]
pub struct UpdateCharacterRequest { 
    new_character: Character
}

#[post("/players/character", data="<new_character>")]
pub async fn post_players_character(new_character: Json<UpdateCharacterRequest>, uuid: ClientUuid, player_list: &State<PlayerList>, current_state: &State<SharedGameState>) -> Json<GenericResponse> {

    // Make sure player is in the game
    let parsed_uuid: Uuid = uuid.into();
    if !player_list.read().await.contains_key(&parsed_uuid) {
        return Json(GenericResponse { 
            success: false, 
            reason: Some(String::from_str("Requested player does not exist").unwrap()), 
        });
    }

    if *current_state.read().await != GameState::Lobby {
        return Json(GenericResponse { 
            success: false, 
            reason: Some(String::from_str("You can't change your character after the game starts!").unwrap()), 
        });
    }

    player_list.write().await.get_mut(&parsed_uuid).unwrap().character = new_character.new_character;
    
    Json(GenericResponse { 
        success: true, 
        reason: None, 
    })
}

#[derive(Deserialize)]
#[serde(crate = "rocket::serde")]
pub struct UpdateQuoteRequest {
    new_quote: String
}

#[post("/players/quote", data="<new_quote>")]
pub async fn post_players_quote(new_quote: Json<UpdateQuoteRequest>, uuid: ClientUuid, player_list: &State<PlayerList>, current_state: &State<SharedGameState>) -> Json<GenericResponse> {

    // Make sure player is in the game
    let parsed_uuid: Uuid = uuid.into();
    if !player_list.read().await.contains_key(&parsed_uuid) {
        return Json(GenericResponse { 
            success: false, 
            reason: Some(String::from_str("Requested player does not exist").unwrap()), 
        });
    }

    if *current_state.read().await != GameState::Lobby {
        return Json(GenericResponse { 
            success: false, 
            reason: Some(String::from_str("You can't change your victory quote after the game starts!").unwrap()), 
        });
    }

    player_list.write().await.get_mut(&parsed_uuid).unwrap().victory_quote = new_quote.new_quote.clone();
    
    Json(GenericResponse { 
        success: true, 
        reason: None, 
    })
}

#[derive(Deserialize)]
#[serde(crate = "rocket::serde")]
pub struct UpdateEmotionRequest { 
    emotion: Emotion
}

#[post("/players/emotion", data="<new_emotion>")]
pub async fn post_players_emotion(new_emotion: Json<UpdateEmotionRequest>, uuid: ClientUuid, player_list: &State<PlayerList>, current_state: &State<SharedGameState>) -> Json<GenericResponse> {

    // Make sure player is in the game
    let parsed_uuid: Uuid = uuid.into();
    if !player_list.read().await.contains_key(&parsed_uuid) {
        return Json(GenericResponse { 
            success: false, 
            reason: Some(String::from_str("Requested player does not exist").unwrap()), 
        });
    }

    if *current_state.read().await != GameState::Lobby {
        return Json(GenericResponse { 
            success: false, 
            reason: Some(String::from_str("You can't change your character after the game starts!").unwrap()), 
        });
    }

    player_list.write().await.get_mut(&parsed_uuid).unwrap().emotion = new_emotion.emotion;
    
    Json(GenericResponse { 
        success: true, 
        reason: None, 
    })
}