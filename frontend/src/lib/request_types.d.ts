interface SuccessResponse {
    success: boolean,
    reason: string | undefined
}

interface JoinResponse extends SuccessResponse {
    id: number | undefined
}

interface StateResponse extends SuccessResponse{
    state: number | undefined
}

interface LobbyResponse extends SuccessResponse {
    admin: boolean | undefined,
    pfp: number | undefined
}

interface DrawResponse extends SuccessResponse {
    timeout: number | undefined,
    submitted: boolean | undefined
}

interface WriteResponse extends SuccessResponse {
    timeout: number | undefined
}