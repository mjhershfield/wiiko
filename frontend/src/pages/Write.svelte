<script lang="ts">
    import { send_json_post_request } from '../lib/requests';
	import { uuid_store } from '../lib/stores';
	import { toastStore } from '@skeletonlabs/skeleton';
	import type {SuccessResponse} from "../lib/request_types"
	
	let slogan: string;

	async function submit_slogan()
	{
		if (slogan == null)
		{
			return;
		}
		let resp_json: SuccessResponse = await send_json_post_request("/write", $uuid_store, {quote: slogan});
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