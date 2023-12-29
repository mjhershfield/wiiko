export enum GameState
{
    Lobby = 0,
    Wait,
    Draw,
    Write,
    Shirt,
    Battle,
    FinalBattle,
    End,
    NotJoined,
    InitialState
}

export function update_game_state()
{
    window.dispatchEvent(new Event("update_state"));
}