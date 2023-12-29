<script lang="ts">
    import type { DrawResponse, QuoteResponse, ShirtOptionsResponse, SuccessResponse } from "../lib/request_types";
    import { send_json_get_request, send_json_post_request } from "../lib/requests";
    import { onMount } from "svelte";
    import { uuid_store } from "../lib/stores";
    import { toastStore } from "@skeletonlabs/skeleton";
    import Wait from "./Wait.svelte";

    interface UserContent {
        id: string,
        data: string
    }

    let images: UserContent[] = [];
    let quotes: UserContent[] = [];

    let submitted: boolean = false;

    let selected_quote_index = 0;
    let selected_image_index = 0;

    function increment_quote() {
        selected_quote_index = (selected_quote_index + 1) % quotes.length;
    }

    function decrement_quote() {
        selected_quote_index = selected_quote_index - 1;
        if (selected_quote_index < 0) selected_quote_index = quotes.length - 1;
    }

    function increment_image() {
        selected_image_index = (selected_image_index + 1) % images.length;
    }

    function decrement_image() {
        selected_image_index = selected_image_index - 1;
        if (selected_image_index < 0) selected_image_index = images.length - 1;
    }

    async function submit_shirt() {
		let resp_json: SuccessResponse = await send_json_post_request("/shirt", $uuid_store, {drawing: images[selected_image_index].id, quote: quotes[selected_quote_index].id});
		if (resp_json.success)
		{
			toastStore.clear();
			toastStore.trigger({
				message: "Shirt submitted successfully",
				background: "variant-filled-success"
			});
			submitted = true;
		}
		else
		{
			toastStore.clear();
			toastStore.trigger({
				message: "Error: " + resp_json.reason!,
				background: "variant-filled-error"
			});
		}
    }

    onMount(async () => {
        let resp_json: ShirtOptionsResponse = await send_json_get_request("/shirt/options", $uuid_store);

        let drawing_promises = resp_json.drawings.map(drawing_uuid => send_json_get_request("/draw?id="+drawing_uuid, $uuid_store));
        Promise.allSettled(drawing_promises).then((promises_arr) => {
            for (let i = 0; i < drawing_promises.length; i++) {
                console.log(i)
                let current_promise: PromiseSettledResult<DrawResponse> = promises_arr[i];
                if (current_promise.status == "fulfilled" && current_promise.value.success) {
                    images.push({id: resp_json.drawings[i], data: current_promise.value.drawing.data_base64});
                    images = images;
                }
            }
        })

        let quote_promises = resp_json.quotes.map(quote_uuid => send_json_get_request("/write?id="+quote_uuid, $uuid_store));
        Promise.allSettled(quote_promises).then((promises_arr) => {
            for (let i = 0; i < quote_promises.length; i++) {
                console.log(i)
                let current_promise: PromiseSettledResult<QuoteResponse> = promises_arr[i];
                if (current_promise.status == "fulfilled" && current_promise.value.success) {
                    quotes.push({id: resp_json.quotes[i], data: current_promise.value.quote.quote});
                    quotes = quotes;
                }
            }
        })
    });

</script>

{#if !submitted}
<div class="space-y-10 text-center flex flex-col items-center">
    <img src={images.length > 0 ? images[selected_image_index].data : ""}>
    <div class="flex-row">
        <button class="btn variant-filled" on:click={decrement_image}> ← </button>
        <button class="btn variant-filled" on:click={increment_image}> → </button>
    </div>
    <p>{quotes.length > 0 ? quotes[selected_quote_index].data : "loading..."}</p>
    <div class="flex-row">
        <button class="btn variant-filled" on:click={decrement_quote}> ← </button>
        <button class="btn variant-filled" on:click={increment_quote}> → </button>
    </div>

    <button class="btn variant-filled-primary" on:click|preventDefault={submit_shirt}>Submit</button>
</div>
{:else}
<Wait />
{/if}
