export const base_url = import.meta.env.DEV ? "http://localhost:8000" : window.location.origin;

export async function send_json_post_request(path: string, uuid: string, body: any)
{
    let request: RequestInit = {
        method: "POST",
        headers: [
            ["Content-Type", "application/json"],
            ["x-client-uuid", uuid]
        ],
        body : JSON.stringify(body)
    };
    // TODO: If request fails, stop interval.
    let url: string = base_url + path
    console.log("Sending POST request to", url);
    let resp = await fetch(url, request);
    let resp_json: Promise<any> = resp.json();
    return resp_json;
}

export async function send_json_get_request(path: string, uuid: string)
{
    let request: RequestInit = {
        method: "GET",
        headers: [
            ["Content-Type", "application/json"],
            ["x-client-uuid", uuid]
        ],
    };
    // TODO: If request fails, stop interval.
    let url: string = base_url + path
    console.log("Sending GET request to", url);
    let resp = await fetch(url, request);
    let resp_json: Promise<any> = resp.json();
    return resp_json;
}

export async function send_raw_request(method: string, path: string, body: any)
{
    let request: RequestInit = {
        method: method,
        body : body
    };
    // TODO: If request fails, stop interval.
    let url: string = base_url + path
    console.log("Sending ", method, " request to", url);
    let resp = await fetch(url, request);
    let resp_json: Promise<any> = resp.json();
    return resp_json;
}