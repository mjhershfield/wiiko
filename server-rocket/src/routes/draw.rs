use std::str::FromStr;

use itertools::Itertools;
use rocket::{serde::{Deserialize, Serialize, json::Json}, State};
use uuid::Uuid;

use crate::{shirt::{Drawing, CurrentRoundDrawings, FinalizedDrawings}, gamestate::{GameState, SharedGameState}, player::{PlayerList, Wii}, uuid::ClientUuid};

use super::{GenericResponse, PlayerSubmissionsResponse, PlayerSubmissionNumber};

// Clients post drawings (in base64? add a color header lol? url param?)
#[derive(Deserialize)]
#[serde(crate = "rocket::serde")]
pub struct AddDrawingRequest<'r> {
    drawing: &'r str,
    bg_color: u32
}

#[post("/draw", data="<new_image>")]
pub async fn post_draw(new_image: Json<AddDrawingRequest<'_>>, 
    uuid: ClientUuid, player_list: &State<PlayerList>, 
    current_state: &State<SharedGameState>, 
    drawings: &State<CurrentRoundDrawings>
) -> Json<GenericResponse> {

    // Make sure player is in the game
    let parsed_uuid: Uuid = uuid.into();
    if !player_list.read().await.contains_key(&parsed_uuid) {
        return Json(GenericResponse { 
            success: false,
            reason: Some(String::from_str("Requested player does not exist").unwrap()), 
        });
    }

    if *current_state.read().await != GameState::Draw {
        return Json(GenericResponse { 
            success: false, 
            reason: Some(String::from_str("You can't draw right now!").unwrap()), 
        });
    }

    let drawing = Drawing {
        creator: Some(parsed_uuid),
        data_base64: new_image.drawing.to_owned(),
        bg_color: new_image.bg_color
    };

    drawings.write().await.push(drawing);
    
    Json(GenericResponse {
        success: true, 
        reason: None, 
    })
}

#[derive(Serialize)]
#[serde(crate = "rocket::serde")]
pub struct GetDrawingResponse {
    success: bool,
    reason: Option<String>,
    drawing: Option<Drawing>
}

#[get("/draw?<id>")]
pub async fn get_draw(id: ClientUuid, client_uuid: ClientUuid, wii: &State<Wii>,player_list: &State<PlayerList>, drawings: &State<FinalizedDrawings>
) -> Json<GetDrawingResponse> {

    // Make sure player is in the game
    let parsed_client_uuid: Uuid = client_uuid.into();
    if *wii.read().await != parsed_client_uuid && !player_list.read().await.contains_key(&parsed_client_uuid) {
        return Json(GetDrawingResponse { 
            success: false, 
            reason: Some(String::from_str("Requested player does not exist").unwrap()), 
            drawing: None
        });
    }

    let parsed_image_uuid: Uuid = id.into();
    match drawings.read().await.get(&parsed_image_uuid) {
        Some(drawing) => {
            let drawing_to_return;
            
            if *wii.read().await == parsed_client_uuid {
                drawing_to_return = drawing.clone()
            } else {
                drawing_to_return = drawing.strip_creator()
            }

            return Json(GetDrawingResponse { 
                success: true, 
                reason: None, 
                drawing: Some(drawing_to_return)
            })
        },
        None => {
            return Json(GetDrawingResponse { 
                success: false, 
                reason: Some(String::from_str("Requested drawing does not exist").unwrap()), 
                drawing: None
            });
        }
    }
}

#[get("/draw/submissions")]
pub async fn get_draw_submissions(client_uuid: ClientUuid, wii: &State<Wii>, drawings: &State<CurrentRoundDrawings>) -> Json<PlayerSubmissionsResponse> {
    let parsed_uuid: Uuid = client_uuid.into();
    if *wii.read().await != parsed_uuid {
        return Json(PlayerSubmissionsResponse { 
            success: false, 
            reason: Some(String::from_str("Only wii can view this").unwrap()), 
            submissions: None 
        });
    }

    let submission_counts: Vec<PlayerSubmissionNumber> = drawings
        .read()
        .await
        .iter()
        .map(|drawing| drawing.creator)
        .counts()
        .iter()
        .map(|(id, num_submitted)| PlayerSubmissionNumber {id: id.unwrap().clone(), num_submitted: *num_submitted as u32})
        .collect();

    return Json(PlayerSubmissionsResponse { 
        success: true, 
        reason: None, 
        submissions: Some(submission_counts)
    });
}