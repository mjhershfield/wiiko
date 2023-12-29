use std::collections::{HashMap, VecDeque};

use rocket::{tokio::sync::RwLock, serde::{Serialize, Deserialize}};
use uuid::Uuid;

#[derive(Serialize, Deserialize, Clone, Debug)]
#[serde(crate = "rocket::serde")]
pub struct Drawing {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub creator: Option<Uuid>,
    pub data_base64: String,
    pub bg_color: u32
}

impl Drawing {
    pub fn strip_creator(&self) -> Self {
        Drawing { creator: None, data_base64: self.data_base64.clone(), bg_color: self.bg_color }
    }
}

pub type CurrentRoundDrawings = RwLock<Vec<Drawing>>;
pub type FinalizedDrawings = RwLock<HashMap<Uuid, Drawing>>;


#[derive(Serialize, Deserialize, Clone, Debug)]
#[serde(crate = "rocket::serde")]
pub struct Quote {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub creator: Option<Uuid>,
    pub quote: String
}

impl Quote {
    pub fn strip_creator(&self) -> Self {
        Quote { creator: None, quote: self.quote.clone() }
    }
}

pub type CurrentRoundQuotes = RwLock<Vec<Quote>>;
pub type FinalizedQuotes = RwLock<HashMap<Uuid, Quote>>;

#[derive(Serialize, Deserialize, Clone, Debug)]
#[serde(crate = "rocket::serde")]
pub struct Shirt {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub creator: Option<Uuid>,
    pub drawing: Drawing,
    pub quote: Quote
}

impl Shirt {
    pub fn strip_creator(&self) -> Self {
        Shirt { creator: None, drawing: self.drawing.strip_creator(), quote: self.quote.strip_creator() }
    }
}

pub struct ShirtOptions {
    pub images: Vec<Uuid>,
    pub quotes: Vec<Uuid>
}

impl ShirtOptions {
    pub fn new() -> Self {
        ShirtOptions { images: Vec::new(), quotes: Vec::new() }
    }
}

pub type CurrentRoundShirtOptions = RwLock<HashMap<Uuid, ShirtOptions>>;

pub type CurrentRoundShirts = RwLock<HashMap<Uuid, Shirt>>;
pub type CurrentShirtBracket = RwLock<VecDeque<Uuid>>;
pub type FinalRoundShirts = RwLock<Vec<Shirt>>;

// Struct to store current shirt id's, votes for each
#[derive(Debug)]
pub struct Matchup {
    pub shirt0: Option<Uuid>,
    pub shirt1: Option<Uuid>,
    pub votes: HashMap<Uuid, Uuid>
}

impl Matchup {
    pub fn new() -> Self {
        Matchup { shirt0: None, shirt1: None, votes: HashMap::new() }
    }
}

pub type CurrentMatchup = RwLock<Matchup>;