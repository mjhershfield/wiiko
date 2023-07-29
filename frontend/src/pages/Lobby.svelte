<script lang="ts">
    import { Avatar } from '@skeletonlabs/skeleton';
    import { ProfilePicture, profile_picture_data, type ProfilePictureData } from '../lib/profile_pictures';
    import { base_url, send_json_get_request, send_json_post_request } from "../lib/requests";
	import { id_store, pfp_store } from '../lib/stores';
    import { toastStore } from '@skeletonlabs/skeleton';
	import { onMount } from 'svelte';
    import { update_game_state } from '../lib/gamestate';

    let current_victory_quote: string = "";
    let victory_quote: string = "";
    let profile_pictures: ProfilePictureData[] = profile_picture_data;
    let admin: boolean = false;
    let selected_pfp: ProfilePicture = $pfp_store;

    onMount(()=>{
        update_lobby();
    });

    function determine_avatar_border(pfp: ProfilePicture): string
    {
        if (pfp == $pfp_store)
        {
            return "border-4 border-primary-500"
        }
        else
        {
            return "border-4 border-surface-300-600-token hover:!border-secondary-500"
        }
    }

    async function update_lobby()
    {
        let resp_json: LobbyResponse = await send_json_get_request(base_url+"/lobby?id="+$id_store);
        if (resp_json.success)
        {
            admin = resp_json.admin!;
            pfp_store.set(resp_json.pfp!);
            profile_pictures = profile_pictures;
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

    async function set_pfp(new_pfp: ProfilePicture)
    {
        if (selected_pfp == new_pfp)
        {
            return;
        }
        let resp_json: SuccessResponse = await send_json_post_request(base_url+"/lobby?id="+$id_store, {pfp: new_pfp});
        if (resp_json.success)
        {
            update_lobby();
            toastStore.clear();
            toastStore.trigger({
				message: "Profile picture updated",
				background: "variant-filled-success"
			});
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

    async function update_quote()
    {   
        if (victory_quote.length == 0)
        {
            return;
        }

        let resp_json: SuccessResponse = await send_json_post_request(base_url+"/lobby?id="+$id_store, {quote: victory_quote});
        if (resp_json.success)
        {
            update_lobby();
            toastStore.clear();
            toastStore.trigger({
				message: "Victory quote updated",
				background: "variant-filled-success"
			});
            current_victory_quote = victory_quote;
            victory_quote = "";
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

    async function start_game() {
        await fetch(base_url+"/start?id="+$id_store, {method:"POST"});
    }
</script>

<div class="space-y-10 text-center flex flex-col items-center p-4">
    <h2 class="h2">Choose your fighter</h2>
    <div class=" flex flex-row flex-wrap gap-4 justify-center">
        {#each profile_pictures as pfp}
            <Avatar 
                initials={pfp.initials} 
                border={determine_avatar_border(pfp.name)}
                width={"w-24"}
                rounded={"rounded-2xl"}
                on:click={()=>{set_pfp(pfp.name);}}
            />
        {/each}
    </div>
    <h2 class="h2">Set your victory quote</h2>
    <!-- TODO: Fix the styling on this form -->
    <form>
        <input type="text" class="input p-2" bind:value={victory_quote} placeholder="{current_victory_quote}">
        <button class="btn variant-filled-primary" on:click|preventDefault={update_quote}>Submit</button>
    </form>

    {#if admin}
        <button class="btn variant-filled-secondary" on:click={start_game}>Start Game</button>
    {/if}
</div>