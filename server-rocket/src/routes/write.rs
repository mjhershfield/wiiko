use std::str::FromStr;

use itertools::Itertools;
use rocket::{serde::{Deserialize, json::Json, Serialize}, State};
use uuid::Uuid;

use crate::{player::{PlayerList, Wii}, gamestate::{SharedGameState, GameState}, shirt::{CurrentRoundQuotes, Quote, FinalizedQuotes}, uuid::ClientUuid};

use super::{GenericResponse, PlayerSubmissionsResponse, PlayerSubmissionNumber};

// Client posts shirt quotes
#[derive(Deserialize)]
#[serde(crate = "rocket::serde")]
pub struct AddQuoteRequest<'r> {
    quote: &'r str
}

#[post("/write", data="<new_quote>")]
pub async fn post_write(new_quote: Json<AddQuoteRequest<'_>>, 
    uuid: ClientUuid, player_list: &State<PlayerList>, 
    current_state: &State<SharedGameState>, 
    quotes: &State<CurrentRoundQuotes>
) -> Json<GenericResponse> {

// Make sure player is in the game
    let parsed_uuid: Uuid = uuid.into();
    if !player_list.read().await.contains_key(&parsed_uuid) {
        return Json(GenericResponse { 
            success: false, 
            reason: Some(String::from_str("Requested player does not exist").unwrap()), 
        });
    }

    if *current_state.read().await != GameState::Write {
        return Json(GenericResponse { 
            success: false, 
            reason: Some(String::from_str("You can't write right now!").unwrap()), 
        });
    }

    let quote = Quote {
        creator: Some(parsed_uuid),
        quote: new_quote.quote.to_owned()
    };

    quotes.write().await.push(quote);

    Json(GenericResponse { 
        success: true, 
        reason: None, 
    })
}

#[derive(Serialize)]
#[serde(crate = "rocket::serde")]
pub struct GetQuoteResponse {
    success: bool,
    reason: Option<String>,
    quote: Option<Quote>
}

#[get("/write?<id>")]
pub async fn get_write(id: ClientUuid, client_uuid: ClientUuid, wii: &State<Wii>,player_list: &State<PlayerList>, quotes: &State<FinalizedQuotes>
) -> Json<GetQuoteResponse> {

    // Make sure player is in the game
    let parsed_client_uuid: Uuid = client_uuid.into();
    if *wii.read().await != parsed_client_uuid && !player_list.read().await.contains_key(&parsed_client_uuid) {
        return Json(GetQuoteResponse { 
            success: false, 
            reason: Some(String::from_str("Requested player does not exist").unwrap()), 
            quote: None
        });
    }

    let parsed_quote_uuid: Uuid = id.into();
    match quotes.read().await.get(&parsed_quote_uuid) {
        Some(quote) => {
            let quote_to_return;
            if *wii.read().await == parsed_client_uuid {
                quote_to_return = quote.clone();
            } else {
                quote_to_return = quote.strip_creator();
            }

            return Json(GetQuoteResponse { 
                success: true, 
                reason: None, 
                quote: Some(quote_to_return)
            })
        },
        None => {
            return Json(GetQuoteResponse { 
                success: false, 
                reason: Some(String::from_str("Requested quote does not exist").unwrap()), 
                quote: None
            });
        }
    }
}

// Wii gets number of quotes submitted for each player
#[get("/write/submissions")]
pub async fn get_write_submissions(client_uuid: ClientUuid, wii: &State<Wii>, quotes: &State<CurrentRoundQuotes>) -> Json<PlayerSubmissionsResponse> {
    let parsed_uuid: Uuid = client_uuid.into();
    if *wii.read().await != parsed_uuid {
        return Json(PlayerSubmissionsResponse { 
            success: false, 
            reason: Some(String::from_str("Only wii can view this").unwrap()), 
            submissions: None 
        });
    }

    let submission_counts: Vec<PlayerSubmissionNumber> = quotes
        .read()
        .await
        .iter()
        .map(|quote| quote.creator)
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