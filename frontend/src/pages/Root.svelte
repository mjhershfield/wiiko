<div class="space-y-10 text-center flex flex-col items-center">
	<h2 class="h2">Join Game</h2>
	<form class="space-y-10">
		<input class="input p-2 {input_class}" title="Name" type="text" placeholder="Name" bind:value={username}/>
		<button class="btn variant-filled-primary" on:click|preventDefault={submit_form} bind:this={submit_btn}>Submit</button>
	</form>
</div>

<script lang="ts">
	import { toastStore } from "@skeletonlabs/skeleton";
	import { admin_store, character_store, username_store, uuid_store } from "../lib/stores";
	import { send_json_post_request } from "../lib/requests"
    import { update_game_state } from "../lib/gamestate";
	import type {JoinResponse} from "../lib/request_types"

	let username: string = "";
	let input_class: string = "";
	let submit_btn: HTMLButtonElement;

	async function submit_form()
	{
		input_class = "";
		let resp_json: JoinResponse = await send_json_post_request("/join", $uuid_store, {username: username.toLowerCase()});
		if (resp_json.success)
		{
			username_store.set(username);
			uuid_store.set(resp_json.uuid);
			character_store.set(resp_json.character);
			admin_store.set(resp_json.admin);
			toastStore.clear();
			toastStore.trigger({
				message: "Joined game successfully",
				background: "variant-filled-success"
			});
			update_game_state();
		}
		else
		{
			input_class = "input-error";
			toastStore.clear();
			toastStore.trigger({
				message: "Error: " + resp_json.reason!,
				background: "variant-filled-error"
			});
		}
	}
</script>
