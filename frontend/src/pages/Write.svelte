<script lang="ts">
    import { onMount } from 'svelte';
    import { update_game_state } from '../lib/gamestate';
	import { base_url, send_json_get_request, send_json_post_request, send_raw_request } from '../lib/requests';
	import { id_store } from '../lib/stores';
	import { toastStore } from '@skeletonlabs/skeleton';
	
	let slogan: string;

	onMount(()=>{
		update_write();
	})

	async function update_write() {
		let resp_json: WriteResponse = await send_json_get_request("/write?id="+$id_store);
		if (resp_json.success)
		{
			console.log("Updating state in ", (resp_json.timeout! + 1000)/1000, " seconds");
			setTimeout(update_game_state, resp_json.timeout! + 1000);
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

	async function submit_slogan()
	{
		if (slogan == null)
		{
			return;
		}
		let resp_json: SuccessResponse = await send_json_post_request("/write?id="+$id_store, {slogan: slogan});
		if (resp_json.success)
		{
			toastStore.clear();
			toastStore.trigger({
				message: "Slogan submitted successfully",
				background: "variant-filled-success"
			});
			slogan = "";
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
</script>

<div class="space-y-10 text-center flex flex-col items-center">
	<h2 class="h2">Write Slogans</h2>
	<form class="space-y-10">
		<input class="input p-2" title="Name" type="text" placeholder="Name" bind:value={slogan}/>
		<button class="btn variant-filled-primary" on:click|preventDefault={submit_slogan}>Submit</button>
	</form>
</div>