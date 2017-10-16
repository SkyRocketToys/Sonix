#ifndef __MID_VC_H__
#define __MID_VC_H__

//#include <snx_mid_isp.h>

extern int snx_isp_fps_set(int ch, int val);

/** \file snx_mid_vc.h
 * Functions in this file as below :
 * \n SONiX Video middleware header file, which include ..., ... functions.
 * \n  
 * \n 
 * \author Raymond Chu
 * \date   2015-11-23 update
 */

/** \class snx_m2m 
 * \brief SoNiX video struction
 * \n
 * @{
 */


struct snx_m2m {
	int channel;					//!< isp channel (default: 0)(range: 0 ~ 1)
	int width;						//!< image width (default: 640)(range: 160 ~ 1920)
	int height;						//!< image height (default: 480)(range: 120 ~ 1080)
	int isp_fps;					//!< isp frate rate. (default: 30)(range: 1 ~ 30)
	int isp_bufs;					//!< isp buffer number. (default: 2)(range: 2 ~ 4)
	int isp_mode;					//!< isp mode (default: VIDEO_PIX_FMT_SNX420)(support: VIDEO_PIX_FMT_SNX420)
//	char isp_only;					//!< isp output only (1: isp_only,  0: isp to vc)

	char start;						//!< video start/stop encode, status flag.
	
	struct snx_vc vc;				//!< video codec driver structure
#ifdef ISP_DUMMY
	struct snx_dummy dummy; 		//!< video dummy sensor structure 
#endif
	struct snx_frame_ctx ctx[2];	//!< video isp driver structure

	QueueHandle_t isp2vc; 			//!< isp to vc send addres (futur add isp_fps)

	QueueHandle_t h264;				//!< record file send address
	QueueHandle_t jpeg;				//!< record file send address
	QueueHandle_t h264_dup;			//!< record file send address
	QueueHandle_t jpeg_dup;			//!< record file send address
	
	unsigned int adaptive_fps_dec_time;		//!< adaptive fps Decrease time
	unsigned int adaptive_fps_inc_time;		//!< adaptive fps Increase time
	int adaptive_dec_counter;
	int adaptive_inc_counter;
	int adaptive_cur_fps;
	int adaptive_prev_fps;
	int adaptive_usr_fps;

	int resume_time;
	int suspend_time;
	int preview_ctrl_en;
	int preview_ctrl_dup;
	xTaskHandle preview_ctrl_task;
};
/** @} */ //  snx_m2m sonix memory to memory information struct

/**
 * \defgroup mid_vc Video middleware modules
 * \n This is the middleware video application interface
 * \n Get H264/JPEG stream example as follows
\verbatim

	snx_video_init (m2m);

	snx_isp_set_fps(m2m, isp_fps);
	snx_video_set_scale(m2m, scale);
	snx_video_set_mode(m2m, dup, mode );
	snx_video_set_fps(m2m, dup, fps, type); // set h264/jpeg fps value
	snx_video_set_bps(m2m, dup, bps); 
	snx_video_set_gop(m2m, dup, gop); // fps == gop
	snx_video_set_qp(m2m, dup, qp, type); // set h264/jpeg qp value

	snx_video_start(m2m);
	// set related parameter

	// Dynamic Modify mode/fps/bps/gop/qp
	snx_isp_set_fps(m2m, isp_fps);
	snx_video_set_scale(m2m, scale);
	snx_video_set_mode(m2m, dup, mode );
	snx_video_set_fps(m2m, dup, fps, type); // set h264/jpeg fps value
	snx_video_set_bps(m2m, dup, bps); 
	snx_video_set_gop(m2m, dup, gop); // fps == gop
	snx_video_set_qp(m2m, dup, qp, type); // set h264/jpeg qp value
	
	for (i = 0; i < frame_count; i++) {
		snx_video_read(m2m);
		//Get bit stream frame
		h264_size = snx_video_h264_stream(m2m, dup, &ptr);
		jpeg_size = snx_video_jpeg_stream(m2m, dup, &ptr);
		isp_size = snx_video_isp_stream(m2m, dup, &ptr);

		// Dynamic set bps/fps (optional)
		snx_video_set_fps(m2m, dup, fps, type);
		snx_video_set_bps(m2m, dup, bps);
	}
	snx_video_stop(vc);
	snx_video_uninit(vc);
\endverbatim
 * \n Get H264/JPEG stream middleware run task example as follows
\verbatim
	main() {
		...
		snx_video_task_start(m2m);
	
		... \\ task get frame
					
		snx_video_task_stop(m2m);
	}

	task_get_h264() {
		...
		recv = snx_video_task_recv(m2m, FMT_H264, dup);

		while(1) {
			if(*recv != NULL) {
		    	for ( ; ; ) {
		    		// The task is blocked until something appears in the queue 
					if (xQueueReceive(*recv, (void*) &ready, portMAX_DELAY)) {
		    			break;
					}
					else {
						printf("<<test>><%s><%d> vc2file timeout\n", __func__, __LINE__);
					}
				}

				if(m2m->start == 0)
					break;

				h264_size = snx_video_h264_stream(m2m, dup, &ptr);
				...	
			}
		}
	}
	task_get_jpeg() {
		...
		recv = snx_video_task_recv(m2m, FMT_MJPEG, dup);

		while(1) {
			if(*recv != NULL) {
		    	for ( ; ; ) {
		    		// The task is blocked until something appears in the queue 
					if (xQueueReceive(*recv, (void*) &ready, portMAX_DELAY)) {
		    			break;
					}
					else {
						printf("<<test>><%s><%d> vc2file timeout\n", __func__, __LINE__);
					}
				}

				if(m2m->start == 0)
					break;
				jpeg_size = snx_video_jpeg_stream(m2m, dup, &ptr);
				...	
			}
		}
	}
\endverbatim

 * \n 
 * @{
 */
/** @} */ //  mid_vc Video middleware modules

/** \defgroup vc_func Video functions
 * \ingroup mid_vc
 * \n  Video control flow API
 * @{
 */

int	snx_adaptive_fps(struct	snx_m2m	*m2m, int dup, int DecFpsTime, int IncFpsTime);

int snx_isp_start(struct snx_m2m *m2m);
int snx_isp_stop(struct snx_m2m *m2m);
int snx_isp_read(struct snx_m2m *m2m);


int snx_video_start(struct snx_m2m *m2m);
int snx_video_stop(struct snx_m2m *m2m);
int snx_video_read(struct snx_m2m *m2m);
int snx_video_init(struct snx_m2m *m2m);
int snx_video_uninit(struct snx_m2m *m2m);

int snx_video_task_start(struct snx_m2m *m2m);
int snx_video_task_stop(struct snx_m2m *m2m);
QueueHandle_t * snx_video_task_recv(struct snx_m2m *m2m,int mode,int dup);

int snx_420line_to_420(char *p_in, char *p_out, unsigned int width, unsigned int height);
int snx_420line_to_422(char *p_in, char *p_out, unsigned int width, unsigned int height);

unsigned int snx_cds_bmp_trans_level_scan(struct snx_vc_ds *cds, int show_all);

/** \fn void snx_preview_ctrl_time(struct snx_m2m *m2m, int dup, int resume_time, int suspend_time)
 * \note Video read function
 * \n
 * \param m2m : video middleware struct
 * \n
 * \return pdPASS/pdFAIL
 */
static inline void snx_preview_ctrl_time(struct snx_m2m *m2m, int dup, int resume_time, int suspend_time)
{
	m2m->preview_ctrl_dup = dup;
	m2m->preview_ctrl_en = 1;	// preview_ctrl_en 1: START MODE, 2 CLOSE MODE, 0 STOP MODE
	m2m->resume_time = resume_time;
	m2m->suspend_time = suspend_time;
}

static inline int snx_preview_get_resume(struct snx_m2m *m2m)
{	
	return m2m->resume_time;
}

static inline int snx_preview_get_suspend(struct snx_m2m *m2m)
{	
	return m2m->suspend_time;
}

/** \fn void snx_isp_set_fps(struct snx_m2m *m2m, int fps)
 * \note  Video set isp frame rate function
 * \n
 * \param m2m : video middleware struct
 * \param fps : isp frame rate (range: 1 ~ 30)
 * \n
 * \return pdPASS/pdFail
 */
static inline void snx_isp_set_fps(struct snx_m2m *m2m, int fps)
{	
	m2m->isp_fps =fps;
	if(m2m->start) {
		snx_isp_fps_set(m2m->channel, m2m->isp_fps);
	}
}

/** \fn int snx_isp_get_fps(struct snx_m2m *m2m)
 * \note  Video get isp frame rate function
 * \n
 * \param m2m : video middleware struct
 * \n
 * \return isp_fps
 */
static inline int snx_isp_get_fps(struct snx_m2m *m2m)
{	
	return m2m->isp_fps;
}

/** \fn void snx_video_set_mode(struct snx_m2m *m2m, int dup, int mode)
 * \note Video set mode function
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param mode : H264:FMT_H264, JPEG:FMT_MJPEG, H264+JPEG:FMT_H264+FMT_MJPEG,
 * \n
 * \return pdPASS/pdFail
 */
static inline void snx_video_set_mode(struct snx_m2m *m2m, int dup, int mode)
{	

//	if(m2m->start == 0) {
		m2m->vc.mode &= (0xF<<((dup==0)?4:0));
		m2m->vc.mode |= (mode<<((dup==0)?0:4));
//	}
	m2m->adaptive_fps_dec_time = 0;
	m2m->adaptive_fps_inc_time = 0;
	
}

/** \fn int snx_video_get_mode(struct snx_m2m *m2m, int dup)
 * \note Video get mode function
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \n
 * \return mode : H264:FMT_H264, JPEG:FMT_MJPEG, H264+JPEG:FMT_H264+FMT_MJPEG,
 */
static inline int snx_video_get_mode(struct snx_m2m *m2m, int dup)
{	
	return ((m2m->vc.mode>>((dup==0)?0:4)) & 0xF);
}


/** \fn struct timeval snx_video_get_timestamp(struct snx_m2m *m2m)
 * \note Video get ISP timestamp function.
 * \n 
 * \n
 * \param m2m : video middleware struct
 * \n           
 * \n
 * \return timestamp
 */
static inline struct timeval snx_video_get_timestamp(struct snx_m2m *m2m)
{	
	return m2m->ctx[m2m->channel].timestamp;
}


/** \fn void snx_video_set_bps(struct snx_m2m *m2m, int dup, int bps)
 * \note Video set H264 bit rate function.
 * \n Support dynamic change bit rate.
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param bps : H264 bit rate 
 * \n           if bps=0 --> QP mode
 * \n
 * \return pdPASS/pdFail
 */
static inline void snx_video_set_bps(struct snx_m2m *m2m, int dup, int bps)
{	
	m2m->vc.vc_enc[dup].bps = bps;
	snx_vc_set_bps(&m2m->vc.vc_enc[dup]);
}

/** \fn void snx_video_get_bps(struct snx_m2m *m2m, int dup)
 * \note Video get bit rate function.
 * \n Support dynamic change bit rate.
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \n           
 * \n
 * \return bps
 */
static inline int snx_video_get_bps(struct snx_m2m *m2m, int dup)
{	
	return m2m->vc.vc_enc[dup].bps;
}


/** \fn void snx_video_set_fps(struct snx_m2m *m2m, int dup, int fps, int type)
 * \note Video set encoder frame rate function
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param fps : codec frame rate (range: 1 ~ 30)
 * \param type : FMT_H264 or FMT_MJPEG
 * \n				FMT_NON_SET_KEYFRAME: non set key frame
 * \n
 * \return pdPASS/pdFail
 */
static inline void snx_video_set_fps(struct snx_m2m *m2m, int dup, int fps, int type)
{	
//	if(type == FMT_H264)
	if(type & FMT_H264) {
		m2m->vc.vc_enc[dup].h264_fps =fps;
		if((type & FMT_NON_SET_ADAPTIVE) == 0) {
			m2m->adaptive_fps_dec_time = 0;
			m2m->adaptive_fps_inc_time = 0;
			m2m->adaptive_usr_fps = fps;
		}
	}
	else //if(type == FMT_MJPEG)
		m2m->vc.vc_enc[dup].jpeg_fps =fps;
	
	if(m2m->start) {
		snx_vc_set_fps(&m2m->vc.vc_enc[dup], type);
	}
}




/** \fn int snx_video_get_fps(struct snx_m2m *m2m, int dup, int type)
 * \note Video get encoder frame rate function
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param type : FMT_H264 or FMT_MJPEG
 * \n
 * \return fps
 */
static inline int snx_video_get_fps(struct snx_m2m *m2m, int dup, int type)
{	
	if(type == FMT_H264)
		return m2m->vc.vc_enc[dup].h264_fps;
	else
		return m2m->vc.vc_enc[dup].jpeg_fps;
	
}
/** \fn void snx_video_set_resolution(struct snx_m2m *m2m, int width, int height)
 * \note Video set resolution function
 * \n
 * \param m2m : video middleware struct
 * \param width : width (range: 160 ~ 1920)
 * \param height : height (range: 120 ~ 1080)
 * \n
 * \return pdPASS/pdFail
 */
static inline void snx_video_set_resolution(struct snx_m2m *m2m, int width, int height)
{	
	if(m2m->start == 0) {
		m2m->width = width;
		m2m->height = height;
	}
}

/** \fn void snx_video_set_resolution(struct snx_m2m *m2m, int *width, int *height)
 * \note Video get resolution function
 * \n
 * \param m2m : video middleware struct
 * \param width : width (range: 160 ~ 1920)
 * \param height : height (range: 120 ~ 1080)
 * \n
 * \return pdPASS/pdFail
 */
static inline void snx_video_get_resolution(struct snx_m2m *m2m, int *width, int *height)
{	
	*width = m2m->width;
	*height = m2m->height;
}


/** \fn void snx_video_set_scale(struct snx_m2m *m2m, int scale)
 * \note Video get mode function
 * \n
 * \param m2m : video middleware struct
 * \param scale : 0=X1, 1=X2, 2=X4
 * \n
 * \return pdPASS/pdFail 
 */
static inline void snx_video_set_scale(struct snx_m2m *m2m, int scale)
{	
	m2m->vc.scale = scale;
}

/** \fn int snx_video_get_scale(struct snx_m2m *m2m)
 * \note Video get mode function
 * \n
 * \param m2m : video middleware struct
 * \return scale : 0=X1, 1=X2, 2=X4
 */
static inline int snx_video_get_scale(struct snx_m2m *m2m)
{	
	return m2m->vc.scale;
}

/** \fn void snx_video_set_refp(struct snx_m2m *m2m, int dup, int gop)
 * \note Video set multiple ref. P/p frame function
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param multiple_ref_p : 1 = I(refP)(refP)(refP) IPPPPPP
 * \                       2 = I(non-refP)(refP)(non-refP)(refP)(non-refP) IpPpPpPp
 * \                       3 = I(non-refP)(non-refP)(refP)(non-refP)(non-refP)(refP) IppPppP 
 * \                        (default multiple_ref_p:1)
 * \n
 * \return pdPASS/pdFail
 */
static inline void snx_video_set_refp(struct snx_m2m *m2m, int dup, int multiple_ref_p)
{	
	m2m->vc.vc_enc[dup].multiple_ref_p =multiple_ref_p;
}

/** \fn int snx_video_get_refp(struct snx_m2m *m2m, int dup, int type)
 * \note Video get multiple ref. P/p frame function
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \n
 * \return multiple_ref_p value
 */
static inline int snx_video_get_refp(struct snx_m2m *m2m, int dup)
{	
	return m2m->vc.vc_enc[dup].multiple_ref_p;
}

/** \fn int snx_video_is_refp(struct snx_m2m *m2m, int dup)
 * \note Video get H264 ref P frame function
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \n
 * \return 0:non-ref, 1: ref
 */
static inline int snx_video_is_refp(struct snx_m2m *m2m, int dup)
{	
	if(m2m->vc.vc_enc[dup].nonref_frame)
		return 0;
	else	
		return 1;
}

/** \fn void snx_video_set_gop(struct snx_m2m *m2m, int dup, int gop)
 * \note Video set gop function
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param gop : group of pictures (range: 0 ~ ..) (gop:0 frame rate = gop)
 * \n
 * \return pdPASS/pdFail
 */
static inline void snx_video_set_gop(struct snx_m2m *m2m, int dup, int gop)
{	
	m2m->vc.vc_enc[dup].gop =gop;
}

/** \fn int snx_video_get_gop(struct snx_m2m *m2m, int dup)
 * \note Video get gop function
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \n
 * \return GOP value
 */
static inline int snx_video_get_gop(struct snx_m2m *m2m, int dup)
{	
	return m2m->vc.vc_enc[dup].gop;
}

/** \fn void snx_video_set_qp(struct snx_m2m *m2m, int dup, int qp, int type)
 * \note Video set H264/JPEG QP function
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param qp : jpeg qp (range: 1 ~ 255), h264 qp (range: 10 ~ 50)
 * \param type : FMT_H264 or FMT_MJPEG
 * \n
 * \return pdPASS/pdFail
 */
static inline void snx_video_set_qp(struct snx_m2m *m2m, int dup, int qp, int type)
{	
	if(type == FMT_H264)
		m2m->vc.vc_enc[dup].qp =qp;
	else
		m2m->vc.vc_enc[dup].jpeg_qp =qp;
}

/** \fn int snx_video_get_qp(struct snx_m2m *m2m, int dup, int type)
 * \note Video get H264/JPEG QP function
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param type : FMT_H264 or FMT_MJPEG
 * \n
 * \return QP Vaule
 */
static inline int snx_video_get_qp(struct snx_m2m *m2m, int dup, int type)
{	
	if(type == FMT_H264)
		return m2m->vc.vc_enc[dup].qp;
	else
		return m2m->vc.vc_enc[dup].jpeg_qp;
}


/** \fn void snx_video_set_bufs(struct snx_m2m *m2m, int dup, int vc_bufs)
 * \note Video set bufs
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param vc_bufs : buffer number
 * \n
 * \return pdPASS/pdFail
 */
static inline void snx_video_set_bufs(struct snx_m2m *m2m, int dup, int vc_bufs)
{	
	m2m->vc.vc_enc[dup].vc_bufs = vc_bufs;
}


/** \fn int snx_video_get_bufs(struct snx_m2m *m2m, int dup)
 * \note Video get bufs
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \n
 * \return buffer number
 */
static inline int snx_video_get_bufs(struct snx_m2m *m2m, int dup)
{	
	return m2m->vc.vc_enc[dup].vc_bufs;
}

/** \fn void snx_video_set_percent(struct snx_m2m *m2m, int dup, int vc_bufs)
 * \note Video set H264/JPEG buffer percent, reduce memory allocation size
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param type : FMT_H264 or FMT_MJPEG
 * \param percent: buffer percent (default:100) (range: 0 ~ 100)
 * \n
 * \return pdPASS/pdFail
 */
static inline void snx_video_set_percent(struct snx_m2m *m2m, int dup, int type,int percent)
{
	if(percent > 100)
		percent = 100;
	else if (percent < 0)
		return ;

	if(type == FMT_H264)
		m2m->vc.vc_enc[dup].vc_h264_percent = percent;
	else
		m2m->vc.vc_enc[dup].vc_jpeg_percent = percent;
}


/** \fn int snx_video_get_percent(struct snx_m2m *m2m, int dup, int type)
 * \note Video get H264/JPEG buffer percent, reduce memory allocation size
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param type : FMT_H264 or FMT_MJPEG
 * \n
 * \return H264/JPEG buffer percent
 */
static inline int snx_video_get_percent(struct snx_m2m *m2m, int dup, int type)
{
	if(type == FMT_H264)
		return m2m->vc.vc_enc[dup].vc_h264_percent;
	else
		return m2m->vc.vc_enc[dup].vc_jpeg_percent;
}

/** \fn void snx_video_set_keyframe(struct snx_m2m *m2m)
 * \note Video set H264 key frame function
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \n
 * \return pdPASS/pdFail
 */
static inline void snx_video_set_keyframe(struct snx_m2m *m2m, int dup)
{
	m2m->vc.vc_enc[dup].flags = FLAG_KEYFRAME;
}


/** \fn int snx_video_is_keyframe(struct snx_m2m *m2m, int dup)
 * \note Video get H264 key frame function
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_video_is_keyframe(struct snx_m2m *m2m, int dup)
{	
	return m2m->vc.vc_enc[dup].keyframe;
}

/** \fn void snx_video_set_skipframe(struct snx_m2m *m2m, int dup,int skip_frame)
 * \note Video set H264 key frame function
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param skip_frame : skip frame counter (1 ~ )
 * \n
 * \return pdPASS/pdFail
 */
static inline void snx_video_set_skipframe(struct snx_m2m *m2m, int dup,int skip_frame)
{
	m2m->vc.vc_enc[dup].h264_skip_frames = skip_frame;
}

/** \fn int snx_video_h264_stream(struct snx_m2m *m2m, int dup, unsigned int *ptr)
 * \note Video get h264 address and size function
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param ptr : H264 address pointer
 * \return (bit stream size)
 * \n
 */
static inline int snx_video_h264_stream(struct snx_m2m *m2m, int dup, unsigned int *ptr)
{	
	int size;
	struct snx_enc_buf *enc_buf;

	if(m2m->vc.vc_enc[dup].vc_h264_get_index == 0)
		enc_buf = m2m->vc.vc_enc[dup].h264_ptr + m2m->vc.vc_enc[dup].vc_bufs -1;
	else
		enc_buf = m2m->vc.vc_enc[dup].h264_ptr + m2m->vc.vc_enc[dup].vc_h264_get_index - 1;


	
	if(enc_buf->status == VC_STATE_ACTIVE) {
		enc_buf->size = 0;
		enc_buf->status = VC_STATE_PREPARED;
	}
/*
	if(dup == 1) {
		struct snx_enc_buf *test;
		int i;
		for(i=0;i < m2m->vc.vc_enc[dup].vc_bufs; i++) {
		test = m2m->vc.vc_enc[dup].h264_ptr+i;
		print_msg_queue("<%s><%d><%d %d>  status=%d size=%d\n",__func__,__LINE__
					, i
					, m2m->vc.vc_enc[dup].vc_h264_get_index
					, test->status
					,test->size);
		}
	}
*/	
	enc_buf = m2m->vc.vc_enc[dup].h264_ptr + m2m->vc.vc_enc[dup].vc_h264_get_index;

	// now
	if(enc_buf->status != VC_STATE_DONE)
		return 0;

	*ptr = enc_buf->ptr;
	size = enc_buf->size;

	if(m2m->vc.vc_enc[dup].vc_bufs != 1)
		enc_buf->status = VC_STATE_ACTIVE;
	else
		enc_buf->status = VC_STATE_PREPARED;
	
	// next h264 get index
	m2m->vc.vc_enc[dup].vc_h264_get_index++;
	if(m2m->vc.vc_enc[dup].vc_h264_get_index  >= m2m->vc.vc_enc[dup].vc_bufs) {
		m2m->vc.vc_enc[dup].vc_h264_get_index = 0;
	}

	return size;
}

/** \fn int snx_video_jpeg_stream(struct snx_m2m *m2m, int dup, unsigned int *ptr)
 * \note Video get JPEG address and size function
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param ptr : JPEG address pointer
 * \return (bit stream size)
 * \n
 */
static inline int snx_video_jpeg_stream(struct snx_m2m *m2m, int dup, unsigned int *ptr)
{	
	int size;
	struct snx_enc_buf *enc_buf;

	if(m2m->vc.vc_enc[dup].vc_jpeg_get_index == 0)
		enc_buf = m2m->vc.vc_enc[dup].jpeg_ptr + m2m->vc.vc_enc[dup].vc_bufs -1;
	else
		enc_buf = m2m->vc.vc_enc[dup].jpeg_ptr + m2m->vc.vc_enc[dup].vc_jpeg_get_index - 1;

	if(enc_buf->status == VC_STATE_ACTIVE) {
		enc_buf->size = 0;
		enc_buf->status = VC_STATE_PREPARED;
	}

//	enc_buf->status = VC_STATE_PREPARED;
	enc_buf = m2m->vc.vc_enc[dup].jpeg_ptr + m2m->vc.vc_enc[dup].vc_jpeg_get_index;

	// now
	if(enc_buf->status != VC_STATE_DONE)
		return 0;

	*ptr = enc_buf->ptr;	
	size = enc_buf->size;
//	size = m2m->vc.vc_enc[dup].jpeg_bs_size;

	if(m2m->vc.vc_enc[dup].vc_bufs != 1)
		enc_buf->status = VC_STATE_ACTIVE;
	else
		enc_buf->status = VC_STATE_PREPARED;
	
	// next h264 get index
	m2m->vc.vc_enc[dup].vc_jpeg_get_index++;
	if(m2m->vc.vc_enc[dup].vc_jpeg_get_index  >= m2m->vc.vc_enc[dup].vc_bufs) {
		m2m->vc.vc_enc[dup].vc_jpeg_get_index = 0;
	}

	return size;
}

/** \fn int snx_video_isp_stream(struct snx_m2m *m2m, int dup, unsigned int *ptr)
 * \note Video get SONIX420 address and size function
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param ptr : SONIX420 address pointer
 * \return (bit stream size)
 * \n
 */
static inline int snx_video_isp_stream(struct snx_m2m *m2m, int dup, unsigned int *ptr)
{	
	int size;
	*ptr = *(m2m->vc.vc_enc[dup].src_ptr + m2m->vc.vc_enc[dup].vc_buf_index -1);
	size = m2m->vc.vc_enc[dup].image_size;
	return size;
}

/** @} */

/** \defgroup rc_func Video Rate Control functions
 * \ingroup mid_vc
 * \n  Video Rate Control Extension paramer set/get API
 * @{
 */

int snx_video_init_ext(struct snx_m2m *m2m, int dup);

/** \fn void int snx_get_ext_qp(struct snx_m2m *m2m, int dup
									, int *pframe_sum_num
									, int *maxqp_range
									, int *extend_max_qp
									, int *upperpframesize)
 * \note Video get extend QP
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param pframe_sum_num :  how many frames to count average P frame size
 * \param maxqp_range : start counting from MAX_QP - MaxQP_Range, ex. MAX_QP=36, MaxQP_Range=2 -> counting from QP >= 34
 * \param extend_max_qp : new max QP after extending QP
 * \param upperpframesize : when PFrameAve > UpperPFrameSize, function will be turn on
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_get_ext_qp(struct snx_m2m *m2m, int dup
									, int *pframe_sum_num
									, int *maxqp_range
									, int *extend_max_qp
									, int *upperpframesize)
{	
	*pframe_sum_num = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.pframe_sum_num;
	*maxqp_range = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.maxqp_range;
	*extend_max_qp = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.extend_max_qp;
	*upperpframesize = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.upperpframesize;
	return pdPASS;
}

/** \fn void int snx_set_ext_qp(struct snx_m2m *m2m, int dup
									, int pframe_sum_num
									, int maxqp_range
									, int extend_max_qp
									, int upperpframesize)
 * \note Video set extend QP
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param pframe_sum_num :  how many frames to count average P frame size
 * \param maxqp_range : start counting from MAX_QP - MaxQP_Range, ex. MAX_QP=36, MaxQP_Range=2 -> counting from QP >= 34
 * \param extend_max_qp : new max QP after extending QP
 * \param upperpframesize : when PFrameAve > UpperPFrameSize, function will be turn on
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_ext_qp(struct snx_m2m *m2m, int dup
									, int pframe_sum_num
									, int maxqp_range
									, int extend_max_qp
									, int upperpframesize)
{	
	m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.pframe_sum_num = pframe_sum_num;
	m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.maxqp_range = maxqp_range;
	m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.extend_max_qp = extend_max_qp;
	m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.upperpframesize = upperpframesize;
	return pdPASS;
}


/** \fn int snx_get_mbrc_en(struct snx_m2m *m2m, int dup, int *enable)
 * \note Get H264 MBRC enable parameter
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param enable : enable=1, disable=0
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_get_mbrc_en(struct snx_m2m *m2m, int dup, int *enable)
{	
	*enable = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.mbrc_en;
	return pdPASS;
}

/** \fn int snx_set_mbrc_en(struct snx_m2m *m2m, int dup, int enable)
 * \note Set H264 MBRC enable parameter
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param enable : enable=1, disable=0
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_mbrc_en(struct snx_m2m *m2m, int dup, int enable)
{	
	m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.mbrc_en = enable;
	return pdPASS;
}


/** \fn int snx_get_mdrc_en(struct snx_m2m *m2m, int dup, int *enable)
 * \note Get Motion detection rate control enable parameter
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param enable : enable=1, disable=0

 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_get_mdrc_en(struct snx_m2m *m2m, int dup, int *enable)
{	
	*enable = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.mdrc_en;
	return pdPASS;
}

/** \fn int snx_set_mdrc_en(struct snx_m2m *m2m, int dup, int enable)
 * \note Set Motion detection rate control enable parameter
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param enable : enable=1, disable=0 (default: 1)

 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_mdrc_en(struct snx_m2m *m2m, int dup, int enable)
{	
	m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.mdrc_en = enable;
	return pdPASS;
}


/** \fn int  snx_get_md_mn(struct snx_m2m *m2m, int dup, int *md_m, int *md_n)
 * \note Get Motion detection rate control corner MB block parameter  
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param md_m : corner width MD block  ( < 1/2*(16), default:3 )
 * \param md_n : corner height MD block ( < 1/2*(12), default:3 )
 * \n
 * \return pdPASS/pdFail
 */
static inline int  snx_get_md_mn(struct snx_m2m *m2m, int dup, int *md_m, int *md_n)
{	
	*md_m = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_m;
	*md_n = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_n;
	return pdPASS;
}


/** \fn int  snx_set_md_mn(struct snx_m2m *m2m, int dup, int md_m, int md_n)
 * \note Set Motion detection rate control corner MB block parameter  
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param md_m : corner width MD block  ( < 1/2*(16), default:3 )
 * \param md_n : corner height MD block ( < 1/2*(12), default:3 )
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_md_mn(struct snx_m2m *m2m, int dup, int md_m, int md_n)
{	
	if(md_m != 0)
		m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_m = md_m;
	if(md_n != 0)
		m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_n = md_n;
	return pdPASS;
}


/** \fn int  snx_get_md_th(struct snx_m2m *m2m, int dup, int *md_th, int *md_recover)
 * \note Get Motion detection rate control md_th/md_recover parameter  
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param md_th : corner trigger counter ( <= (md_m * md_n), default:1 )
 * \param md_recover : finish MD, wating frame counter (default:25)
 * \n
 * \return pdPASS/pdFail
 */
static inline int  snx_get_md_th(struct snx_m2m *m2m, int dup, int *md_th, int *md_recover)
{	

	*md_th = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_th;
	*md_recover = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_recover;
	return pdPASS;
}


/** \fn int  snx_set_md_th(struct snx_m2m *m2m, int dup, int md_th, int md_recover)
 * \note Set Motion detection rate control md_th/md_recover parameter  
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param md_th : corner trigger counter ( <= (md_m * md_n), default:1 )
 * \param md_recover : finish MD, wating frame counter (default:25)
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_md_th(struct snx_m2m *m2m, int dup, int md_th, int md_recover)
{	
	if(md_th != 0)
		m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_th = md_th;
	if(md_recover != 0)
		m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_recover = md_recover;
	return pdPASS;
}
/** \fn int  snx_get_md_2dnr(struct snx_m2m *m2m, int dup, int *md_2dnr, int *md_isp_nr)
 * \note Get Motion detection rate control md_2dnr/md_isp_nr parameter  
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param md_2dnr : MD enable, blured level ( range:0 ~ 4, default:1 )
 * \param md_isp_nr : finish MD, wating blured frame counter (default:23)
 * \n
 * \return pdPASS/pdFail
 */
static inline int  snx_get_md_2dnr(struct snx_m2m *m2m, int dup, int *md_2dnr, int *md_isp_nr)
{
	*md_2dnr = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_2dnr;
	*md_isp_nr = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_isp_nr;

	return pdPASS;
}
/** \fn int  snx_set_md_2dnr(struct snx_m2m *m2m, int dup, int md_2dnr, int md_isp_nr)
 * \note Set Motion detection rate control md_2dnr/md_isp_nr parameter  
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param md_2dnr : MD enable, blured level ( range:0 ~ 4, default:1 )
 * \param md_isp_nr : finish MD, wating blured frame counter (default:23)
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_md_2dnr(struct snx_m2m *m2m, int dup, int md_2dnr, int md_isp_nr)
{
	if(md_2dnr != 0)
		m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_2dnr = md_2dnr;
	if(md_isp_nr != 0)
		m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_isp_nr = md_isp_nr;
	return pdPASS;
}
/** \fn int  snx_get_md_fpsbps(struct snx_m2m *m2m, int dup, int *md_max_fps, int *md_can_add_bitrate)
 * \note Get Motion detection rate control md_max_fps/md_can_add_bitrate parameter  
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param md_max_fps : max sensor frame rate ( range: < sensor fps, default:30 )
 * \param md_can_add_bitrate : MD enable, add bitrate ( ragne: +Kbps, default:0 )
 * \n
 * \return pdPASS/pdFail
 */
static inline int  snx_get_md_fpsbps(struct snx_m2m *m2m, int dup, int *md_max_fps, int *md_can_add_bitrate)
{
	*md_max_fps = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_max_fps;
	*md_can_add_bitrate = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_can_add_bitrate;

	return pdPASS;
}
/** \fn int  snx_set_md_fpsbps(struct snx_m2m *m2m, int dup, int md_max_fps, int md_can_add_bitrate)
 * \note Set Motion detection rate control md_max_fps/md_can_add_bitrate parameter  
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param md_max_fps : max sensor frame rate ( range: < sensor fps, default:30 )
 * \param md_can_add_bitrate : MD enable, add bitrate ( range: +Kbps, default:0 )
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_md_fpsbps(struct snx_m2m *m2m, int dup, int md_max_fps, int md_can_add_bitrate)
{
	if(md_max_fps != 0)
		m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_max_fps = md_max_fps;
	if(md_can_add_bitrate != 0)
		m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_can_add_bitrate = md_can_add_bitrate;
	return pdPASS;
}


/** \fn int snx_get_mdcnt_en(struct snx_m2m *m2m, int dup, int *enable)
 * \note Get Low bit rate control enable parameter
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param enable : enable=1, disable=0 (default:1)

 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_get_mdcnt_en(struct snx_m2m *m2m, int dup, int *enable)
{	
	*enable = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_cnt_en;
	return pdPASS;
}
/** \fn int snx_set_mdrc_en(struct snx_m2m *m2m, int dup, int enable)
 * \note Set Low bit rate control enable parameter 
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param enable : enable=1, disable=0 (default:1)
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_mdcnt_en(struct snx_m2m *m2m, int dup, int enable)
{	
	m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_cnt_en = enable;
	return pdPASS;
}


/** \fn int snx_get_mdcnt_th(struct snx_m2m *m2m, int dup, int *md_cnt_sum_th, int *md_cnt_th)
 * \note Get Low bit rate control md_cnt_sum_th/md_cnt_th parameter  
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param md_cnt_sum_th : low bit rate mode motion detect count (md_cnt_sum_th/192) (default:10) 
 * \param md_cnt_th : low bit rate mode motion detection threshold (default:150) 
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_get_mdcnt_th(struct snx_m2m *m2m, int dup, int *md_cnt_sum_th, int *md_cnt_th)
{	
	*md_cnt_sum_th = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_cnt_sum_th;
	*md_cnt_th = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_cnt_th;
	return pdPASS;
}


/** \fn int snx_set_mdcnt_th(struct snx_m2m *m2m, int dup, int md_cnt_sum_th, int md_cnt_th)
 * \note Set Low bit rate control md_cnt_sum_th/md_cnt_th parameter  
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param md_cnt_sum_th : low bit rate mode motion detect count (md_cnt_sum_th/192) (default:10) 
 * \param md_cnt_th : low bit rate mode motion detection threshold (default:150) 
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_mdcnt_th(struct snx_m2m *m2m, int dup, int md_cnt_sum_th, int md_cnt_th)
{	
	if(md_cnt_sum_th != 0)
		m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_cnt_sum_th = md_cnt_sum_th;
	if(md_cnt_th != 0)
		m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_cnt_th = md_cnt_th;
	return pdPASS;
}

/** \fn int snx_get_mdcnt_bps(struct snx_m2m *m2m, int dup, int *md_cnt_bps, int *md_cnt_bps2)
 * \note Get Low bit rate control md_cnt_bps/md_cnt_bps2 parameter    
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param md_cnt_bps : low bit rate mode set dup 0 bps 
 * \param md_cnt_bps2 : low bit rate mode set dup 1 bps 

 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_get_mdcnt_bps(struct snx_m2m *m2m, int dup, int *md_cnt_bps, int *md_cnt_bps2)
{	
	*md_cnt_bps = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_cnt_bps;
	*md_cnt_bps2 = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_cnt_bps2;
	return pdPASS;
}

/** \fn int snx_set_mdcnt_bps(struct snx_m2m *m2m, int dup, int md_cnt_bps, int md_cnt_bps2)
 * \note Set Low bit rate control md_cnt_bps/md_cnt_bps2 parameter    
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param md_cnt_bps : low bit rate mode set dup 0 bps 
 * \param md_cnt_bps2 : low bit rate mode set dup 1 bps 
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_mdcnt_bps(struct snx_m2m *m2m, int dup, int md_cnt_bps, int md_cnt_bps2)
{	
	if(md_cnt_bps != 0)
		m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_cnt_bps = md_cnt_bps;
	if(md_cnt_bps2 != 0)
		m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_cnt_bps2 = md_cnt_bps2;
	return pdPASS;
}

/** \fn int snx_get_mdcnt_lowbound(struct snx_m2m *m2m, int dup, int *md_cnt_lowbound, int *md_cnt_qp)
 * \note Get Low bit rate control md_cnt_lowbound/md_cnt_qp parameter
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param md_cnt_lowbound : low bit rate mode set minimum bps (detect by image quality) 
 * \param md_cnt_qp : low bit rate mode max QP for keep image quality (default:23) 
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_get_mdcnt_lowbound(struct snx_m2m *m2m, int dup, int *md_cnt_lowbound, int *md_cnt_qp)
{	
	*md_cnt_lowbound = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_cnt_lowbound;
	*md_cnt_qp = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_cnt_qp;
	return pdPASS;
}

/** \fn int snx_set_mdcnt_lowbound(struct snx_m2m *m2m, int dup, int md_cnt_lowbound, int md_cnt_qp)
 * \note Set Low bit rate control md_cnt_lowbound/md_cnt_qp parameter
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param md_cnt_lowbound :low bit rate mode set minimum bps (detect by image quality) 
 * \param md_cnt_qp : low bit rate mode max QP for keep image quality (default:23)

 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_mdcnt_lowbound(struct snx_m2m *m2m, int dup, int md_cnt_lowbound, int md_cnt_qp)
{	
	if(md_cnt_lowbound != 0)
		m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_cnt_lowbound = md_cnt_lowbound;
	if(md_cnt_qp != 0)
		m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_cnt_qp = md_cnt_qp;
	return pdPASS;
}

/** \fn int snx_get_mdcnt_absy(struct snx_m2m *m2m, int dup, int *md_cnt_absy)
 * \note Get Low bit rate control md_cnt_absy parameter
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param md_cnt_absy : low bit rate mode low lux disable low bit rate mode (default:0 disable) 
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_get_mdcnt_absy(struct snx_m2m *m2m, int dup, int *md_cnt_absy)
{	
	*md_cnt_absy = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_cnt_absy;
	return pdPASS;
}

/** \fn int snx_set_mdcnt_absy(struct snx_m2m *m2m, int dup, int *md_cnt_absy)
 * \note Set Low bit rate control md_cnt_absy parameter
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param md_cnt_absy : low bit rate mode low lux disable low bit rate mode (default:0 disable) 
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_mdcnt_absy(struct snx_m2m *m2m, int dup, int md_cnt_absy)
{	
	m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_cnt_absy = md_cnt_absy;
	return pdPASS;
}

/** \fn int snx_get_mdcnt_gop(struct snx_m2m *m2m, int dup, int *md_cnt_gop_multiple)
 * \note Get Low bit rate control md_cnt_gop_multiple parameter
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param md_cnt_gop_multiple : low bit rate mode set gop multiple, GOP=frame x md_cnt_gop_multiple (default 8)
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_get_mdcnt_gop(struct snx_m2m *m2m, int dup, int *md_cnt_gop_multiple)
{	
	*md_cnt_gop_multiple = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_cnt_gop_multiple;
	return pdPASS;
}
/** \fn int snx_set_mdcnt_gop(struct snx_m2m *m2m, int dup, int md_cnt_gop_multiple)
 * \note Set Low bit rate control md_cnt_gop_multi parameter
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param md_cnt_gop_multiple : low bit rate mode set gop multiple, GOP=frame x md_cnt_gop_multiple (default:8)
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_mdcnt_gop(struct snx_m2m *m2m, int dup, int md_cnt_gop_multiple)
{	
	m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_cnt_gop_multiple = md_cnt_gop_multiple;
	return pdPASS;
}

/** \fn int snx_get_mdcnt_count(struct snx_m2m *m2m, int dup, int *md_cnt_count)
 * \note Get Low bit rate control md_cnt_count parameter
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param md_cnt_count : Enter low bit rate mode, wating frame counter (deafault:120)
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_get_mdcnt_count(struct snx_m2m *m2m, int dup, int *md_cnt_count)
{	
	*md_cnt_count = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_cnt_count;
	return pdPASS;
}

/** \fn int snx_set_mdcnt_count(struct snx_m2m *m2m, int dup, int md_cnt_count)
 * \note Set Low bit rate control md_cnt_count parameter
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param md_cnt_count : Enter low bit rate mode, wating frame counter (deafault:120)
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_mdcnt_count(struct snx_m2m *m2m, int dup, int md_cnt_count)
{	
	m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.md_cnt_count = md_cnt_count;
	return pdPASS;
}

/** \fn int snx_get_rc_up(struct snx_m2m *m2m, int dup, int *rc_up, int *rc_rate)
 * \note Get Max bitrate up bound and Bitrate Exceed
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param rc_up : max bit rate == bit_rate * (1 + (rc_up /32)) (default:6 )
 * \param rc_rate : bit_rate_exceed = bitrate_exceed *(1 - (1/ rc_rate)) (default:50)
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_get_rc_up(struct snx_m2m *m2m, int dup, int *rc_up, int *rc_rate)
{
	*rc_up = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.rc_up;
	*rc_rate = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.rc_rate;
	return pdPASS;
}

/** \fn int snx_set_rc_up(struct snx_m2m *m2m, int dup, int rc_max_qp, int rc_min_qp)
 * \note Set Max bitrate up bound and Bitrate Exceed
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param rc_up : max bit rate == bit_rate * (1 + (rc_up /32)) (default:6 )
 * \param rc_rate : bit_rate_exceed = bitrate_exceed *(1 - (1/ rc_rate)) (default:50)

 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_rc_up(struct snx_m2m *m2m, int dup, int rc_up, int rc_rate)
{
	m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.rc_up = rc_up;
	m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.rc_rate = rc_rate;
	return pdPASS;	
}

/** \fn int snx_get_delta_qp(struct snx_m2m *m2m, int dup, int *delta_qp, int *delta_qp_stop_cnt)
 * \note Get Max bitrate up bound and Bitrate Exceed
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param delta_qp : delta QP up bound (default:8, dashcam suggest set 5)
 * \delta_qp_stop_cnt delta_qp : delta QP stop counter (default:5)
 * \Max QP of Pframe to be used withing one GOP is QP of Iframe + delta_qp
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_get_delta_qp(struct snx_m2m *m2m, int dup, int *delta_qp, int *delta_qp_stop_cnt)
{
	*delta_qp = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.delta_qp;
	*delta_qp_stop_cnt = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.delta_qp_stop_cnt;
	return pdPASS;
}

/** \fn int snx_set_delta_qp(struct snx_m2m *m2m, int dup, int delta_qp, int delta_qp_stop_cnt)
 * \note Set Max bitrate up bound and Bitrate Exceed
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param delta_qp : delta QP up bound (default:8, dashcam suggest set 5)
 * \delta_qp_stop_cnt delta_qp : delta QP stop counter (default:5)
 * \Max QP of Pframe to be used withing one GOP is QP of Iframe + delta_qp
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_delta_qp(struct snx_m2m *m2m, int dup, int delta_qp, int delta_qp_stop_cnt)
{
	m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.delta_qp = delta_qp;
	m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.delta_qp_stop_cnt = delta_qp_stop_cnt;
	return pdPASS;	
}

/** \fn int snx_get_qp_boundary(struct snx_m2m *m2m, int dup, int *rc_max_qp, int *rc_min_qp)
 * \note Get Low bit rate control md_cnt_lowbound/md_cnt_qp parameter
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param rc_max_qp : Max QP (range: < 50)
 * \param rc_min_qp : Min QP (range: > 10)
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_get_qp_boundary(struct snx_m2m *m2m, int dup, int *rc_max_qp, int *rc_min_qp)
{	
	*rc_max_qp = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.rc_max_qp;
	*rc_min_qp = m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.rc_min_qp;
	return pdPASS;
}

/** \fn int snx_set_qp_boundary(struct snx_m2m *m2m, int dup, int rc_max_qp, int rc_min_qp)
 * \note Set QP boundary rc_max_qp/rc_min_qp parameter
 * \n
 * \param m2m : video middleware struct
 * \param dup : master path=0, duplicat path=1
 * \param rc_max_qp : Max QP (range: < 50)
 * \param rc_min_qp : Min QP (range: > 10)

 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_qp_boundary(struct snx_m2m *m2m, int dup, int rc_max_qp, int rc_min_qp)
{	
	m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.rc_max_qp = rc_max_qp;
	m2m->vc.vc_enc[dup].snx_rc.snx_rc_ext.rc_min_qp = rc_min_qp;
	return pdPASS;
}

/** @} */

 
//static char ascii_2_font[] =
extern char ascii_2_font[];


#define DS_RED				"255 0 0"
#define DS_GREEN			"0 255 0"
#define DS_BLUE				"0 0 255"
#define DS_YELLOW			"255 242 0"
#define DS_MAGENTA			"163 73 164"
#define DS_CYAN				"15 217 217"
#define DS_BLACK			"0 0 0"
#define DS_WHITE			"255 255 255"

#define EXT_SIZE_LARGE	2
#define EXT_SIZE_MEDIUM	1
#define EXT_SIZE_NONE	0

/** \defgroup rc_func MROI middleware functions
 * \ingroup mid_vc
 * \n 
 * @{
 */

/** \fn int snx_set_mroi_mode_en(struct snx_rc *rc,uint8_t enable)
 * \note Video set MROI mode enable function.
 * \n
 * \param rc : video rate control struct
 * \param enable : 0 -> disable
 * \n			1 -> enable
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_mroi_mode_en(struct snx_rc *rc,uint8_t enable)
{
	rc->mroi_en = enable;
	return pdPASS;
}

/** \fn int snx_set_mroi_region_en(struct snx_rc *rc,uint8_t num, uint8_t enable)
 * \note Video set MROI region enable function.
 * \n
 * \param rc : video rate control struct
 * \param num : region num, 0~7
 * \param enable : 0 -> disable
 * \n			1 -> enable
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_mroi_region_en(struct snx_rc *rc,uint8_t num, uint8_t enable)
{
	rc->mroi_region[num].region_en = enable;
	return pdPASS;
}

/** \fn int snx_set_mroi_frame_size(struct snx_rc *rc, uint32_t width, uint32_t height)
 * \note Video set MROI frame size. In order to check wrong setting of MROI.
 * \n
 * \param rc : video rate control struct
 * \param width : frame width
 * \param height : frame height
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_mroi_frame_size(struct snx_rc *rc, uint32_t width, uint32_t height)
{
	rc->width = width;
	rc->height = height;
	return pdPASS;
}

int snx_set_mroi_weight(struct snx_rc *rc, uint8_t weight, uint8_t num);
int snx_set_mroi_qp(struct snx_rc *rc, int8_t qp, uint8_t num);
int snx_set_mroi_pos_dim(struct snx_rc *rc, uint32_t pos_x, uint32_t pos_y, uint32_t dim_x, uint32_t dim_y, uint8_t num);

/** \fn int snx_set_mroi_ext(struct snx_rc *rc, uint8_t ext_size, uint8_t num)
 * \note Video set MROI region ext_size function. To reduce sharpness in boundary area.
 * \n
 * \param rc : video rate control struct
 * \param ext_size : Extension size, can be 0~2
 * \param num : region num, 0~7
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_mroi_ext(struct snx_rc *rc, uint8_t ext_size, uint8_t num)
{
	if (ext_size == 1)
		rc->mroi_region[num].ext_size = 3;
	else if (ext_size == 2)
		rc->mroi_region[num].ext_size = 7;
	else
		rc->mroi_region[num].ext_size = 0;
	return pdPASS;
}
/** @} */

/** \defgroup vc_ds Video Data Stamp functions
 * \ingroup mid_vc
 * \n
 * @{
 */
int snx_set_ds_bmp_scale_down(unsigned char *in_bmp_file, unsigned char *out_bmp_file);
int snx_set_ds_bmp_scale_up(unsigned char *in_bmp_file, unsigned char *out_bmp_file);
int snx_set_ds_bmp(struct snx_vc_ds *cds);
unsigned int* snx_get_ds_font_table(struct snx_vc_ds *cds, void* font_file, void *string);
int snx_set_ds_string(struct snx_vc_ds *cds, char *outside_ascii, unsigned int *outside_font);

/** \fn int snx_set_ds_en(struct snx_vc_enc *vc_enc, uint8_t enable)
 * \note Video set ds enable function.
 * \n
 * \param vc_enc : video encode struct
 * \param enable : 0 -> disable
 * \n			1 -> enable
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_ds_en(struct snx_vc_enc *vc_enc, uint8_t enable)
{
	vc_enc->ds_en = enable;
	return pdPASS;
}

/** \fn int snx_set_ds_mode(struct snx_vc_ds *cds, uint8_t mode)
 * \note Video set ds mode function.
 * \n
 * \param cds : video data stamp struct
 * \param mode : 0 -> weighting mode
 * \n			1 -> direct text mode
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_ds_mode(struct snx_vc_ds *cds, uint8_t mode)
{
	cds->attr.mode = mode;
	return pdPASS;
}

/** \fn int snx_set_ds_weight(struct snx_vc_ds *cds, uint8_t weight)
 * \note Video set ds weight function.
 * \n
 * \param cds : video data stamp struct
 * \param weight : weighting value, 0~7, valid in weighting mode
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_ds_weight(struct snx_vc_ds *cds, uint8_t weight)
{
	cds->attr.weight = weight;
	return pdPASS;
}

/** \fn int snx_set_ds_scale(struct snx_vc_ds *cds, uint8_t scale)
 * \note Video set ds scale function.
 * \n
 * \param cds : video data stamp struct
 * \param scale : can be 0, 1, 2
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_ds_scale(struct snx_vc_ds *cds, uint8_t scale)
{
	cds->scale = scale;
	return pdPASS;
}

/** \fn int snx_set_ds_pos(struct snx_vc_ds *cds, uint32_t pos_x, uint32_t pos_y)
 * \note Video set ds pos function.
 * \n
 * \param cds : video data stamp struct
 * \param pos_x : horizontal start pixel from (0, 0)
 * \param pos_y : vertical start pixel from (0, 0)
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_ds_pos(struct snx_vc_ds *cds, uint32_t pos_x, uint32_t pos_y)
{
	cds->pos.start_x = pos_x;
	cds->pos.start_y = pos_y;
	return pdPASS;
}

/** \fn int snx_set_ds_transparent_value(struct snx_vc_ds *cds, uint32_t value)
 * \note Video set ds transparent value function. User can specify transparent color.
 * \n
 * \param cds : video data stamp struct
 * \param value : video data stamp transparent value
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_ds_transparent_value(struct snx_vc_ds *cds, uint32_t value)
{
	cds->transparent_value = value;
	return pdPASS;
}

/** \fn int snx_set_ds_transparent_level(struct snx_vc_ds *cds)
 * \note Video set ds transparent level function.
 * \n
 * \param cds : video data stamp struct
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_ds_transparent_level(struct snx_vc_ds *cds)
{
	cds->transparent_level = snx_cds_bmp_trans_level_scan(cds, 0);
	return pdPASS;
}

/** \fn int snx_set_ds_transparent(struct snx_vc_ds *cds, uint8_t transparent)
 * \note Video set ds transparent mode function.
 * \n
 * \param cds : video data stamp struct
 * \param transparent : 0 -> none
 * \n			1 -> color white will be transparent
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_ds_transparent(struct snx_vc_ds *cds, uint8_t transparent)
{
	cds->transparent_mode = transparent;
	return pdPASS;
}

/** \fn int snx_set_ds_string_color(struct snx_vc_ds *cds, uint8_t text_R, uint8_t text_G,
 * uint8_t text_B, uint8_t background_R, uint8_t background_G, uint8_t background_B)
 * \note Video set ds string color function.
 * \n
 * \param cds : video data stamp struct
 * \param text_R : text R value
 * \param text_G : text G value
 * \param text_B : text B value
 * \param background_R : background R value
 * \param background_G : background G value
 * \param background_B : background B value
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_ds_string_color(struct snx_vc_ds *cds, uint8_t text_R,
										  uint8_t text_G, uint8_t text_B, uint8_t background_R,
										  uint8_t background_G, uint8_t background_B)
{
	cds->t_color.color_R = text_R;
	cds->t_color.color_G = text_G;
	cds->t_color.color_B = text_B;

	cds->b_color.color_R = background_R;
	cds->b_color.color_G = background_G;
	cds->b_color.color_B = background_B;
	return pdPASS;
}

/** \fn int snx_set_ds_font_size(struct snx_vc_ds *cds, uint8_t width, uint8_t height)
 * \note Video set ds font size function.
 * \n
 * \param cds : video data stamp struct
 * \param width : font width
 * \param height : font height
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_ds_font_size(struct snx_vc_ds *cds, uint8_t width, uint8_t height)
{
	cds->font_height = height << cds->scale;
	cds->font_width = width << cds->scale;
	return pdPASS;
}

/** \fn int snx_set_ds_bmp_threshold(struct snx_vc_ds *cds, uint32_t threshold_R,
 * uint32_t threshold_G, uint32_t threshold_B)
 * \note Video set ds bmp background threshold function.
 * \n
 * \param cds : video data stamp struct
 * \param threshold_R : threshold R value
 * \param threshold_G : threshold G value
 * \param threshold_B : threshold B value
 * \n image R, G, B value greater than threshold will be set to background
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_ds_bmp_threshold(struct snx_vc_ds *cds, uint32_t threshold_R,
										   uint32_t threshold_G, uint32_t threshold_B)
{
	cds->bmp_threshold.color_R = threshold_R;
	cds->bmp_threshold.color_G = threshold_G;
	cds->bmp_threshold.color_B = threshold_B;
	return pdPASS;
}

/** \fn int snx_set_ds_bmp_rounding(struct snx_vc_ds *cds, uint8_t rounding)
 * \note Video set ds bmp rounding or not when convert to 8-bit pallet mode function.
 * \n
 * \param cds : video data stamp struct
 * \param rounding : 0 -> none
 * \n			1 -> rounding when bmp src is 24-bit image
 * \n
 * \return pdPASS/pdFail
 */
static inline int snx_set_ds_bmp_rounding(struct snx_vc_ds *cds, uint8_t rounding)
{
	cds->bmp_rounding = rounding;
	return pdPASS;
}

int snx_video_is_iframe(unsigned char *addr, unsigned size);

/** @} */

#endif //__MID_VC_H__
