#include "live_text_osd.h"
#include "sample_osd.h"


#include <stdio.h>
#include <string.h>
#include <time.h>



char names[3][20] = { "Amir", "Atharva", "Rakesh" };
static int name_idx = 0;
static time_t last_switch_time = 0;


void updateLiveOsdText(uint8_t c_idx){
	time_t now;
	char *live_text;

	int width_acc = 0;
	unsigned int i, j;
	unsigned char input_ascii;
	void *real_addr;

	if(c_idx >= MPI_MAX_VIDEO_CHN_NUM){
		printf("Invalid channel index for live OSD update: %d\n", c_idx);
		return;
	}
	if(g_handle[c_idx][3] < 0 || p_canvas_attr_live[c_idx].canvas_addr == 0){
		printf("Live OSD not initialized for channel index: %d\n", c_idx);
		return;
	}

	now = time(NULL);

	if(difftime(now, last_switch_time) >= 5){
		last_switch_time = now;
		name_idx = (name_idx + 1) % 3;
	}

	live_text = names[name_idx];

	memset((void *)p_canvas_attr_live[c_idx].canvas_addr, 0, LIVE_OSD_SIZE_WIDTH * LIVE_OSD_SIZE_HEIGHT * 2);

	for(i=0;i<strlen(live_text);i++){
		input_ascii = live_text[i] -32;
		real_addr = (void *)p_font_ayuv + osdindex[input_ascii].image_offset;

		for(j=0;(UINT32)j < (osdindex[input_ascii].image_height);j++){
			memcpy((void *)((p_canvas_attr_live[c_idx].canvas_addr) +
			                (j * LIVE_OSD_SIZE_WIDTH * 2) + (width_acc * 2)),
			       real_addr, (osdindex[input_ascii].image_width * 2));
			real_addr = (real_addr + (osdindex[input_ascii].image_width * 2));
		}
		width_acc = width_acc + osdindex[input_ascii].image_width;
	}
	MPI_updateOsdCanvas(g_handle[c_idx][3]);
}