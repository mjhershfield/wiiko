<script lang="ts">
    import { toastStore } from "@skeletonlabs/skeleton";
    import type { Drawing, MatchupResponse, Quote, ShirtResponse, Uuid, Shirt, SuccessResponse } from "../lib/request_types";
    import { send_json_get_request, send_json_post_request } from "../lib/requests";
    import { uuid_store } from "../lib/stores";
    import { onMount } from "svelte";
    import Wait from "./Wait.svelte";

    let shirt0_uuid: Uuid, shirt1_uuid: Uuid;
    let shirt0: Shirt, shirt1: Shirt;
    let matchup_loaded = false;

    onMount(async () => {
        while (!matchup_loaded) {
            let resp_json: MatchupResponse = await send_json_get_request("/matchup", $uuid_store);
            if (!resp_json.success)
            {
                await new Promise(r => setTimeout(r, 2000));
                continue;
            }
            shirt0_uuid = resp_json.shirt0;
            shirt1_uuid = resp_json.shirt1;
            let shirt0_resp: ShirtResponse = await send_json_get_request("/shirt?id="+shirt0_uuid, $uuid_store);
            if (shirt0_resp.success)
            {
                shirt0 = shirt0_resp.shirt;
            }
            let shirt1_resp: ShirtResponse = await send_json_get_request("/shirt?id="+shirt1_uuid, $uuid_store);
            if (shirt1_resp.success)
            {
                shirt1= shirt1_resp.shirt;
            }
            matchup_loaded = resp_json.success
        }
    })

    async function vote_shirt0() {
        let resp_json: SuccessResponse = await send_json_post_request("/matchup/vote", $uuid_store, {vote: shirt0_uuid})
        if (!resp_json.success) {
            toastStore.clear();
            toastStore.trigger({
                message: "Error: " + resp_json.reason!,
                background: "variant-filled-error"
            });
        }
    }

    async function vote_shirt1() {
        let resp_json: SuccessResponse = await send_json_post_request("/matchup/vote", $uuid_store, {vote: shirt1_uuid})
        if (!resp_json.success) {
            toastStore.clear();
            toastStore.trigger({
                message: "Error: " + resp_json.reason!,
                background: "variant-filled-error"
            });
        }
    }
</script>

{#if matchup_loaded}
    <div class="space-y-10 text-center flex flex-col items-center">
        <button class="btn variant-filled-primary" on:click={vote_shirt0}>
            <img src={shirt0.drawing.data_base64} class="h-14 m-4">
            {shirt0.quote.quote}
        </button>
        <button class="btn variant-filled-primary" on:click={vote_shirt1}>
            <img src={shirt1.drawing.data_base64} class="h-14 m-4">
            {shirt1.quote.quote}
        </button>
    </div>
{:else}
    <Wait />
{/if}
