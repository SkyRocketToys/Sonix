#include <queue.h>

/** \file snx_vc.h
 * Functions in this file as below :
 * \n  SONiX Video codec driver header file, which include ..., ... functions.
 * \n  
 * \n 
 * \author Raymond Chu
 * \date   2015-10-20 update
 */

// --------------------------------------------------------------------
//		codec hardware related definition
// --------------------------------------------------------------------



#define VC_STATE_PREPARED	0
#define VC_STATE_ENC		1
#define VC_STATE_DONE		2
#define VC_STATE_ACTIVE		3
#define VC_STATE_ERROR		4

#define FMT_H264				(0x1 << 1)	// 2
#define FMT_MJPEG				(0x1 << 2)	// 4
#define FMT_SNX420				(0x1 << 3)	// 8

#define FMT_NON_SET_KEYFRAME	(0x1 << 4)	// 16
#define FMT_NON_SET_ADAPTIVE 	(0x1 << 5)	// 32



/* Image is a keyframe (I-frame) */
#define FLAG_KEYFRAME			0x00000008

typedef	struct snx_vc_ds_position
{
	unsigned int	start_x;
	unsigned int	start_y;
}POSITION;

typedef	struct snx_vc_ds_dimension
{
	unsigned int	dim_x;
	unsigned int	dim_y;
}DIMENSION;

typedef	struct snx_vc_ds_color
{
	unsigned int	color_R;
	unsigned int	color_G;
	unsigned int	color_B;

}COLOR;

typedef	struct snx_vc_ds_color_attr
{
	int	weight;			    // range: 0~7 //
	int	mode;			   // range: 0~3 //
}COLOR_ATTR;

#define	BI_RGB			0
#define	BI_RLE8			1
#define	BI_RLE4			2
#define	BI_BITFIELDS		3

#define	BMP_ID_OFFSET			0x0
#define	BMP_SIZE_OFFSET			0x2
#define	BMP_DATAOFFSET_OFFSET		0xA
#define	BMP_HEADER_SIZE_OFFSET		0xE
#define	BMP_WIDTH_OFFSET		0x12
#define	BMP_HEIGHT_OFFSET		0x16
#define	BMP_PLANES_OFFSET		0x1A
#define	BMP_BIT_COUNT_OFFSET		0x1C
#define	BMP_COMPRESS_TYPE_OFFSET	0x1E
#define	BMP_IMAGE_SIZE_OFFSET		0x22
#define	BMP_COLORMAP_OFFSET		0x36

typedef	struct snx_cds_rgb_quad
{
	unsigned char blue;
	unsigned char green;
	unsigned char red;
	unsigned char reserved;
}RGB_QUAD;

struct snx_vc_ds {
	char bmp_file[64];
	char *string;
	struct snx_vc_ds_position pos;
	struct snx_vc_ds_dimension dim;
	struct snx_vc_ds_color t_color;
	struct snx_vc_ds_color b_color;
	struct snx_vc_ds_color bmp_threshold;

	struct snx_vc_ds_color_attr attr;
	unsigned int enable;
	unsigned int scale;

	size_t width;
	size_t height;
	// bmp used
	unsigned char *ds_src;
	int bmp_size;

	unsigned short planes;		// number of color planes
	unsigned short bit_count;	// number of bits per pixel
	unsigned int compress_type;
	unsigned int image_size;	// size of image data
	unsigned int hw_size;		// size of image data
	unsigned int transparent_mode;
	unsigned int transparent_value;
	unsigned int transparent_level;
	size_t font_width;
	size_t font_height;
	struct snx_cds_rgb_quad bmp_rgb[256];
	unsigned int pallet[256];
	unsigned char font_table_path[128];

	uint8_t pallet_init;
	uint32_t *image_buffer; 
	volatile uint8_t *ds_buffer; 
	uint32_t pre_image_size;
	uint8_t bmp_rounding;
	uint8_t bytes_per_line[64];
	uint8_t pixels_per_line[64];
	uint32_t str_unicode[128];
	uint8_t str_len;
};

struct snx_rc_mroi
{
	unsigned int region_en;
//	unsigned int region_num;
	unsigned char weight; 
	int qp;
	unsigned int pos_x;
	unsigned int dim_x;
	unsigned int pos_y;
	unsigned int dim_y;
	unsigned char ext_size;	
};

struct snx_rc_ext
{
	char mdrc_en;			//motion detection rate contorl enable (default 1)
	char md_m;				//corner width MD block  ( < 1/2*(16), default 3 )
	char md_n;				// corner height MD block ( < 1/2*(12), default 3 )
	char md_th;				// corner trigger counter ( <= (md_m * md_n), default 1 )

	int md_recover;			//finish MD, wating frame counter (default 25)
	int md_can_add_bitrate;	// MD enable, add bitrate ( Kbps, default 0 )

	char md_max_fps;			// MD enable, output frame rate ( < sensor fps, default 8 )
	char md_2dnr;			// MD enable, blured level ( 0 ~ 4, default 1 )
	char rc_up;				// max bit rate == bit_rate * (1 + (rc_up /32)) (default 6 )
	char rc_rate;			// bit_rate_exceed = bitrate_exceed *(1 - (1/ rc_rate)) (default 50)
	char delta_qp;			// Max QP of Pframe = QP of Iframe + delta_qp (default ipcam = 8 , dashcam = 5)

	char delta_qp_stop_cnt;
	
	char mbrc_en;
	char skip_rc_cycle;
	char rc_update;

	char md_cnt_en;
	char md_cnt_sum_th;
	char md_cnt_gop_multiple;
	char md_cnt_qp;
	char md_cnt_absy;
	char snx_msg_id;


	int md_isp_nr;			// finish MD, wating blured frame counter (default 23)
	// rc 
	int bps;
	int isp_fps;
	int codec_fps;

	int Iframe_UpBound ;	//

	int md_cnt_th;

	int md_cnt_bps;
	int md_cnt_bps2;

	int md_cnt_count;
	int md_cnt_lowbound;

	unsigned char rc_max_qp;
	unsigned char rc_min_qp;

	//////////////Ryan add 20140406, For extend QP
	int pframe_sum_num;		// how many frames to count average P frame size
	int maxqp_range;		// start counting from MAX_QP - MaxQP_Range, ex. MAX_QP=36, MaxQP_Range=2 -> counting from QP >= 34
	int extend_max_qp;		// new max QP after extending QP
	int upperpframesize;	// when PFrameAve > UpperPFrameSize, function will be turn on

};

struct snx_rc
{
	int codec_fd;
	size_t width;
	size_t height;
	int gop;
	int frames_gop;
	int Targetbitrate;	
	int rtn_quant;
	int Previous_frame_size;
	int BitRemain;
	int IFrameAve;
	int IntraPreDeviation;
	int FirstPQP;
	int IntraQP;
	int FrameCntRemain;
	int framerate;
	int total_quant_gop;
	int QpAdjEnable;
	int QpAdjStopCnt;
	int QpDifSum;
	int NFirstGOP;
	int IFrameCnt;
	int CurSecHaveIFrame;
	int TotalBitPerSec;
	int frames;
	int current_frame_size;

//initial value
	int UpBoundUp;
//	int UpBoundDw;
	int ConvergenceRate;
	int QPaddUpBound;
	
	int QPStopCnt;
	int SumQPBound;
	int UpBoundBytesPerSec;
	int BitExceed;
	int RealBytesPerSec;
	int SumQPCnt;
	int I_frame_size[4];
	int bufsize;

	// Ryan add 20150313, For skip rate control
	int AveTarFrameSize; 
	int RC_Calculate_Cycle;
	int RC_Skip_Enable;
	
	// Ryan add 2014/04/07
	int Pre_Framerate;
	int md_flag;	// re-init rate contorl function
	
	// Ryan add 2015/04/27
	int Iframe_Control ;
	int AVE_quant ;
	int Iframe_UpBound ;
	int total_quant_per_second;
	int AVE_quant_per_second ;
	int AVE_quant_Cnt;

	int stable_count;
	int in_md;
	int can_add_bitrate;
	int Tempbitrate;
	
/////////////////////
	//Ryan add 20140406, For extend QP
	unsigned int P_frame_size[30];
	int pframe_ave;		// average P frame size
	int pframe_cnt;		// internal argument
//	int Pframe_Sum_Num;	// how many frames to count average P frame size
//	int MaxQP_Range;	// start counting from MAX_QP - MaxQP_Range, ex. MAX_QP=36, MaxQP_Range=2 -> counting from QP >= 34
//	int Extend_Max_QP;	// new max QP after extending QP
//	int UpperPFrameSize;	// when PFrameAve > UpperPFrameSize, extend QP function will be turn on
/////////////////////

	// 2015 0502
	int md_cnt_count;	
	int md_cnt_down;
	int md_cnt_bps;
	int md_cnt_lowlux;
	
	int md_cnt;
	
	int iframe_large;

//MBRC
	unsigned int predbit;
	unsigned int mb_num;
	unsigned char ratio[4];
	unsigned int bs_target_sz[4];
	int mad_sum;
	int mad_ave;
	unsigned int qp_part;
	unsigned int bs_actual0;
	unsigned int bs_actual1;
	unsigned int bs_actual2;
	unsigned int bs_actual3;
	unsigned int current_slice_size[4];
	unsigned int previous_slice_size[4];
	unsigned int current_slice_qp[4];
	unsigned int previous_slice_qp[4];
	unsigned int current_par_slice_size[4];
	unsigned int previous_par_slice_size[4];
	unsigned char par_qp_adj[7];
	unsigned char par_qp_num[8];
	unsigned char mb_qp_adj[6];
	unsigned char mb_qp_num[7];
	unsigned char mb_qp_max;
	unsigned char mb_qp_min;
	unsigned int mb_mode;
	unsigned int mb_thd;
	unsigned char skin_det_en;
	unsigned char skin_qp_sub1;
	unsigned char skin_qp_sub2;
	unsigned char scene_chd_det_en;
	unsigned char scene_chd_det_adj;
	unsigned char scene_chd_det_num;
	uint8_t mbrc_init;
	
//MROI
	unsigned int sum_area;
	uint8_t mroi_en;
	struct snx_rc_mroi mroi_region[8];
	struct snx_rc_ext snx_rc_ext;
	
};

struct snx_h264_priv {
	unsigned int idr_period;

	// HW relative
	unsigned int	ahb_w_burst;
	unsigned int	ahb_r_burst;
	unsigned char	intr_mask;
	unsigned char	intr_en;
	unsigned char	frame_type;
	unsigned char	nonref_frame;
	unsigned int	multiple_ref_p;
	unsigned int	i_frame_qp;
	unsigned int	p_frame_qp;
	unsigned int	poc_num;
	unsigned int	frame_num_idr_pic_id;
	unsigned char	fmc_flag;
	unsigned char	symbol_mode;
	unsigned char	idr_flag;
	unsigned char	sps_en;
	unsigned char	pps_en;
	unsigned int	profile_idc;
	unsigned char	constraint_set0_flag;
	unsigned char	constraint_set1_flag;
	unsigned char	constraint_set2_flag;
	unsigned char	constraint_set3_flag;
	unsigned int	level_idc;

	unsigned char	mb_qp_generrator;

	unsigned int rec_mb_y_offset;
	unsigned int ref_mb_y_offset;
	unsigned int rec_ref_max_mb_y_offset;

	unsigned int dynamic_search_range_x;
	unsigned int dynamic_search_range_y;

	unsigned char  fast_fme_mode;
	unsigned char  i4_mode_num_i;
	unsigned char  i4_mode_num_p;
	unsigned char  mslice_info_sei_en;
	unsigned int slice_height_mb;
	unsigned char  fast_fme_en;
	unsigned char  intra_i16_dis;	// Disable intra i16
	unsigned char  intra_i4_dis;	// Disable intra i4
	unsigned char  pframe_intra_predict_dis;
	unsigned int adv_mvp_predict;
	unsigned int db_dis;		// Disable DB
	unsigned int up_db_dis;		// Disable up DB

	unsigned char frame_crop_left_offset;
	unsigned char frame_crop_right_offset;
	unsigned char frame_crop_top_offset;
	unsigned char frame_crop_bottom_offset;

	unsigned int frames;

	// Buffers
	volatile char *recref_buf;
	volatile char *recref_luma_buf;
	volatile char *recref_chroma_buf;

};
#ifdef ISP_DUMMY
struct snx_dummy {
	int bar_size;
	int snx_line_size;
	int addr;
	volatile char *src;
};
#endif

struct snx_vc_fps {
////////////////////////////////
	char fps_range;	//
	char fps_range_cur;	//	drop frame flag
	char fps_count;	//
	char fps_count_cur;	//	
	char fps_mode;	//
	char fps_cur;	//
	char fps_drop;	//	drop frame flag
	char dup_cnt;	//	duplicate output
////////////////////////////////	
};
struct snx_enc_buf {
	unsigned int ptr;
	unsigned int status;
	unsigned int size;
};

struct snx_vc_enc {

	int image_size;
	int width;
	int height;

	int dup;
	int bps;

	int gop;
	int multiple_ref_p;

	unsigned char nonref_frame;
	unsigned char channel;
	
	char mode;
	char isp_fps;
	char h264_fps;
	char jpeg_fps;

	char qp;
	char jpeg_qp;
	char keyframe;
	char scale;

	unsigned char vc_bufs;
	unsigned char vc_buf_index;

	unsigned char vc_h264_index;
	unsigned char vc_h264_get_index;

	unsigned char vc_jpeg_index;
	unsigned char vc_jpeg_get_index;
	
	unsigned char vc_h264_percent;
	unsigned char vc_jpeg_percent;


	unsigned int *src_ptr;
	
	struct snx_enc_buf *h264_ptr;
	struct snx_enc_buf *jpeg_ptr;
	
	volatile char *h264_bs;
	volatile char *jpeg_bs;
	char flags;

//	int jpeg_bs_size;
//	int h264_bs_size;
	int h264_skip_frames;


	struct snx_h264_priv snx_h264;
	struct snx_rc snx_rc;

	uint8_t ds_en;
	struct snx_vc_ds snx_vc_ds;


	struct snx_vc_fps vc_fps[3]; // H264=0 / JPEG=1 / YUV=2

};

struct snx_vc {

	char mode;
	char isp_fps;
	char scale;
	volatile char *mid;

	unsigned char channel;

	int width;
	int height;

	int image_size;

	struct snx_vc_enc vc_enc[2]; // 0: first encoder  1: secand encode (dup)

};
/**
 * \defgroup vc Video driver modules
 * \n This is the video driver
 * \n 
 * @{
 */

int snx_vc_init(struct snx_vc *vc);
int snx_vc_uninit(struct snx_vc *vc);
int snx_vc_start(struct snx_vc *vc);
int snx_vc_stop(struct snx_vc *vc);
int snx_vc_read(struct snx_vc *vc, QueueHandle_t recv);

int snx_vc_isp_fps(struct snx_vc_enc *vc_enc, int isp_fps);
int snx_vc_set_fps(struct snx_vc_enc *vc_enc, int type);
int snx_vc_set_bps(struct snx_vc_enc *vc_enc);
int snx_vc_set_keyframe(struct snx_vc_enc *vc_enc);
int snx_vc_set_gop(struct snx_vc_enc *vc_enc);
int snx_vc_get_gop(struct snx_vc_enc *vc_enc);
int snx_vc_set_qp(struct snx_vc_enc *vc_enc);
int snx_vc_get_qp(struct snx_vc_enc *vc_enc);

#ifdef ISP_DUMMY
int snx_dummy_read(struct snx_dummy *snx_dummy, struct snx_vc *vc, QueueHandle_t recv);
int snx_dummy_start(struct snx_dummy *snx_dummy, struct snx_vc *vc);
int snx_dummy_stop(struct snx_dummy *snx_dummy);
int snx_dummy_init(void);
int snx_dummy_uninit(void);
#endif

void snx_ic_ds_set_pallet(struct snx_vc_ds *cds);

/** @} */ //  vc Video driver modules
