use serde::{Serialize, Deserialize};

#[derive(Serialize, Deserialize, Debug)]
#[serde(tag = "type")]
pub enum WsMessage {
    // Sent from client to server
    Join {name: String, magic: u32},
    CharacterUpdate {uuid: String, character: Character},
    QuoteUpdate {uuid: String, quote: String},
    StartGame {uuid: String},
    CancelStart {uuid: String},
    SubmitImage {uuid: String, bg_color: u32, image: String},
    SubmitQuote {uuid: String, quote: String},
    SubmitShirt {uuid: String, image: String, quote: String},
    Vote {uuid: String, vote: u32},

    // Sent from server to clients
    Uuid {uuid: String},
    CurrentState {state: State},
    TakenCharacters {taken_characters: Vec<Character>},
    ShirtOptions {images: Vec<Image>, quotes: Vec<Quote>},
    Result {success: bool, reason: String},
    Admin {is_admin: bool},
    
    // Sent from Wii to server
    WiiJoin { magic: u32 },
    SetState {uuid: String, state: State},
    StartNextMatchup {uuid: String},
    EndVoting {uuid: String},

    // Sent from server to Wii
    PlayerJoined {uuid: String, name: String},
    CharacterUpdated {uuid: String, character: Character},
    ImageAdded {uuid: String, total: u32},
    QuoteAdded {uuid: String, total: u32},
    ShirtAdded {uuid: String},
    NumMatchups {remaining_matchups: u32},
    Matchup {shirt1: Shirt, shirt2: Shirt},
    VoteResults {shirt1: u32, shirt2: u32}

    // SetEmotion {uuid: String, emotion: Emotion}
}

pub struct MpscMessage {
    pub client_id: String,
    pub msg: WsMessage
}

#[derive(Serialize, Deserialize, Debug, Clone, Copy)]
pub enum State {
    Lobby,
    Wait,
    Draw,
    Write,
    Vote,
    End
}

#[derive(Serialize, Deserialize, Debug)]
pub enum Character {
    Jesus,
    Whale,
    Horse,
    TwoFace,
    Pumpkin,
    Dino,
    Hamburger,
    Shark,
    MonaLisa,
    Panda,
    Fish,
    Frog,
    Butterfly,
    Basketball,
    Owl,
    FSM,
    Turkey,
    Dragon,
    UpsideDown,
    Undecided
}

#[derive(Serialize, Deserialize, Debug)]
pub struct Image {
    id: String,
    creator: String,
    data: Vec<u8>
}

#[derive(Serialize, Deserialize, Debug)]
pub struct Quote {
    id: String,
    creator: String,
    data: String
}

#[derive(Serialize, Deserialize, Debug)]
pub struct Shirt {
    id: String,
    creator: String,
    image: Image,
    quote: Quote
}

#[derive(Debug)]
pub struct Player {
    pub id: String,
    pub name: String,
    pub character: Character,
    pub victory_quote: String
}

#[derive(Debug)]
pub struct GameState {
    pub players_can_join: bool,
    pub player_join_magic: u32,
    pub state: State,
    // UUID of wii associated with the game
    pub wii: Option<String>,
    // Players. Each player can have multiple clients over 
    // the course of the game
    // TODO: How to make sure only one client active per player? Probably just verify in Rejoin
    pub players: Vec<Player>,
    pub admin_player: Option<String>,
    pub images: Vec<Image>,
    pub quotes: Vec<Quote>,
    pub shirts: Vec<Shirt>,
    pub winning_shirts: Vec<Shirt>,
}

impl GameState {
    pub fn new() -> GameState {
        GameState { 
            players_can_join: false,
            player_join_magic: u32::MAX,
            state: State::Lobby, 
            wii: None, 
            players: Vec::new(), 
            admin_player: None, 
            images: Vec::new(), 
            quotes: Vec::new(), 
            shirts: Vec::new(), 
            winning_shirts: Vec::new() 
        }
    }
}