mod join;
mod game;
mod players;
mod draw;
mod write;
mod shirt;
mod matchup;

pub use join::*;
pub use game::*;
pub use players::*;
pub use draw::*;
pub use write::*;
pub use shirt::*;
pub use matchup::*;

use rocket::{serde::{Serialize, json::Json}, Request, http::Status};
use uuid::Uuid;

#[derive(Serialize)]
#[serde(crate = "rocket::serde")]
pub struct GenericResponse {
    success: bool,
    reason: Option<String>,
} 

#[catch(default)]
pub fn default_catcher(status: Status, _request: &Request) -> Json<GenericResponse>{
    println!("{:?}", _request);
    Json(GenericResponse { success: false, reason: Some(format!("Error code {}", status.code))})
}

#[derive(Serialize)]
#[serde(crate = "rocket::serde")]
pub struct PlayerSubmissionNumber {
    id: Uuid,
    num_submitted: u32
}

#[derive(Serialize)]
#[serde(crate = "rocket::serde")]
pub struct PlayerSubmissionsResponse {
    success: bool,
    reason: Option<String>,
    submissions: Option<Vec<PlayerSubmissionNumber>>
}