import type { Character } from "./characters"
import type { GameState } from "./gamestate"

interface SuccessResponse {
    success: boolean,
    reason: string
}

interface JoinResponse extends SuccessResponse {
    uuid: string,
    character: Character,
    admin: boolean
}

interface StateResponse extends SuccessResponse{
    state: GameState
}

type Uuid = string;

interface Drawing {
    data_base64: string,
    bg_color: number
}

interface Quote {
    quote: string
}

interface Shirt {
    drawing: Drawing,
    quote: Quote
}

interface ShirtOptionsResponse extends SuccessResponse {
    drawings: Uuid[],
    quotes: Uuid[]
}

interface DrawResponse extends SuccessResponse {
    drawing: Drawing
}

interface QuoteResponse extends SuccessResponse {
    quote: Quote
}

interface ShirtResponse extends SuccessResponse {
    shirt: Shirt
}

interface MatchupResponse extends SuccessResponse {
    shirt0: Uuid,
    shirt1: Uuid
}