export const base_url = import.meta.env.DEV ? "http://localhost" : "";

export async function send_json_post_request(url: string, body: any)
{
    console.log("Sending POST request to", url);
    let request: RequestInit = {
        method: "POST",
        headers: [
            ["Content-Type", "application/json"]
        ],
        body : JSON.stringify(body)
    };
    // TODO: If request fails, stop interval.
    let resp = await fetch(url, request);
    let resp_json: Promise<any> = resp.json();
    return resp_json;
}

export async function send_json_get_request(url: string)
{
    console.log("Sending GET request to", url);
    let request: RequestInit = {
        method: "GET",
        headers: [
            ["Content-Type", "application/json"]
        ],
    };
    // TODO: If request fails, stop interval.
    let resp = await fetch(url, request);
    let resp_json: Promise<any> = resp.json();
    return resp_json;
}

export async function send_raw_request(method: string, url: string, body: any)
{
    console.log("Sending ", method, " request to", url);
    let request: RequestInit = {
        method: method,
        body : body
    };
    // TODO: If request fails, stop interval.
    let resp = await fetch(url, request);
    let resp_json: Promise<any> = resp.json();
    return resp_json;
}