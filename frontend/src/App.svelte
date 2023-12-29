<script lang="ts">

	// The ordering of these imports is critical to your app working properly
	import '@skeletonlabs/skeleton/themes/theme-modern.css';
	// If you have source.organizeImports set to true in VSCode, then it will auto change this ordering
	import '@skeletonlabs/skeleton/styles/skeleton.css';
	// Most of your app wide CSS should be put in this file
	import './app.postcss';
	import { AppShell, AppBar } from '@skeletonlabs/skeleton';
	import { Toast } from '@skeletonlabs/skeleton';
	import { uuid_store, character_store, username_store } from "./lib/stores";
	import { GameState } from "./lib/gamestate";
	import { onMount } from 'svelte';
	import { send_json_get_request } from "./lib/requests"
	import { character_data } from './lib/characters';
	import Draw from './pages/Draw.svelte';
	import Lobby from './pages/Lobby.svelte';
	import Root from './pages/Root.svelte';
	import Wait from './pages/Wait.svelte';
	import Write from './pages/Write.svelte';
    import type { StateResponse } from './lib/request_types';
    import Shirt from './pages/Shirt.svelte';
    import Vote from './pages/Vote.svelte';

	// TODO: Only navigate if new state != current state. Set an invisible <a>'s href to the link
	// we want to visit and click it.
	let current_state: GameState = GameState.InitialState;

	// TODO: Get state and redirect on initial page load (clear id, username if invalid)
	onMount(() => {
		window.addEventListener("update_state", update_game_state);
		update_game_state();
	});

	async function update_game_state() {
		let resp_json: StateResponse = await send_json_get_request("/game/state", $uuid_store);
		let new_state: GameState;
		if (resp_json.success)
		{
			new_state = resp_json.state!;
		}
		else
		{
			new_state = GameState.NotJoined;
			username_store.set("");
			uuid_store.set("");
			character_store.set(null);
		}

		if (current_state != new_state)
		{
			current_state = new_state;
			console.log("New state: ", GameState[current_state]);
		}

		if (current_state != GameState.NotJoined) {
			setTimeout(update_game_state, 2000)
		}
	}
</script>

<!-- App Shell -->
<AppShell>
	<svelte:fragment slot="header">
		<!-- App Bar -->
		<AppBar>
			<svelte:fragment slot="lead">
				<strong class="text-xl uppercase">WiiKO</strong>
			</svelte:fragment>
			<svelte:fragment slot="trail">
				<p>{$character_store == null ? "" : character_data[$character_store].initials}</p>
				<strong class="text-xl uppercase">{$username_store}</strong>
			</svelte:fragment>
		</AppBar>
	</svelte:fragment>
	<!-- Page Route Content -->
	<div class="container h-full mx-auto flex flex-col justify-center items-center p-4">
		{#if current_state == GameState.NotJoined}
			<Root />
		{:else if current_state == GameState.Lobby}
			<Lobby />
		{:else if current_state == GameState.Draw}
			<Draw />
		{:else if current_state == GameState.Write}
			<Write />
		{:else if current_state == GameState.Shirt}
			<Shirt />
		{:else if current_state == GameState.Battle || current_state == GameState.FinalBattle}
			<Vote />
		{:else}
			<Wait />
		{/if}
	</div>
</AppShell>

<Toast max={1}/>

