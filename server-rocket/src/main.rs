mod routes;
mod player;
mod gamestate;
mod uuid;
mod shirt;

use std::collections::{HashMap, VecDeque};

use routes::*;
use gamestate::*;
use player::*;
use shirt::{CurrentRoundDrawings, FinalizedDrawings, CurrentRoundQuotes, FinalizedQuotes, CurrentRoundShirtOptions, CurrentRoundShirts, CurrentMatchup, Matchup, CurrentShirtBracket, FinalRoundShirts};
use ::uuid::uuid;

#[macro_use] extern crate rocket;

#[launch]
fn rocket() -> _ {
    rocket::build()
        .manage(PlayerList::new(HashMap::new()))
        .manage(Wii::new(uuid!("6b043b00-49d2-4832-be99-9d85b9cbe2cb")))
        .manage(SharedGameState::new(GameStateToShare::new()))
        .manage(Started::new(false))
        .manage(CurrentRoundDrawings::new(Vec::new()))
        .manage(FinalizedDrawings::new(HashMap::new()))
        .manage(CurrentRoundQuotes::new(Vec::new()))
        .manage(FinalizedQuotes::new(HashMap::new()))
        .manage(CurrentRoundShirtOptions::new(HashMap::new()))
        .manage(CurrentRoundShirts::new(HashMap::new()))
        .manage(CurrentMatchup::new(Matchup::new()))
        .manage(CurrentShirtBracket::new(VecDeque::new()))
        .manage(FinalRoundShirts::new(Vec::new()))
        .mount("/", 
            routes![
                post_join,
                get_game_state, post_game_state, get_game_start, post_game_start,
                get_players, post_players_character, post_players_quote,
                get_draw, get_draw_submissions, post_draw,
                get_write, get_write_submissions, post_write,
                get_shirt, get_shirt_options, get_shirt_submissions, post_shirt,
                get_matchup, get_matchup_vote, post_matchup, post_matchup_vote
            ])
        .register("/", catchers![default_catcher])
}