<script lang="ts">
    import { Avatar } from '@skeletonlabs/skeleton';
    import { Character, character_data, type CharacterData } from '../lib/characters';
    import { send_json_post_request } from "../lib/requests";
	import { uuid_store, character_store, admin_store } from '../lib/stores';
    import { toastStore } from '@skeletonlabs/skeleton';
    import type {SuccessResponse} from '../lib/request_types'

    let current_victory_quote: string = "";
    let victory_quote: string = "";
    let characters: CharacterData[] = character_data;
    let selected_character: Character = $character_store;

    function determine_avatar_border(pfp: Character): string
    {
        if (pfp == $character_store)
        {
            return "border-4 border-primary-500"
        }
        else
        {
            return "border-4 border-surface-300-600-token hover:!border-secondary-500"
        }
    }

    async function set_pfp(new_pfp: Character)
    {
        if (selected_character == new_pfp)
        {
            return;
        }
        let resp_json: SuccessResponse = await send_json_post_request("/players/character", $uuid_store, {new_character: new_pfp});
        if (resp_json.success)
        {
            character_store.set(new_pfp);
            characters = characters;
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

        let resp_json: SuccessResponse = await send_json_post_request("/players/quote", $uuid_store, {new_quote: victory_quote});
        if (resp_json.success)
        {
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
        let resp_json: SuccessResponse = await send_json_post_request("/game/start", $uuid_store, {starting: true});
        if (!resp_json.success)
        {
            toastStore.clear();
            toastStore.trigger({
				message: "Error: " + resp_json.reason!,
				background: "variant-filled-error"
			});
        }
    }
</script>

<div class="space-y-10 text-center flex flex-col items-center p-4">
    <h2 class="h2">Choose your fighter</h2>
    <div class=" flex flex-row flex-wrap gap-4 justify-center">
        {#each characters as pfp}
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

    {#if $admin_store}
        <button class="btn variant-filled-secondary" on:click={start_game}>Start Game</button>
    {/if}
</div>