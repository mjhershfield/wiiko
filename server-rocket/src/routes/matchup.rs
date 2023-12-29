use std::str::FromStr;

use itertools::Itertools;
use uuid::Uuid;
use rocket::{serde::{json::Json, Serialize, Deserialize}, State};

use crate::{gamestate::{SharedGameState, GameState}, shirt::{CurrentMatchup, CurrentShirtBracket, FinalRoundShirts, CurrentRoundShirts}, player::{Wii, PlayerList}, uuid::ClientUuid};

use super::{GenericResponse, PlayerSubmissionsResponse, PlayerSubmissionNumber};

#[derive(Serialize)]
#[serde(crate = "rocket::serde")]
pub struct ShirtForRequests {
    drawing: Uuid,
    quote: Uuid
}

// Get current matchup
#[derive(Serialize)]
#[serde(crate = "rocket::serde")]
pub struct MatchupResponse {
    success: bool,
    reason: Option<String>,
    shirt0: Option<Uuid>,
    shirt1: Option<Uuid>
}

#[get("/matchup")]
pub async fn get_matchup(
    client_uuid: ClientUuid, 
    wii: &State<Wii>,
    player_list: &State<PlayerList>, 
    current_state: &State<SharedGameState>,
    current_matchup: &State<CurrentMatchup>
) -> Json<MatchupResponse> {

    // Make sure player is in the game
    let parsed_client_uuid: Uuid = client_uuid.into();
    if *wii.read().await != parsed_client_uuid && !player_list.read().await.contains_key(&parsed_client_uuid) {
        return Json(MatchupResponse { 
            success: false, 
            reason: Some(String::from_str("Requested player does not exist").unwrap()), 
            shirt0: None,
            shirt1: None
        });
    }

    // Make sure we are in Battle state
    if *current_state.read().await != GameState::Battle && *current_state.read().await != GameState::FinalBattle {
        return Json(MatchupResponse { 
            success: false, 
            reason: Some(String::from_str("You can't vote right now!").unwrap()), 
            shirt0: None,
            shirt1: None
        });
    }

    let locked_shirt0 = &current_matchup.read().await.shirt0;
    let locked_shirt1 = &current_matchup.read().await.shirt1;

    let (owned_shirt0, owned_shirt1) = match (locked_shirt0, locked_shirt1) {
        (Some(s0), Some(s1)) => (s0.clone(), s1.clone()),
        _ => return Json(MatchupResponse { 
            success: false, 
            reason: Some(String::from_str("Matchup is invalid").unwrap()), 
            shirt0: None,
            shirt1: None
        })
    };

    // Return matchup

    Json(MatchupResponse { 
        success: true, 
        reason: None, 
        shirt0: Some(owned_shirt0), 
        shirt1: Some(owned_shirt1) 
    })
}

// correct order of control: set state to wait, get final vote results, move to next matchup, set state to vote
// post to move to next matchup. move all shirts in final matchup to the final round also
#[derive(Serialize)]
#[serde(crate = "rocket::serde")]
pub struct NextMatchupResponse {
    success: bool,
    reason: Option<String>,
    prev_winner: Option<Uuid>,
    final_matchup: Option<bool>,
}

#[post("/matchup")]
pub async fn post_matchup(
    uuid: ClientUuid, 
    wii: &State<Wii>, 
    current_shirts: &State<CurrentRoundShirts>,
    current_matchup: &State<CurrentMatchup>,
    current_bracket: &State<CurrentShirtBracket>,
    final_shirts: &State<FinalRoundShirts>
) -> Json<NextMatchupResponse> {

    let parsed_client_uuid: Uuid = uuid.into();
    if *wii.read().await != parsed_client_uuid {
        return Json(NextMatchupResponse { 
            success: false, 
            reason: Some(String::from_str("Only wii can do this").unwrap()), 
            prev_winner: None,
            final_matchup: None
        });
    }

    
    // Move winner of current matchup (if existing) to end of bracket
    let locked_matchup = current_matchup.read().await;
    let locked_shirt0 = &locked_matchup.shirt0;
    let locked_shirt1 = &locked_matchup.shirt1;

    let prev_winner;

    match (locked_shirt0, locked_shirt1) {
        (Some(s0), Some(s1)) => {
            println!("Both shirts in matchup exist!");
            let votes = current_matchup
                .read()
                .await
                .votes
                .iter()
                .map(|(_player_uuid, shirt_uuid)| shirt_uuid.clone())
                .counts();

            let shirt0_votes = votes.get(s0).unwrap_or(&0);
            let shirt1_votes = votes.get(s1).unwrap_or(&0);

            if shirt0_votes > shirt1_votes {
                current_bracket.write().await.push_back(s0.clone());
                prev_winner = Some(s0.clone());
            } else if shirt1_votes > shirt0_votes {
                current_bracket.write().await.push_back(s1.clone());
                prev_winner = Some(s1.clone());
            } else {
                let winner = s0.clone();
                current_bracket.write().await.push_back(winner);
                prev_winner = Some(winner);
            }
        }
        _ => {
            println!("At least one shirt in matchup doesn't exist!");
            prev_winner = None
        }
    }

    drop(locked_matchup);
    println!("Clearing matchup");
    let mut locked_matchup = current_matchup.write().await;
    locked_matchup.shirt0 = None;
    locked_matchup.shirt1 = None;

    println!("Clearing votes");
    locked_matchup.votes.clear();

    println!("Length of bracket deque: {}", current_bracket.read().await.len());

    // put next two images from bracket into matchup
    if current_bracket.read().await.len() >= 2 {
        println!("Putting next two shirts into matchup");
        locked_matchup.shirt0 = current_bracket.write().await.pop_front();
        locked_matchup.shirt1 = current_bracket.write().await.pop_front();
    }

    println!("New matchup: {:?}", *locked_matchup);

    // if bracket is empty, set final_matchup to true
    let final_matchup = current_bracket.read().await.is_empty();

    if final_matchup {
        println!("Setting up final matchup!");
        let mut locked_final_matchup = final_shirts.write().await;
        locked_final_matchup.push(current_shirts.read().await.get(&locked_matchup.shirt0.unwrap()).unwrap().clone());
        locked_final_matchup.push(current_shirts.read().await.get(&locked_matchup.shirt1.unwrap()).unwrap().clone());
        println!("New final bracket: {:?}", *locked_final_matchup);
    }

    let final_matchup = Some(final_matchup);

    Json(NextMatchupResponse {
        success: true,
        reason: None,
        prev_winner,
        final_matchup
    })
}

// post vote for current matchup. 0 = vote for first shirt, 1 = vote for second shirt
#[derive(Deserialize)]
#[serde(crate = "rocket::serde")]
pub struct VoteRequest {
    vote: Uuid,
}

#[post("/matchup/vote", data="<vote>")]
pub async fn post_matchup_vote(
    vote: Json<VoteRequest>, 
    uuid: ClientUuid, 
    player_list: &State<PlayerList>,
    current_state: &State<SharedGameState>,
    current_matchup: &State<CurrentMatchup>,
) -> Json<GenericResponse> {
    // Check player exists
    let parsed_uuid: Uuid = uuid.into();
    if !player_list.read().await.contains_key(&parsed_uuid) {
        return Json(GenericResponse { 
            success: false, 
            reason: Some(String::from_str("Requested player does not exist").unwrap())
        });
    }

    // Make sure we are in Battle state
    if *current_state.read().await != GameState::Battle && *current_state.read().await != GameState::FinalBattle {
        return Json(GenericResponse { 
            success: false, 
            reason: Some(String::from_str("You can't vote right now!").unwrap())
        });
    }

    let _ = current_matchup.write().await.votes.insert(parsed_uuid, vote.vote);

    return Json(GenericResponse { 
        success: true, 
        reason: None
    });

}

// get status of vote for wii
#[get("/matchup/vote")]
pub async fn get_matchup_vote(client_uuid: ClientUuid, wii: &State<Wii>, current_matchup: &State<CurrentMatchup>) -> Json<PlayerSubmissionsResponse> {
    let parsed_uuid: Uuid = client_uuid.into();
    if *wii.read().await != parsed_uuid {
        return Json(PlayerSubmissionsResponse { 
            success: false, 
            reason: Some(String::from_str("Only wii can view this").unwrap()), 
            submissions: None 
        });
    }

    let submission_counts: Vec<PlayerSubmissionNumber> = current_matchup
        .read()
        .await
        .votes
        .iter()
        .map(|(_player_uuid, shirt_uuid)| shirt_uuid)
        .counts()
        .iter()
        .map(|(id, num_submitted)| PlayerSubmissionNumber {id: (*id).clone(), num_submitted: *num_submitted as u32})
        .collect();

    return Json(PlayerSubmissionsResponse { 
        success: true, 
        reason: None, 
        submissions: Some(submission_counts)
    });
}