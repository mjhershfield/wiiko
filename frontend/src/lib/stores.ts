import { writable, type Writable } from 'svelte/store';
import { localStorageStore } from '@skeletonlabs/skeleton';
import type { Character } from "./characters"

export const uuid_store: Writable<string> = localStorageStore('uuid', "");
export const username_store: Writable<string> = localStorageStore('username', "");
export const character_store: Writable <Character> = localStorageStore('character', null);
export const admin_store: Writable<boolean> = writable(false);