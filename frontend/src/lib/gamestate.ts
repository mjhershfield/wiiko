export enum GameState
{
    Wait = 0,
    Lobby,
    Draw,
    Write,
    Shirt,
    Battle,
    End,
    NotJoined,
    InitialState
}

export function update_game_state()
{
    window.dispatchEvent(new Event("update_state"));
}