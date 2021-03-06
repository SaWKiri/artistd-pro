#include "../../../../MarlinCore.h"
#if ENABLED(TFT_LITTLE_VGL_UI)
#include "draw_ui.h"
#include "../../../../module/temperature.h"
#include "../../../../module/motion.h"
#include "../../../../sd/cardreader.h"
#include "../../../../gcode/queue.h"
#if ENABLED(POWER_LOSS_RECOVERY)
#include "../../../../feature/powerloss.h"
#endif


#define ID_PAUSE		1
#define ID_STOP			2
#define ID_OPTION		3

static lv_obj_t *scr;
static lv_obj_t *labelExt1, *labelExt2, *labelFan, *labelZpos, *labelTime;
static lv_obj_t *labelPause, *labelStop, *labelOperat;
static lv_obj_t *bar1, *bar1ValueText;
static lv_obj_t *buttonPause, *buttonOperat, *buttonStop;
#if HAS_HEATED_BED
static lv_obj_t *labelBed;
#endif

uint8_t once_flag = 0;
extern uint32_t To_pre_view;
extern uint8_t flash_preview_begin;
extern uint8_t default_preview_flg;
extern uint8_t gcode_preview_over;

static void event_handler(lv_obj_t *obj, lv_event_t event)
{
	switch(obj->mks_obj_id)
	{
		case ID_PAUSE:
			if(event == LV_EVENT_CLICKED) {
		
			}
			else if(event == LV_EVENT_RELEASED) {
				if(gcode_preview_over != 1)
				{
					if(uiCfg.print_state == WORKING)
					{
						#if ENABLED(PARK_HEAD_ON_PAUSE)
						queue.inject_P(PSTR("M25 P\nM24"));
						uiCfg.print_state = PAUSED;
						#elif ENABLED(SDSUPPORT)
						queue.inject_P(PSTR("M25"));
						uiCfg.print_state = PAUSED;
						#endif
						
						lv_obj_set_event_cb_mks(buttonPause, event_handler, ID_PAUSE, "bmp_Pause.bin", 0);
						lv_label_set_text(labelPause, printing_menu.resume);
						lv_obj_align(labelPause, buttonPause, LV_ALIGN_CENTER, 30, 0);
					}
					else if(uiCfg.print_state == PAUSED)
					{
						uiCfg.print_state = WORKING;
						
						#if ENABLED(PARK_HEAD_ON_PAUSE)
						wait_for_heatup = wait_for_user = false;
						#endif
						if (IS_SD_PAUSED())queue.inject_P(PSTR("M24"));// queue.inject_P(M24_STR);
						
						lv_obj_set_event_cb_mks(obj, event_handler, ID_PAUSE, "bmp_Resume.bin", 0);
						lv_label_set_text(labelPause, printing_menu.pause);
						lv_obj_align(labelPause, buttonPause, LV_ALIGN_CENTER, 30, 0);
					}
					#if ENABLED(POWER_LOSS_RECOVERY)
					else if(uiCfg.print_state == REPRINTING)
					{
						digitalWrite(Z_ENABLE_PIN, LOW);
						digitalWrite(E2_ENABLE_PIN, LOW);
						
						uiCfg.print_state = REPRINTED;
						lv_obj_set_event_cb_mks(obj, event_handler, ID_PAUSE, "bmp_Resume.bin", 0);
						lv_label_set_text(labelPause, printing_menu.pause);
						lv_obj_align(labelPause, buttonPause, LV_ALIGN_CENTER, 30, 0);
						//recovery.resume();
						print_time.minutes = recovery.info.print_job_elapsed / 60;
						print_time.seconds = recovery.info.print_job_elapsed % 60;
						print_time.hours = print_time.minutes / 60;
						start_print_time();
					}
					#endif
				}
			}
			break;
		case ID_STOP:
			if(event == LV_EVENT_CLICKED) {
				
			}
			else if(event == LV_EVENT_RELEASED) {
				if(gcode_preview_over != 1)
				{
					lv_clear_printing();
					lv_draw_dialog(DIALOG_TYPE_STOP);
				}
			}
			break;
		case ID_OPTION:
			if(event == LV_EVENT_CLICKED) {
				
			}
			else if(event == LV_EVENT_RELEASED) {
				if(gcode_preview_over != 1)
				{
					lv_clear_printing();
					lv_draw_opration();
				}
			}
			break;
	}
}

void lv_draw_printing(void)
{
	lv_obj_t *buttonExt1, *buttonExt2, *buttonFanstate;
	lv_obj_t *buttonZpos, *buttonTime;
	#if HAS_HEATED_BED
	lv_obj_t *buttonBedstate;
	#endif

	disp_state_stack._disp_index = 0;
	memset(disp_state_stack._disp_state, 0, sizeof(disp_state_stack._disp_state));
	disp_state_stack._disp_state[disp_state_stack._disp_index] = PRINTING_UI;

	disp_state = PRINTING_UI;

	scr = lv_obj_create(NULL, NULL);

	//static lv_style_t tool_style;
	
	lv_obj_set_style(scr, &tft_style_scr);
  	lv_scr_load(scr);
  	lv_obj_clean(scr);

  	lv_obj_t *title = lv_label_create(scr, NULL);
	lv_obj_set_style(title, &tft_style_label_rel);
	lv_obj_set_pos(title, TITLE_XPOS, TITLE_YPOS);
	lv_label_set_text(title, creat_title_text());
  
  	lv_refr_now(lv_refr_get_disp_refreshing());
	
	LV_IMG_DECLARE(bmp_pic_150x80);
	LV_IMG_DECLARE(bmp_pic_45x45);
	
    /*Create an Image button*/
    buttonExt1 = lv_imgbtn_create(scr, NULL);
	if(EXTRUDERS == 2)
		buttonExt2 = lv_imgbtn_create(scr, NULL);
	#if HAS_HEATED_BED
	buttonBedstate = lv_imgbtn_create(scr, NULL);
	#endif
	buttonFanstate = lv_imgbtn_create(scr, NULL);
	buttonZpos = lv_imgbtn_create(scr, NULL);
	buttonPause = lv_imgbtn_create(scr, NULL);
	buttonStop = lv_imgbtn_create(scr, NULL);
	buttonOperat = lv_imgbtn_create(scr, NULL);
	buttonTime = lv_imgbtn_create(scr, NULL);

	lv_obj_set_event_cb_mks(buttonExt1, event_handler, 0, "bmp_Ext1_state.bin", 0);
	lv_imgbtn_set_src(buttonExt1, LV_BTN_STATE_REL, &bmp_pic_45x45);
	lv_imgbtn_set_src(buttonExt1, LV_BTN_STATE_PR, &bmp_pic_45x45);
	lv_imgbtn_set_style(buttonExt1, LV_BTN_STATE_PR, &tft_style_label_pre);
	lv_imgbtn_set_style(buttonExt1, LV_BTN_STATE_REL, &tft_style_label_rel);
	lv_obj_clear_protect(buttonExt1, LV_PROTECT_FOLLOW);

	if(EXTRUDERS == 2)
	{
		lv_obj_set_event_cb_mks(buttonExt2, event_handler, 0, "bmp_Ext2_state.bin", 0);
		lv_imgbtn_set_src(buttonExt2, LV_BTN_STATE_REL, &bmp_pic_45x45);
		lv_imgbtn_set_src(buttonExt2, LV_BTN_STATE_PR, &bmp_pic_45x45);
		lv_imgbtn_set_style(buttonExt2, LV_BTN_STATE_PR, &tft_style_label_pre);
		lv_imgbtn_set_style(buttonExt2, LV_BTN_STATE_REL, &tft_style_label_rel);
	}
	#if HAS_HEATED_BED
	lv_obj_set_event_cb_mks(buttonBedstate, event_handler, 0, "bmp_Bed_state.bin", 0);	
    lv_imgbtn_set_src(buttonBedstate, LV_BTN_STATE_REL, &bmp_pic_45x45);
    lv_imgbtn_set_src(buttonBedstate, LV_BTN_STATE_PR, &bmp_pic_45x45);	
	lv_imgbtn_set_style(buttonBedstate, LV_BTN_STATE_PR, &tft_style_label_pre);
	lv_imgbtn_set_style(buttonBedstate, LV_BTN_STATE_REL, &tft_style_label_rel);
	#endif
	lv_obj_set_event_cb_mks(buttonFanstate, event_handler, 0, "bmp_Fan_state.bin", 0);	
    lv_imgbtn_set_src(buttonFanstate, LV_BTN_STATE_REL, &bmp_pic_45x45);
    lv_imgbtn_set_src(buttonFanstate, LV_BTN_STATE_PR, &bmp_pic_45x45);	
	lv_imgbtn_set_style(buttonFanstate, LV_BTN_STATE_PR, &tft_style_label_pre);
	lv_imgbtn_set_style(buttonFanstate, LV_BTN_STATE_REL, &tft_style_label_rel);

	lv_obj_set_event_cb_mks(buttonTime, event_handler, 0, "bmp_Time_state.bin", 0);	
    lv_imgbtn_set_src(buttonTime, LV_BTN_STATE_REL, &bmp_pic_45x45);
    lv_imgbtn_set_src(buttonTime, LV_BTN_STATE_PR, &bmp_pic_45x45);	
	lv_imgbtn_set_style(buttonTime, LV_BTN_STATE_PR, &tft_style_label_pre);
	lv_imgbtn_set_style(buttonTime, LV_BTN_STATE_REL, &tft_style_label_rel);
	
	lv_obj_set_event_cb_mks(buttonZpos, event_handler, 0, "bmp_Zpos_state.bin", 0);	
    lv_imgbtn_set_src(buttonZpos, LV_BTN_STATE_REL, &bmp_pic_45x45);
    lv_imgbtn_set_src(buttonZpos, LV_BTN_STATE_PR, &bmp_pic_45x45);	
	lv_imgbtn_set_style(buttonZpos, LV_BTN_STATE_PR, &tft_style_label_pre);
	lv_imgbtn_set_style(buttonZpos, LV_BTN_STATE_REL, &tft_style_label_rel);
	if(uiCfg.print_state == WORKING)
		lv_obj_set_event_cb_mks(buttonPause, event_handler, ID_PAUSE, "bmp_Resume.bin", 0);
	else
		lv_obj_set_event_cb_mks(buttonPause, event_handler, ID_PAUSE, "bmp_Pause.bin", 0);
    lv_imgbtn_set_src(buttonPause, LV_BTN_STATE_REL, &bmp_pic_150x80);
    lv_imgbtn_set_src(buttonPause, LV_BTN_STATE_PR, &bmp_pic_150x80);	
	lv_imgbtn_set_style(buttonPause, LV_BTN_STATE_PR, &tft_style_label_pre);
	lv_imgbtn_set_style(buttonPause, LV_BTN_STATE_REL, &tft_style_label_rel);

	lv_obj_set_event_cb_mks(buttonStop, event_handler, ID_STOP, "bmp_Stop.bin",0);	
    lv_imgbtn_set_src(buttonStop, LV_BTN_STATE_REL, &bmp_pic_150x80);
    lv_imgbtn_set_src(buttonStop, LV_BTN_STATE_PR, &bmp_pic_150x80);	
	lv_imgbtn_set_style(buttonStop, LV_BTN_STATE_PR, &tft_style_label_pre);
	lv_imgbtn_set_style(buttonStop, LV_BTN_STATE_REL, &tft_style_label_rel);

	lv_obj_set_event_cb_mks(buttonOperat, event_handler, ID_OPTION, "bmp_Operate.bin", 0);	
    lv_imgbtn_set_src(buttonOperat, LV_BTN_STATE_REL, &bmp_pic_150x80);
    lv_imgbtn_set_src(buttonOperat, LV_BTN_STATE_PR, &bmp_pic_150x80);	
	lv_imgbtn_set_style(buttonOperat, LV_BTN_STATE_PR, &tft_style_label_pre);
	lv_imgbtn_set_style(buttonOperat, LV_BTN_STATE_REL, &tft_style_label_rel);
	
	lv_obj_set_pos(buttonExt1, 205, 136);
	if(EXTRUDERS == 2)
		lv_obj_set_pos(buttonExt2, 350, 136);
	#if HAS_HEATED_BED
	lv_obj_set_pos(buttonBedstate, 205, 186);
	#endif
	lv_obj_set_pos(buttonFanstate, 350, 186);
	lv_obj_set_pos(buttonTime, 205, 86);
	lv_obj_set_pos(buttonZpos, 350, 86);
	lv_obj_set_pos(buttonPause, 5, 240);
	lv_obj_set_pos(buttonStop, 165, 240);
	lv_obj_set_pos(buttonOperat, 325, 240);

    /*Create a label on the Image button*/
	lv_btn_set_layout(buttonExt1, LV_LAYOUT_OFF);
	if(EXTRUDERS == 2)
		lv_btn_set_layout(buttonExt2, LV_LAYOUT_OFF);
	#if HAS_HEATED_BED
	lv_btn_set_layout(buttonBedstate, LV_LAYOUT_OFF);
	#endif
	lv_btn_set_layout(buttonFanstate, LV_LAYOUT_OFF);
	lv_btn_set_layout(buttonZpos, LV_LAYOUT_OFF);
	lv_btn_set_layout(buttonPause, LV_LAYOUT_OFF);
	lv_btn_set_layout(buttonStop, LV_LAYOUT_OFF);
	lv_btn_set_layout(buttonOperat, LV_LAYOUT_OFF);
	
    labelExt1 = lv_label_create(scr, NULL);
	lv_obj_set_style(labelExt1, &tft_style_label_rel);
	lv_obj_set_pos(labelExt1, 250, 146);
	if(EXTRUDERS == 2)
	{
		labelExt2 = lv_label_create(scr, NULL);
		lv_obj_set_style(labelExt2, &tft_style_label_rel);
		lv_obj_set_pos(labelExt2, 395, 146);
	}
	#if HAS_HEATED_BED
	labelBed = lv_label_create(scr, NULL);
	lv_obj_set_style(labelBed, &tft_style_label_rel);
	lv_obj_set_pos(labelBed, 250, 196);
	#endif
	
	labelFan = lv_label_create(scr, NULL);
	lv_obj_set_style(labelFan, &tft_style_label_rel);
	lv_obj_set_pos(labelFan, 395, 196);
	
	labelZpos = lv_label_create(scr, NULL);
	lv_obj_set_style(labelZpos, &tft_style_label_rel);
	lv_obj_set_pos(labelZpos, 395, 96);

	labelTime = lv_label_create(scr, NULL);
	lv_obj_set_style(labelTime, &tft_style_label_rel);
	lv_obj_set_pos(labelTime, 250, 96);
	
	labelPause = lv_label_create(buttonPause, NULL);
	labelStop = lv_label_create(buttonStop, NULL);
	labelOperat = lv_label_create(buttonOperat, NULL);
	
	if(gCfgItems.multiple_language != 0)
	{
		if(uiCfg.print_state == WORKING)
		{
	    	lv_label_set_text(labelPause, printing_menu.pause);
		}
		else
		{
			lv_label_set_text(labelPause, printing_menu.resume);
		}
		lv_obj_align(labelPause, buttonPause, LV_ALIGN_CENTER, 30, 0);

		lv_label_set_text(labelStop, printing_menu.stop);
		lv_obj_align(labelStop, buttonStop, LV_ALIGN_CENTER, 30, 0);

		lv_label_set_text(labelOperat, printing_menu.option);
		lv_obj_align(labelOperat, buttonOperat, LV_ALIGN_CENTER, 30, 0);
	}
		
	lv_style_copy(&lv_bar_style_indic, &lv_style_pretty_color);
	lv_bar_style_indic.text.color        = lv_color_hex3(0xADF);
	lv_bar_style_indic.image.color       = lv_color_hex3(0xADF);
	lv_bar_style_indic.line.color        = lv_color_hex3(0xADF);
	lv_bar_style_indic.body.main_color   = lv_color_hex3(0xADF);
	lv_bar_style_indic.body.grad_color   = lv_color_hex3(0xADF);
	lv_bar_style_indic.body.border.color = lv_color_hex3(0xADF);
	bar1 = lv_bar_create(scr, NULL);
	lv_obj_set_pos(bar1, 205, 36);
	lv_obj_set_size(bar1, 270, 40); 
	lv_bar_set_style(bar1, LV_BAR_STYLE_INDIC, &lv_bar_style_indic);
	lv_bar_set_anim_time(bar1, 1000);
	lv_bar_set_value(bar1, 0, LV_ANIM_ON);

	bar1ValueText  = lv_label_create(bar1, NULL);
	lv_label_set_text(bar1ValueText,"0%");
	lv_obj_align(bar1ValueText, bar1, LV_ALIGN_CENTER, 0, 0);

	disp_ext_temp();
	disp_bed_temp();
	disp_fan_speed();
	disp_print_time();
	disp_fan_Zpos();
}

void disp_ext_temp(void)
{
	memset(public_buf_l, 0, sizeof(public_buf_l));
	sprintf(public_buf_l, printing_menu.temp1, (int)thermalManager.temp_hotend[0].celsius, (int)thermalManager.temp_hotend[0].target);
	lv_label_set_text(labelExt1, public_buf_l);

	if(EXTRUDERS == 2)
	{
		memset(public_buf_l, 0, sizeof(public_buf_l));
		sprintf(public_buf_l, printing_menu.temp1, (int)thermalManager.temp_hotend[1].celsius, (int)thermalManager.temp_hotend[1].target);
		lv_label_set_text(labelExt2, public_buf_l);
	}
}

void disp_bed_temp(void)
{
	#if HAS_HEATED_BED
	memset(public_buf_l, 0, sizeof(public_buf_l));
	sprintf(public_buf_l, printing_menu.bed_temp,(int)thermalManager.temp_bed.celsius, (int)thermalManager.temp_bed.target);
	lv_label_set_text(labelBed, public_buf_l);
	#endif
}

void disp_fan_speed(void)
{
	memset(public_buf_l, 0, sizeof(public_buf_l));
	sprintf(public_buf_l, "%3d", thermalManager.fan_speed[0]);
	lv_label_set_text(labelFan, public_buf_l);
}

void disp_print_time(void)
{
	memset(public_buf_l, 0, sizeof(public_buf_l));
	sprintf(public_buf_l, "%d%d:%d%d:%d%d", print_time.hours/10, print_time.hours%10, print_time.minutes/10, print_time.minutes%10,  print_time.seconds/10, print_time.seconds%10);
	lv_label_set_text(labelTime, public_buf_l);
}

void disp_fan_Zpos(void)
{
	memset(public_buf_l, 0, sizeof(public_buf_l));
	sprintf(public_buf_l,"%.3f", current_position[Z_AXIS]);
	lv_label_set_text(labelZpos, public_buf_l);
}

void reset_print_time(void)
{
	print_time.hours = 0;
	print_time.minutes = 0;
	print_time.seconds = 0;
	print_time.ms_10 = 0;
}

void start_print_time(void)
{
	print_time.start = 1;
}

void stop_print_time(void)
{
	print_time.start = 0;
}

void setProBarRate(void)
{
	int rate;
	volatile long long rate_tmp_r;
	
	if(gCfgItems.from_flash_pic != 1)
	{
		#if ENABLED (SDSUPPORT)
		rate_tmp_r =(long long)card.getIndex() * 100;
		#endif
		rate = rate_tmp_r / gCfgItems.curFilesize;
	}
	else
	{
		#if ENABLED (SDSUPPORT)
		rate_tmp_r =(long long)card.getIndex();
		#endif
		rate = (rate_tmp_r-(PREVIEW_SIZE+To_pre_view))* 100 / (gCfgItems.curFilesize-(PREVIEW_SIZE+To_pre_view));
	}
	
	if(rate <= 0)
		return;

	if(disp_state == PRINTING_UI)
	{
		lv_bar_set_value(bar1, rate, LV_ANIM_ON);
		ZERO(public_buf_l);
		sprintf_P(public_buf_l, "%d%%", rate);
		lv_label_set_text(bar1ValueText, public_buf_l);
		lv_obj_align(bar1ValueText, bar1, LV_ALIGN_CENTER, 0, 0);

		if(marlin_state == MF_SD_COMPLETE)
		{
			if(once_flag == 0)
			{					
				stop_print_time();				
				
				flash_preview_begin = 0;
				default_preview_flg = 0;
				lv_clear_printing();
				lv_draw_dialog(DIALOG_TYPE_FINISH_PRINT);
                			
				once_flag = 1;
				
				#if ENABLED(PSU_CONTROL)
				if(gCfgItems.finish_power_off == 1)
				//queue.enqueue_one_now(PSTR("M81"));
				PSU_OFF();
				#endif
			}					
		}
	}	
}

void lv_clear_printing(void)
{
	lv_obj_del(scr);
}

#endif
