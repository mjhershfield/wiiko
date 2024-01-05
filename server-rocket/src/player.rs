use std::collections::HashMap;

use rand::{thread_rng, Rng};
use rocket::{serde,  tokio::sync::RwLock};
use serde_repr::{Serialize_repr, Deserialize_repr};
use strum::EnumCount;
use strum_macros::{FromRepr, EnumCount};
use uuid::Uuid;

#[derive(Serialize_repr, Deserialize_repr, Clone, Copy, FromRepr, EnumCount, Debug)]
#[serde(crate = "rocket::serde")]
#[repr(u8)]
pub enum Character {
    Whale = 0,
    Horse,
    TwoFace,
    // Pumpkin,
    // Dino,
    Hamburger,
    // Shark,
    MonaLisa,
    Fish,
    Butterfly,
    Turkey,
    UpsideDown,
    Robot,
    Frog,
    Superhero,
    Dad
}

#[derive(Serialize_repr, Deserialize_repr, Clone, Copy, FromRepr, EnumCount, Debug)]
#[serde(crate = "rocket::serde")]
#[repr(u8)]
pub enum Emotion {
    Default = 0,
    Happy,
    Sad,
    Angry,
    Shocked
}

impl Character {
    pub fn random() -> Character {
        let mut rng = thread_rng();
        let character_count: u8 = Character::COUNT.try_into().unwrap_or(0);
        Character::from_repr(rng.gen_range(0..character_count-1)).unwrap_or(Character::Whale)
    }
}

#[derive(Debug)]
pub struct Player {
    pub name: String,
    pub character: Character,
    pub victory_quote: String,
    pub emotion: Emotion,
    pub admin: bool
}

impl Player {
    pub fn new(name: String, admin: bool) -> Player {
        Player { name, character: Character::random(), victory_quote: String::new(), emotion: Emotion::Default, admin}
    }
}

pub type PlayerList = RwLock<HashMap<Uuid, Player>>;

pub type Wii = RwLock<Uuid>;