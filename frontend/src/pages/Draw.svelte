<script lang="ts">
    import { update_game_state } from '../lib/gamestate';
	import { base_url, send_json_get_request, send_json_post_request, send_raw_request } from '../lib/requests';
	import { id_store } from '../lib/stores';
	import { toastStore } from '@skeletonlabs/skeleton';
	
	let colors = [
		'#ce2029',
		'#d58141',
		'#d7c44c',
		'#3f8d27',
		'#4fa9cc',
		'#9c50b6',
		'#653818',
		'#000000',
		'#ffffff'
	]
	let background = '#fff'
	let brush_color = colors[0]

	let brush_sizes = [5, 10, 15];
	let brush_size: number = brush_sizes[1];

	let submitted: boolean = false;
	let submit_timeout: number = setTimeout(submit_image, 1000000);

	import { onMount } from 'svelte'
	
	let width = 300
	let height = 300
		
	let canvas: HTMLCanvasElement;
	let context: CanvasRenderingContext2D;
	let isDrawing: boolean;
    interface Coord {
        x: number,
        y: number
    }
    interface OffsetCoord {
        offsetX: number,
        offsetY: number
    }
	let start: Coord;
	
	let t: number, l: number;
	
	onMount(() => {
		context = canvas.getContext('2d')!;
		context.lineWidth = brush_size;
		context.lineCap = "round";
		context.fillStyle = background;
		context.fillRect(0, 0, width, height);
		context.strokeStyle = brush_color;	
		handleSize();
		update_draw();
	})
	
	const handleStart = (({ offsetX: x, offsetY: y }: OffsetCoord) => { 
		isDrawing = true
		start = { x, y }
	})
	
	const handleEnd = () => { isDrawing = false; context.closePath()}
	const handleMove = (({ offsetX: x1, offsetY: y1 }: OffsetCoord) => {
		if(!isDrawing) return
		
		const { x, y } = start
		context.beginPath()
		context.moveTo(x, y)
		context.lineTo(x1, y1)
		context.stroke()
		
		start = { x: x1, y: y1 }
	})
	
	const handleSize = () => {
		const { top, left } = canvas.getBoundingClientRect()
		t = top
		l = left
	}

	function color_button_style(color: string): string
	{
		let style: string = "w-7 h-7 cursor-pointer rounded-full m-0";
		style = style + (color == brush_color ? " border-4 border-primary-500" : "");
		return style;
	}

	async function submit_image()
	{
		canvas.toBlob(send_image_blob, 'image/jpeg', 0.5);
	}

	async function send_image_blob(blob: Blob|null)
	{
		if (blob == null)
		{
			console.log("Image failed to save");
			return;
		}
		let resp_json: SuccessResponse = await send_raw_request("POST", "/draw?id="+$id_store, blob);
		if (resp_json.success)
		{
			toastStore.clear();
			toastStore.trigger({
				message: "Drawing submitted successfully",
				background: "variant-filled-success"
			});
			update_draw();
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

	async function update_draw()
    {
        let resp_json: DrawResponse = await send_json_get_request("/draw?id="+$id_store);
        if (resp_json.success)
        {
            submitted = resp_json.submitted!;
			clearTimeout(submit_timeout);
			if (!submitted)
			{
            	submit_timeout = setTimeout(submit_image, resp_json.timeout! - 5000);
				console.log("Auto-submitting in ", (resp_json.timeout!-5000)/1000," seconds");
				console.log("Updating state in ", (resp_json.timeout! + 1000)/1000, " seconds");
				setTimeout(update_game_state, resp_json.timeout!+1000);
			}
			else
			{
				console.log("Submitted", (resp_json.timeout!-1000)/1000, " seconds left");
			}
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

<svelte:window on:resize={handleSize} />

<!-- TODO: buttons to change background and pen size -->
{#if !submitted}
<div class="flex flex-col max-w-md">
	<h2 class="h2 mb-4">Draw</h2>
	<canvas
		{width}
		{height}
		style:background
		bind:this={canvas} 
		on:mousedown={handleStart}	
		on:touchstart={e => {
			const { clientX, clientY } = e.touches[0]
			handleStart({
				offsetX: clientX - l,
				offsetY: clientY - t
			})
		}}	
		on:mouseup={handleEnd}				
		on:touchend={handleEnd}				
		on:mouseleave={handleEnd}
		on:mousemove={handleMove}
		on:touchmove={e => {
			const { clientX, clientY } = e.touches[0]
			handleMove({
				offsetX: clientX - l,
				offsetY: clientY - t
			})
		}}
	/>

	<section>
		<div>
			{#each colors as color}
			<button 
				on:click="{() => {
					brush_color = color
					context.strokeStyle = brush_color;
					colors = colors;
				}}"
				style:background={color}
				class={color_button_style(color)}>
				<span class="visually-hidden">
					Select the color {color}
				</span>
			</button>
			{/each}
		</div>
	</section>
	<button class="btn variant-filled-primary mt-8" on:click={submit_image}>Submit</button>
</div>
{:else}
<h1 class="h1">WiiKO</h1>
{/if}



<style>
	section {
		--size: 1.75rem;
		padding: 0.25rem;
		display: flex;
		align-items: center;
		gap: 0 1rem;
	}
	
	section > div {
		flex-grow: 1;
	}
	
	div {
		display: flex;
		gap: 0 0.5rem;
		align-items: center;
		overflow-x: clip;
	}

	.visually-hidden {
		clip-path: inset(50%);
	}
</style>