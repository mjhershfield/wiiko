use std::str::FromStr;

use itertools::Itertools;
use rocket::{serde::{json::Json, Serialize, Deserialize}, State};
use uuid::Uuid;

use crate::{shirt::{CurrentRoundShirtOptions, FinalizedDrawings, FinalizedQuotes, CurrentRoundShirts, Shirt}, player::{PlayerList, Wii}, gamestate::{SharedGameState, GameState}, uuid::ClientUuid};

use super::{GenericResponse, PlayerSubmissionsResponse, PlayerSubmissionNumber};

#[derive(Serialize)]
#[serde(crate = "rocket::serde")]
pub struct GetShirtOptionsResponse {
    success: bool,
    reason: Option<String>,
    drawings: Option<Vec<Uuid>>,
    quotes: Option<Vec<Uuid>>
}

#[get("/shirt/options")]
pub async fn get_shirt_options(
    uuid: ClientUuid, player_list: &State<PlayerList>, 
    current_state: &State<SharedGameState>, 
    shirt_options: &State<CurrentRoundShirtOptions>
) -> Json<GetShirtOptionsResponse> {

    // Make sure player is in the game
    let parsed_uuid: Uuid = uuid.into();
    if !player_list.read().await.contains_key(&parsed_uuid) {
        return Json(GetShirtOptionsResponse { 
            success: false, 
            reason: Some(String::from_str("Requested player does not exist").unwrap()), 
            drawings: None,
            quotes: None
        });
    }

    if *current_state.read().await != GameState::Shirt {
        return Json(GetShirtOptionsResponse { 
            success: false, 
            reason: Some(String::from_str("You can't make a shirt right now!").unwrap()), 
            drawings: None,
            quotes: None
        });
    }

    let locked_shirt_options = shirt_options.read().await;
    let options = match locked_shirt_options.get(&parsed_uuid) {
        Some(o) => o,
        None => return Json(GetShirtOptionsResponse { 
            success: false, 
            reason: Some(String::from_str("Could not get shirt options for requested player").unwrap()), 
            drawings: None,
            quotes: None
        })
    };

    Json(GetShirtOptionsResponse { 
        success: true, 
        reason: None, 
        drawings: Some(options.images.to_owned()), 
        quotes: Some(options.quotes.to_owned()) 
    })
}

// Clients POST their selection
#[derive(Deserialize)]
#[serde(crate = "rocket::serde")]
pub struct ShirtSelectionRequest {
    drawing: Uuid,
    quote: Uuid
}

#[post("/shirt", data="<new_shirt>")]
pub async fn post_shirt(
    new_shirt: Json<ShirtSelectionRequest>,
    uuid: ClientUuid, 
    player_list: &State<PlayerList>, 
    current_state: &State<SharedGameState>, 
    drawings: &State<FinalizedDrawings>,
    quotes: &State<FinalizedQuotes>,
    shirts: &State<CurrentRoundShirts>
) -> Json<GenericResponse> {

    // Make sure player is in the game
    let parsed_uuid: Uuid = uuid.into();
    if !player_list.read().await.contains_key(&parsed_uuid) {
        return Json(GenericResponse { 
            success: false, 
            reason: Some(String::from_str("Requested player does not exist").unwrap()), 
        });
    }

    // Make sure we are in the shirt design state
    if *current_state.read().await != GameState::Shirt {
        return Json(GenericResponse { 
            success: false, 
            reason: Some(String::from_str("You can't make a shirt right now!").unwrap()), 
        });
    }

    // Make sure player has not already submitted a shirt
    // Actually don't, as a band aid fix for storing multiple round's shirts in the same shirt array
    // let already_submitted = shirts.read()
    //     .await.iter()
    //     .find_map(|(_, shirt)| if shirt.creator == Some(parsed_uuid) {Some(())} else {None})
    //     .is_some();

    // if already_submitted {
    //     return Json(GenericResponse { 
    //         success: false, 
    //         reason: Some(String::from_str("You already submitted a shirt!").unwrap()), 
    //     });
    // }

    // Add shirt
    let chosen_drawing = drawings.write().await.remove(&new_shirt.drawing).unwrap();
    let chosen_quote = quotes.write().await.remove(&new_shirt.quote).unwrap();

    let new_shirt_struct = Shirt {
        creator: Some(parsed_uuid),
        drawing: chosen_drawing,
        quote: chosen_quote
    };

    shirts.write().await.insert(Uuid::new_v4(), new_shirt_struct);

    Json(GenericResponse { 
        success: true, 
        reason: None, 
    })
}

// Wii gets submission status for each player
#[get("/shirt/submissions")]
pub async fn get_shirt_submissions(client_uuid: ClientUuid, wii: &State<Wii>, shirts: &State<CurrentRoundShirts>) -> Json<PlayerSubmissionsResponse> {
    let parsed_uuid: Uuid = client_uuid.into();
    if *wii.read().await != parsed_uuid {
        return Json(PlayerSubmissionsResponse { 
            success: false, 
            reason: Some(String::from_str("Only wii can view this").unwrap()), 
            submissions: None 
        });
    }

    let submission_counts: Vec<PlayerSubmissionNumber> = shirts
        .read()
        .await
        .iter()
        .map(|(_, shirt)| shirt.creator)
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

#[derive(Serialize, Debug)]
#[serde(crate = "rocket::serde")]
pub struct GetShirtResponse {
    success: bool,
    reason: Option<String>,
    shirt: Option<Shirt>,
}

#[get("/shirt?<id>")]
pub async fn get_shirt(id: ClientUuid, client_uuid: ClientUuid, wii: &State<Wii>,player_list: &State<PlayerList>, shirts: &State<CurrentRoundShirts>
) -> Json<GetShirtResponse> {
    // Make sure player is in the game
    let parsed_client_uuid: Uuid = client_uuid.into();
    if *wii.read().await != parsed_client_uuid && !player_list.read().await.contains_key(&parsed_client_uuid) {
        return Json(GetShirtResponse { 
            success: false, 
            reason: Some(String::from_str("Requested player does not exist").unwrap()), 
            shirt: None
        });
    }

    if *wii.read().await == parsed_client_uuid {
        println!("WII REQUESTED SHIRT!!\n\n");
    }

    let parsed_shirt_uuid: Uuid = id.into();
    match shirts.read().await.get(&parsed_shirt_uuid) {
        Some(shirt) => {
            let shirt_to_return;
            if *wii.read().await == parsed_client_uuid {
                shirt_to_return = shirt.clone();
            } else {
                shirt_to_return = shirt.strip_creator();
            }

            let resp = Json(GetShirtResponse { 
                success: true, 
                reason: None, 
                shirt: Some(shirt_to_return)
            });

            if *wii.read().await == parsed_client_uuid {
                println!("WII REQUESTED SHIRT!!\n\n{:?}\n", resp);
            }

            return resp;
        },
        None => {
            return Json(GetShirtResponse { 
                success: false, 
                reason: Some(String::from_str("Requested shirt does not exist").unwrap()), 
                shirt: None
            });
        }
    }
}