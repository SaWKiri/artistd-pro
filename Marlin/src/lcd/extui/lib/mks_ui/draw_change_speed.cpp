#include "../../../../MarlinCore.h"
#if ENABLED(TFT_LITTLE_VGL_UI)
#include "draw_ui.h"
#include "../../../../module/planner.h"


#define ID_C_ADD		1
#define ID_C_DEC		2
#define ID_C_MOVE		3
#define ID_C_EXT		4
#define ID_C_STEP		5
#define ID_C_RETURN		6

static lv_obj_t *scr;
static lv_obj_t *labelStep, *buttonStep, *buttonMov, *buttonExt;
static lv_obj_t *labelMov, *labelExt;
static lv_obj_t *printSpeedText;

static uint8_t speedType;

static void event_handler(lv_obj_t *obj, lv_event_t event)
{
	switch(obj->mks_obj_id)
	{
		case ID_C_ADD:
			if(event == LV_EVENT_CLICKED) {

			}
			else if(event == LV_EVENT_RELEASED) {
			if(speedType == 0)
				{
					if(feedrate_percentage  < MAX_EXT_SPEED_PERCENT - uiCfg.stepPrintSpeed)
					{
						feedrate_percentage += uiCfg.stepPrintSpeed;
					}
					else
					{
						feedrate_percentage = MAX_EXT_SPEED_PERCENT;
					}
				}
				else if(speedType == 1)
				{
					if(planner.flow_percentage[0]< MAX_EXT_SPEED_PERCENT - uiCfg.stepPrintSpeed)
					{
						planner.flow_percentage[0] += uiCfg.stepPrintSpeed;
					}
					else
					{
						planner.flow_percentage[0] = MAX_EXT_SPEED_PERCENT;
					}

					planner.refresh_e_factor(0);

					if(EXTRUDERS == 2)
					{
						planner.flow_percentage[1] = planner.flow_percentage[0];
						planner.refresh_e_factor(1);
					}
				}
				disp_print_speed();
			}
			break;
		case ID_C_DEC:
			if(event == LV_EVENT_CLICKED) {
				
			}
			else if(event == LV_EVENT_RELEASED) {
				if(speedType == 0)
				{						
					if(feedrate_percentage > MIN_EXT_SPEED_PERCENT + uiCfg.stepPrintSpeed)
					{
						feedrate_percentage -= uiCfg.stepPrintSpeed;							
					}
					else
					{
						feedrate_percentage = MIN_EXT_SPEED_PERCENT;
					}					
				}
				else if(speedType == 1)
				{
					if(planner.flow_percentage[0] > MIN_EXT_SPEED_PERCENT + uiCfg.stepPrintSpeed)
					{
						planner.flow_percentage[0] -= uiCfg.stepPrintSpeed;							
					}
					else
					{
						planner.flow_percentage[0] = MIN_EXT_SPEED_PERCENT;
					}
	
					planner.refresh_e_factor(0);

					if(EXTRUDERS == 2)
					{
						planner.flow_percentage[1] = planner.flow_percentage[0];
						planner.refresh_e_factor(1);
					}
				}
				disp_print_speed();
			}
			break;
		case ID_C_MOVE:
			if(event == LV_EVENT_CLICKED) {

			}
			else if(event == LV_EVENT_RELEASED) {
				speedType = 0;
				disp_speed_type();
				disp_print_speed();
			}
			break;
		case ID_C_EXT:
			if(event == LV_EVENT_CLICKED) {

			}
			else if(event == LV_EVENT_RELEASED) {
				speedType = 1;
				disp_speed_type();
				disp_print_speed();
			}
			break;
		case ID_C_STEP:
			if(event == LV_EVENT_CLICKED) {
				
			}
			else if(event == LV_EVENT_RELEASED) {
				if(uiCfg.stepPrintSpeed == 1)
				{
					uiCfg.stepPrintSpeed = 5;
				}
				else if(uiCfg.stepPrintSpeed == 5)
				{
					uiCfg.stepPrintSpeed = 10;						
				}
				else
				{
					uiCfg.stepPrintSpeed = 1;	
				}
				disp_speed_step();
			}
			break;
		case ID_C_RETURN:
			if(event == LV_EVENT_CLICKED) {
				
			}
			else if(event == LV_EVENT_RELEASED) {
				lv_clear_change_speed();
				draw_return_ui();
			}
			break;
	}
}

void lv_draw_change_speed(void)
{
	if(disp_state_stack._disp_state[disp_state_stack._disp_index] != CHANGE_SPEED_UI)
	{
		disp_state_stack._disp_index++;
		disp_state_stack._disp_state[disp_state_stack._disp_index] = CHANGE_SPEED_UI;
	}
	disp_state = CHANGE_SPEED_UI;

	scr = lv_obj_create(NULL, NULL);
	
	lv_obj_set_style(scr, &tft_style_scr);
  	lv_scr_load(scr);
  	lv_obj_clean(scr);

  	lv_obj_t *title = lv_label_create(scr, NULL);
	lv_obj_set_style(title, &tft_style_label_rel);
	lv_obj_set_pos(title, TITLE_XPOS, TITLE_YPOS);
	lv_label_set_text(title, creat_title_text());
  
  	lv_refr_now(lv_refr_get_disp_refreshing());
	
	LV_IMG_DECLARE(bmp_pic);
	
    /*Create an Image button*/
    lv_obj_t *buttonAdd = lv_imgbtn_create(scr, NULL);
	lv_obj_t *buttonDec = lv_imgbtn_create(scr, NULL);
	buttonMov = lv_imgbtn_create(scr, NULL);
	buttonExt = lv_imgbtn_create(scr, NULL);
	buttonStep = lv_imgbtn_create(scr, NULL);
	lv_obj_t *buttonBack = lv_imgbtn_create(scr, NULL);

	lv_obj_set_event_cb_mks(buttonAdd, event_handler, ID_C_ADD, "bmp_Add.bin",0);
	lv_imgbtn_set_src(buttonAdd, LV_BTN_STATE_REL, &bmp_pic);
	lv_imgbtn_set_src(buttonAdd, LV_BTN_STATE_PR, &bmp_pic);
	lv_imgbtn_set_style(buttonAdd, LV_BTN_STATE_PR, &tft_style_label_pre);
	lv_imgbtn_set_style(buttonAdd, LV_BTN_STATE_REL, &tft_style_label_rel);
	lv_obj_clear_protect(buttonAdd, LV_PROTECT_FOLLOW);

	lv_obj_set_event_cb_mks(buttonDec, event_handler, ID_C_DEC, "bmp_Dec.bin",0);
	lv_imgbtn_set_src(buttonDec, LV_BTN_STATE_REL, &bmp_pic);
	lv_imgbtn_set_src(buttonDec, LV_BTN_STATE_PR, &bmp_pic);
	lv_imgbtn_set_style(buttonDec, LV_BTN_STATE_PR, &tft_style_label_pre);
	lv_imgbtn_set_style(buttonDec, LV_BTN_STATE_REL, &tft_style_label_rel);

    lv_imgbtn_set_src(buttonMov, LV_BTN_STATE_REL, &bmp_pic);
    lv_imgbtn_set_src(buttonMov, LV_BTN_STATE_PR, &bmp_pic);	
	lv_imgbtn_set_style(buttonMov, LV_BTN_STATE_PR, &tft_style_label_pre);
	lv_imgbtn_set_style(buttonMov, LV_BTN_STATE_REL, &tft_style_label_rel);

    lv_imgbtn_set_src(buttonExt, LV_BTN_STATE_REL, &bmp_pic);
    lv_imgbtn_set_src(buttonExt, LV_BTN_STATE_PR, &bmp_pic);	
	lv_imgbtn_set_style(buttonExt, LV_BTN_STATE_PR, &tft_style_label_pre);
	lv_imgbtn_set_style(buttonExt, LV_BTN_STATE_REL, &tft_style_label_rel);
	
    lv_imgbtn_set_src(buttonStep, LV_BTN_STATE_REL, &bmp_pic);
    lv_imgbtn_set_src(buttonStep, LV_BTN_STATE_PR, &bmp_pic);	
	lv_imgbtn_set_style(buttonStep, LV_BTN_STATE_PR, &tft_style_label_pre);
	lv_imgbtn_set_style(buttonStep, LV_BTN_STATE_REL, &tft_style_label_rel);

	lv_obj_set_event_cb_mks(buttonBack, event_handler, ID_C_RETURN, "bmp_Return.bin",0);	
    lv_imgbtn_set_src(buttonBack, LV_BTN_STATE_REL, &bmp_pic);
    lv_imgbtn_set_src(buttonBack, LV_BTN_STATE_PR, &bmp_pic);	
	lv_imgbtn_set_style(buttonBack, LV_BTN_STATE_PR, &tft_style_label_pre);
	lv_imgbtn_set_style(buttonBack, LV_BTN_STATE_REL, &tft_style_label_rel);
	
	lv_obj_set_pos(buttonAdd, INTERVAL_V, titleHeight);
	lv_obj_set_pos(buttonDec, BTN_X_PIXEL*3+INTERVAL_V*4, titleHeight);
	lv_obj_set_pos(buttonMov, INTERVAL_V, BTN_Y_PIXEL+INTERVAL_H+titleHeight);
	lv_obj_set_pos(buttonExt, BTN_X_PIXEL+INTERVAL_V*2, BTN_Y_PIXEL+INTERVAL_H+titleHeight);
	lv_obj_set_pos(buttonStep, BTN_X_PIXEL*2+INTERVAL_V*3, BTN_Y_PIXEL+INTERVAL_H+titleHeight);
	lv_obj_set_pos(buttonBack, BTN_X_PIXEL*3+INTERVAL_V*4, BTN_Y_PIXEL+INTERVAL_H+titleHeight);

    /*Create a label on the Image button*/
	lv_btn_set_layout(buttonAdd, LV_LAYOUT_OFF);
	lv_btn_set_layout(buttonDec, LV_LAYOUT_OFF);
	lv_btn_set_layout(buttonMov, LV_LAYOUT_OFF);
	lv_btn_set_layout(buttonExt, LV_LAYOUT_OFF);
	lv_btn_set_layout(buttonStep, LV_LAYOUT_OFF);
	lv_btn_set_layout(buttonBack, LV_LAYOUT_OFF);
	
    lv_obj_t *labelAdd = lv_label_create(buttonAdd, NULL);
	lv_obj_t *labelDec = lv_label_create(buttonDec, NULL);
	labelMov = lv_label_create(buttonMov, NULL);
	labelExt = lv_label_create(buttonExt, NULL);
	labelStep = lv_label_create(buttonStep, NULL);
	lv_obj_t *label_Back = lv_label_create(buttonBack, NULL);
	
	
	if(gCfgItems.multiple_language != 0)
	{
	    lv_label_set_text(labelAdd, speed_menu.add);
		lv_obj_align(labelAdd, buttonAdd, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);

		lv_label_set_text(labelDec, speed_menu.dec);
		lv_obj_align(labelDec, buttonDec, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);

		lv_label_set_text(label_Back, common_menu.text_back);
		lv_obj_align(label_Back, buttonBack, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
	}
	disp_speed_type();
	disp_speed_step();

	printSpeedText = lv_label_create(scr, NULL);
	lv_obj_set_style(printSpeedText, &tft_style_label_rel);
	disp_print_speed();
}

void disp_speed_step(void)
{
	if(uiCfg.stepPrintSpeed == 1)
	{
		lv_obj_set_event_cb_mks(buttonStep, event_handler, ID_C_STEP, "bmp_Step1_percent.bin", 0);  							
	}
	else if(uiCfg.stepPrintSpeed == 5)
	{
		lv_obj_set_event_cb_mks(buttonStep, event_handler, ID_C_STEP, "bmp_Step5_percent.bin", 0);  										
	}
	else if(uiCfg.stepPrintSpeed == 10)
	{         								
		lv_obj_set_event_cb_mks(buttonStep, event_handler, ID_C_STEP, "bmp_Step10_percent.bin", 0);										
	}

	if(gCfgItems.multiple_language != 0)
	{
		if(uiCfg.stepPrintSpeed == 1)
		{
			lv_label_set_text(labelStep, speed_menu.step_1percent);
			lv_obj_align(labelStep, buttonStep, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
		}
		else if(uiCfg.stepPrintSpeed == 5)
		{
			lv_label_set_text(labelStep, speed_menu.step_5percent);
			lv_obj_align(labelStep, buttonStep, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);	
		}
		else if(uiCfg.stepPrintSpeed == 10)
		{
			lv_label_set_text(labelStep, speed_menu.step_10percent);
			lv_obj_align(labelStep, buttonStep, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
		}
	}
}

void disp_print_speed(void)
{
	char buf[30] = {0};

	public_buf_l[0] = '\0';
	
	if(speedType ==  0)	//move
	{
		strcat(public_buf_l, speed_menu.move_speed);
		strcat(public_buf_l, ": ");
		sprintf(buf, "%d%%", feedrate_percentage);
		strcat(public_buf_l, buf);
	}
	else if(speedType ==  1)	// e1
	{
		strcat(public_buf_l, speed_menu.extrude_speed);
		strcat(public_buf_l, ": ");
		sprintf(buf, "%d%%", planner.flow_percentage[0]);
		strcat(public_buf_l, buf);
	}
	lv_label_set_text(printSpeedText, public_buf_l);
	lv_obj_align(printSpeedText, NULL, LV_ALIGN_CENTER, 0, -65);
}

void disp_speed_type(void)
{
	switch(speedType)
	{
		case 1:
			lv_obj_set_event_cb_mks(buttonExt, event_handler, ID_C_EXT, "bmp_Extruct_speed_sel.bin", 0);
			lv_obj_set_event_cb_mks(buttonMov, event_handler, ID_C_MOVE, "bmp_Mov_speed.bin", 0);	
			break;
		default:
			lv_obj_set_event_cb_mks(buttonExt, event_handler, ID_C_EXT, "bmp_Extruct_speed.bin", 0);
			lv_obj_set_event_cb_mks(buttonMov, event_handler, ID_C_MOVE, "bmp_Mov_speed_sel.bin", 0);			
			break;
	}
	lv_obj_refresh_ext_draw_pad(buttonExt);
	lv_obj_refresh_ext_draw_pad(buttonMov);
		
	if(gCfgItems.multiple_language != 0)
	{
		lv_label_set_text(labelMov, speed_menu.move);
		lv_obj_align(labelMov, buttonMov, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);

		lv_label_set_text(labelExt, speed_menu.extrude);
		lv_obj_align(labelExt, buttonExt, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
	}

}

void lv_clear_change_speed(void)
{
	lv_obj_del(scr);
}
#endif
