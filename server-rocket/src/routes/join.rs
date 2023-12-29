use std::str::FromStr;
use rocket::{serde::{Deserialize, json::Json, Serialize}, State};
use uuid::Uuid;

use crate::player::{PlayerList, Player, Character};

#[derive(Deserialize)]
#[serde(crate = "rocket::serde")]
pub struct JoinRequest<'r> {
    username: &'r str,
}

#[derive(Serialize)]
#[serde(crate = "rocket::serde")]
pub struct JoinResponse {
    success: bool,
    reason: Option<String>,
    uuid: Option<Uuid>,
    character: Option<Character>,
    admin: Option<bool>
}

#[post("/join", data = "<new_player>")]
pub async fn post_join<'a>(new_player: Json<JoinRequest<'_>>, player_list: &State<PlayerList>) -> Json<JoinResponse> {

    // Make sure there are no players that have the same name
    for player in player_list.read().await.iter() {
        if player.1.name == new_player.username {
            return Json(JoinResponse { 
                success: false, 
                reason: Some(String::from_str("Player with same name already exists").unwrap()), 
                uuid: None,
                character: None,
                admin: None
            });
        }
    }

    // Add new player to players list
    let new_player_obj = Player::new(new_player.username, player_list.read().await.is_empty());
    let new_player_uuid = Uuid::new_v4();

    let response = Json(JoinResponse { 
        success: true, 
        reason: None, 
        uuid: Some(new_player_uuid),
        character: Some(new_player_obj.character),
        admin: Some(new_player_obj.admin)
    });

    player_list.write().await.insert(new_player_uuid, new_player_obj);

    response
}