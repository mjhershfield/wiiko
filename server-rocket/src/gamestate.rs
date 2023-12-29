use rocket::{serde, tokio::sync::RwLock};
use serde_repr::{Serialize_repr, Deserialize_repr};


#[derive(Serialize_repr, Deserialize_repr, Debug, Clone, Copy, PartialEq)]
#[serde(crate = "rocket::serde")]
#[repr(u8)]
pub enum GameState {
    Lobby = 0,
    Wait,
    Draw,
    Write,
    Shirt,
    Battle,
    FinalBattle,
    End
}

pub struct GameStateToShare {
    current_state: GameState,
    waiting: bool
}

impl PartialEq<GameState> for GameStateToShare {
    fn eq(&self, other: &GameState) -> bool {
        &self.get_state() == other
    }
}

impl GameStateToShare {
    pub fn new() -> GameStateToShare {
        GameStateToShare { current_state: GameState::Lobby, waiting: false }
    }

    pub fn set_state(&mut self, state: GameState) -> bool {
        self.waiting = state == GameState::Wait;
        if state != GameState::Wait && self.current_state != state {
            self.current_state = state;
            true
        } else {
            false
        }
    }

    pub fn get_state(&self) -> GameState{
        if self.waiting {
            GameState::Wait
        } else {
            self.current_state
        }
    }
}

pub type SharedGameState = RwLock<GameStateToShare>;
pub type Started = RwLock<bool>;