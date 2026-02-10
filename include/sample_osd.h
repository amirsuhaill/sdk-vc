#ifndef SAMPLE_OSD_H_
#define SAMPLE_OSD_H_

#ifdef __cplusplus
extern "C" {
#endif /**< __cplusplus */

#include "mpi_index.h"
#include "mpi_osd.h"

#define DISPLAYABLE_CHARACTER_NUMBERS 104
#define LIVE_OSD_SIZE_WIDTH 336
#define LIVE_OSD_SIZE_HEIGHT 32

typedef struct ascii_index {
    uint32_t image_offset;
    uint32_t image_size;
    uint32_t image_width;
    uint32_t image_height;
} ASCII_INDEX;


INT32 SAMPLE_createOsdUpdateThread(void);
INT32 SAMPLE_destroyOsdUpdateThread(void);
INT32 SAMPLE_stopOsd(MPI_CHN chn_idx);
INT32 SAMPLE_createOsd(bool visible, MPI_CHN chn_idx, INT32 output_num, UINT16 width, UINT16 height);
void SAMPLE_initOsd(void);
void SAMPLE_freeOsdResources(void);

extern OSD_HANDLE g_handle[MPI_MAX_VIDEO_CHN_NUM][MPI_OSD_MAX_BIND_CHANNEL];
extern MPI_OSD_CANVAS_ATTR_S p_canvas_attr_live[MPI_MAX_VIDEO_CHN_NUM];
extern ASCII_INDEX osdindex[DISPLAYABLE_CHARACTER_NUMBERS];
extern char *p_font_ayuv;

#ifdef __cplusplus
}
#endif /**< __cplusplus */

#endif /**< SAMPLE_OSD_H_ */