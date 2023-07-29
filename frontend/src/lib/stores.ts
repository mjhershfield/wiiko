import type { Writable } from 'svelte/store';
import { localStorageStore } from '@skeletonlabs/skeleton';
import { ProfilePicture } from "./profile_pictures"

export const id_store: Writable<number> = localStorageStore('player_id', 0);
export const username_store: Writable<string> = localStorageStore('username', '');
export const pfp_store: Writable <ProfilePicture> = localStorageStore('pfp', ProfilePicture.None);