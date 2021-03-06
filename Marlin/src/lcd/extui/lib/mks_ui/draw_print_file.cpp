#include "../../../../MarlinCore.h"
#if ENABLED(TFT_LITTLE_VGL_UI)
#include "draw_ui.h"
#include "../../../../sd/cardreader.h"


#define ID_P_UP				7
#define ID_P_DOWN			8
#define ID_P_RETURN			9

static lv_obj_t *scr;
static lv_obj_t *buttonPageUp, *buttonPageDown, *buttonBack, *buttonGcode[FILE_BTN_CNT];
static lv_obj_t *labelPageUp[FILE_BTN_CNT];
static lv_obj_t *buttonText[FILE_BTN_CNT];

int8_t curDirLever = 0;
LIST_FILE list_file;
DIR_OFFSET dir_offset[10];

extern uint8_t public_buf[512];
extern char public_buf_m[100];

uint8_t sel_id=0;

#if ENABLED (SDSUPPORT)
static uint8_t search_file()
{
	int valid_name_cnt = 0;
	
	list_file.Sd_file_cnt = 0;
	
	if(curDirLever != 0)
		card.cd(list_file.curDirPath);
	else 
		card.cdroot();
	
    const uint16_t fileCnt = card.get_num_Files();
	for (uint16_t i = 0; i < fileCnt; i++)
	{
		if(list_file.Sd_file_cnt == list_file.Sd_file_offset)
		{
			const uint16_t nr =
			#if ENABLED(SDCARD_RATHERRECENTFIRST) && DISABLED(SDCARD_SORT_ALPHA)
				fileCnt - 1 -
			#endif
			i;

			#if ENABLED(SDCARD_SORT_ALPHA)
			card.getfilename_sorted(nr);
			#else
			card.getfilename_sorted(nr);
			#endif

			if (card.flag.filenameIsDir)
			{
				list_file.IsFolder[valid_name_cnt] = 1;
			}
			else
			{
				list_file.IsFolder[valid_name_cnt] = 0;
			}

			memset(list_file.file_name[valid_name_cnt], 0, strlen(list_file.file_name[valid_name_cnt]));
			strcpy(list_file.file_name[valid_name_cnt], list_file.curDirPath);
			strcat(list_file.file_name[valid_name_cnt], "/");
			strcat(list_file.file_name[valid_name_cnt], card.filename);
			memset(list_file.long_name[valid_name_cnt], 0, strlen(list_file.long_name[valid_name_cnt]));
			if(card.longFilename[0] == 0)
				strncpy(list_file.long_name[valid_name_cnt], card.filename, strlen(card.filename));
			else
				strncpy(list_file.long_name[valid_name_cnt], card.longFilename, strlen(card.longFilename));
			
			valid_name_cnt++;
			if (valid_name_cnt == 1)
				dir_offset[curDirLever].cur_page_first_offset = list_file.Sd_file_offset;

			if(valid_name_cnt >= FILE_NUM)
			{
				dir_offset[curDirLever].cur_page_last_offset = list_file.Sd_file_offset;
				list_file.Sd_file_offset++;					
				break;
			}
			list_file.Sd_file_offset++;	
		}
		list_file.Sd_file_cnt++;
	}
	return valid_name_cnt;
}
#endif

uint8_t have_pre_pic(char *path)
{
	#if ENABLED (SDSUPPORT)
	char *ps1,*ps2;
	char *cur_name;

	cur_name=strrchr(path, '/');
	card.openFileRead(cur_name);
	card.read(public_buf, 512);
	ps1 = strstr((char *)public_buf, ";simage:");
	card.read(public_buf, 512);
	ps2 = strstr((char *)public_buf, ";simage:");

	if(ps1 || ps2)
	{
		card.closefile();
		return 1;			
	}
	else
	{
		card.closefile();
		return 0;			
	}
	#endif
}

LV_IMG_DECLARE(bmp_pic_117x92);
LV_IMG_DECLARE(bmp_pic_100x100);
LV_IMG_DECLARE(bmp_pic);
LV_IMG_DECLARE(bmp_pic_100x40);

static void event_handler(lv_obj_t *obj, lv_event_t event)
{
	uint8_t i;
	uint8_t file_count;

	if(obj->mks_obj_id == ID_P_UP)
	{
	    if(event == LV_EVENT_CLICKED) {

	    }
	    else if(event == LV_EVENT_RELEASED) {
	        if(dir_offset[curDirLever].curPage > 0)
			{
				list_file.Sd_file_cnt = 0;
				if(dir_offset[curDirLever].cur_page_first_offset >= FILE_NUM)
					list_file.Sd_file_offset = dir_offset[curDirLever].cur_page_first_offset - FILE_NUM;
				#if ENABLED (SDSUPPORT)
				file_count = search_file();
				#endif
				if(file_count != 0)
				{
					dir_offset[curDirLever].curPage--;
					lv_obj_del(scr);
					disp_gcode_icon(file_count);
				}
			}
	    }
	}
	else if(obj->mks_obj_id == ID_P_DOWN)
	{
		if(event == LV_EVENT_CLICKED) {
			
	    }
	    else if(event == LV_EVENT_RELEASED) {
	        if(dir_offset[curDirLever].cur_page_last_offset > 0)
			{
				list_file.Sd_file_cnt = 0;
				list_file.Sd_file_offset = dir_offset[curDirLever].cur_page_last_offset + 1;
				#if ENABLED (SDSUPPORT)
				file_count = search_file();
				#endif
				if(file_count != 0)
				{
					dir_offset[curDirLever].curPage++;
					lv_obj_del(scr);
					disp_gcode_icon(file_count);
				}
				if(file_count < FILE_NUM)
				{
					dir_offset[curDirLever].cur_page_last_offset = 0;
				}
			}
	    }
	}
	else if(obj->mks_obj_id == ID_P_RETURN)
	{
	    if(event == LV_EVENT_CLICKED) {
			
	    }
	    else if(event == LV_EVENT_RELEASED) {
			if(curDirLever > 0)
			{
				int8_t *ch = 0;
				
				ch =  (int8_t *)strrchr(list_file.curDirPath, '/');
				if(ch != 0)
				{
					*ch = 0;
					#if ENABLED (SDSUPPORT)
					card.cdup();
					#endif
					dir_offset[curDirLever].curPage = 0;
					dir_offset[curDirLever].cur_page_first_offset = 0;
					dir_offset[curDirLever].cur_page_last_offset = 0;
					curDirLever--;
					list_file.Sd_file_offset = dir_offset[curDirLever].cur_page_first_offset;
					#if ENABLED (SDSUPPORT)
					file_count = search_file();
					#endif
					lv_obj_del(scr);
					disp_gcode_icon(file_count);
				}
			}
			else
			{
				lv_clear_print_file();
				lv_draw_ready_print(1);
			}
	    }
	}
	else
	{
		for(i = 0; i < FILE_BTN_CNT; i++)
		{
			if(obj->mks_obj_id == (i+1))
			{
				if(event == LV_EVENT_CLICKED) {
			
	    		}
				else if(event == LV_EVENT_RELEASED) {
		        	if(list_file.file_name[i][0] != 0)
					{
						if(list_file.IsFolder[i] == 1)
						{
							memset(list_file.curDirPath, 0, sizeof(list_file.curDirPath));
							strcpy(list_file.curDirPath, list_file.file_name[i]);
							curDirLever++;
							list_file.Sd_file_offset = dir_offset[curDirLever].cur_page_first_offset;
							#if ENABLED (SDSUPPORT)
							file_count = search_file();
							#endif
							lv_obj_del(scr);
							disp_gcode_icon(file_count);
						}
						else
						{
							sel_id = i;
							lv_obj_del(scr);
							lv_draw_dialog(DIALOG_TYPE_PRINT_FILE);
						}
						break;
					}
				}
			}
		}
	}
}

void lv_draw_print_file(void)
{
	uint8_t file_count;

	if(disp_state_stack._disp_state[disp_state_stack._disp_index] != PRINT_FILE_UI)
	{
		disp_state_stack._disp_index++;
		disp_state_stack._disp_state[disp_state_stack._disp_index] = PRINT_FILE_UI;
	}
	disp_state = PRINT_FILE_UI;

	curDirLever = 0;	
	dir_offset[curDirLever].curPage = 0;

	list_file.Sd_file_offset = 0;
	list_file.Sd_file_cnt = 0;    

	memset(dir_offset, 0, sizeof(dir_offset));
	memset(list_file.IsFolder, 0, sizeof(list_file.IsFolder));
	memset(list_file.curDirPath, 0, sizeof(list_file.curDirPath));

	list_file.Sd_file_offset = dir_offset[curDirLever].cur_page_first_offset;
	#if ENABLED (SDSUPPORT)
	card.mount();
	file_count = search_file();
	#endif
	disp_gcode_icon(file_count);
}

void disp_gcode_icon(uint8_t file_num)
{
	uint8_t i;

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

	buttonPageUp = lv_imgbtn_create(scr, NULL);
	buttonPageDown = lv_imgbtn_create(scr, NULL);
	buttonBack = lv_imgbtn_create(scr, NULL);

	lv_obj_set_event_cb_mks(buttonPageUp, event_handler, ID_P_UP, "bmp_pageUp.bin", 0);
	lv_imgbtn_set_src(buttonPageUp, LV_BTN_STATE_REL, &bmp_pic_117x92);
	lv_imgbtn_set_src(buttonPageUp, LV_BTN_STATE_PR, &bmp_pic_117x92);
	lv_imgbtn_set_style(buttonPageUp, LV_BTN_STATE_PR, &tft_style_label_pre);
	lv_imgbtn_set_style(buttonPageUp, LV_BTN_STATE_REL, &tft_style_label_rel);
	lv_obj_clear_protect(buttonPageUp, LV_PROTECT_FOLLOW);
	
	lv_obj_set_event_cb_mks(buttonPageDown, event_handler, ID_P_DOWN, "bmp_pageDown.bin", 0);
	lv_imgbtn_set_src(buttonPageDown, LV_BTN_STATE_REL, &bmp_pic_117x92);
	lv_imgbtn_set_src(buttonPageDown, LV_BTN_STATE_PR, &bmp_pic_117x92);
	lv_imgbtn_set_style(buttonPageDown, LV_BTN_STATE_PR, &tft_style_label_pre);
	lv_imgbtn_set_style(buttonPageDown, LV_BTN_STATE_REL, &tft_style_label_rel);

	lv_obj_set_event_cb_mks(buttonBack, event_handler, ID_P_RETURN, "bmp_FileBack.bin", 0);	
    lv_imgbtn_set_src(buttonBack, LV_BTN_STATE_REL, &bmp_pic_117x92);
    lv_imgbtn_set_src(buttonBack, LV_BTN_STATE_PR, &bmp_pic_117x92);	
	lv_imgbtn_set_style(buttonBack, LV_BTN_STATE_PR, &tft_style_label_pre);
	lv_imgbtn_set_style(buttonBack, LV_BTN_STATE_REL, &tft_style_label_rel);

	lv_obj_set_pos(buttonPageUp, OTHER_BTN_XPIEL*3+INTERVAL_V*4, titleHeight);
	lv_obj_set_pos(buttonPageDown, OTHER_BTN_XPIEL*3+INTERVAL_V*4, titleHeight+OTHER_BTN_YPIEL+INTERVAL_H);
	lv_obj_set_pos(buttonBack, OTHER_BTN_XPIEL*3+INTERVAL_V*4, titleHeight+OTHER_BTN_YPIEL*2+INTERVAL_H*2);

    /*Create a label on the Image button*/
	lv_btn_set_layout(buttonPageUp, LV_LAYOUT_OFF);
	lv_btn_set_layout(buttonPageDown, LV_LAYOUT_OFF);
	lv_btn_set_layout(buttonBack, LV_LAYOUT_OFF);

	for(i = 0; i < FILE_BTN_CNT; i++)
	{
		if(i  >= file_num)
		{
			break;
		}
	
		#ifdef TFT35
		buttonGcode[i] = lv_imgbtn_create(scr, NULL);
			
		lv_imgbtn_set_style(buttonGcode[i], LV_BTN_STATE_PR, &tft_style_label_pre);
		lv_imgbtn_set_style(buttonGcode[i], LV_BTN_STATE_REL, &tft_style_label_rel);
		lv_obj_clear_protect(buttonGcode[i], LV_PROTECT_FOLLOW);
		lv_btn_set_layout(buttonGcode[i], LV_LAYOUT_OFF);

		memset(public_buf_m, 0, sizeof(public_buf_m));
		cutFileName((char *)list_file.long_name[i], 16, 8, (char *)public_buf_m);
		
		if(list_file.IsFolder[i] == 1)
		{
			lv_obj_set_event_cb_mks(buttonGcode[i], event_handler, (i+1), "bmp_Dir.bin",0);
			lv_imgbtn_set_src(buttonGcode[i], LV_BTN_STATE_REL, &bmp_pic);
			lv_imgbtn_set_src(buttonGcode[i], LV_BTN_STATE_PR, &bmp_pic);
			if(i < 3)
				lv_obj_set_pos(buttonGcode[i], BTN_X_PIXEL*i+INTERVAL_V*(i+1), titleHeight);
			else
				lv_obj_set_pos(buttonGcode[i], BTN_X_PIXEL*(i-3)+INTERVAL_V*((i-3)+1), BTN_Y_PIXEL+INTERVAL_H+titleHeight);

			labelPageUp[i] = lv_label_create(buttonGcode[i], NULL);
			lv_obj_set_style(labelPageUp[i], &tft_style_label_rel);
			lv_label_set_text(labelPageUp[i], public_buf_m);
			lv_obj_align(labelPageUp[i], buttonGcode[i], LV_ALIGN_IN_BOTTOM_MID, 0, -5);
		}
		else
		{
			if(have_pre_pic((char *)list_file.file_name[i]))
			{
				lv_obj_set_event_cb_mks(buttonGcode[i], event_handler, (i+1),list_file.file_name[i], 1);
				lv_imgbtn_set_src(buttonGcode[i], LV_BTN_STATE_REL, &bmp_pic_100x100);
				lv_imgbtn_set_src(buttonGcode[i], LV_BTN_STATE_PR, &bmp_pic_100x100);
				
				if(i < 3)
				{
					lv_obj_set_pos(buttonGcode[i], BTN_X_PIXEL*i+INTERVAL_V*(i+1)+FILE_PRE_PIC_X_OFFSET, titleHeight+FILE_PRE_PIC_Y_OFFSET);
					buttonText[i] = lv_btn_create(scr, NULL);					
					lv_btn_set_style(buttonText[i], LV_BTN_STATE_PR, &tft_style_label_pre);
					lv_btn_set_style(buttonText[i], LV_BTN_STATE_REL, &tft_style_label_rel);
					lv_obj_clear_protect(buttonText[i], LV_PROTECT_FOLLOW);
					lv_btn_set_layout(buttonText[i], LV_LAYOUT_OFF);
					lv_obj_set_pos(buttonText[i], BTN_X_PIXEL*i+INTERVAL_V*(i+1)+FILE_PRE_PIC_X_OFFSET, titleHeight+FILE_PRE_PIC_Y_OFFSET+100);
					lv_obj_set_size(buttonText[i], 100, 40);
				}
				else
				{
					lv_obj_set_pos(buttonGcode[i], BTN_X_PIXEL*(i-3)+INTERVAL_V*((i-3)+1)+FILE_PRE_PIC_X_OFFSET, BTN_Y_PIXEL+INTERVAL_H+titleHeight+FILE_PRE_PIC_Y_OFFSET);
					buttonText[i] = lv_btn_create(scr, NULL);					
					lv_btn_set_style(buttonText[i], LV_BTN_STATE_PR, &tft_style_label_pre);
					lv_btn_set_style(buttonText[i], LV_BTN_STATE_REL, &tft_style_label_rel);
					lv_obj_clear_protect(buttonText[i], LV_PROTECT_FOLLOW);
					lv_btn_set_layout(buttonText[i], LV_LAYOUT_OFF);
					lv_obj_set_pos(buttonText[i], BTN_X_PIXEL*(i-3)+INTERVAL_V*((i-3)+1)+FILE_PRE_PIC_X_OFFSET, BTN_Y_PIXEL+INTERVAL_H+titleHeight+FILE_PRE_PIC_Y_OFFSET+100);
					lv_obj_set_size(buttonText[i], 100, 40);
				}
				labelPageUp[i] = lv_label_create(buttonText[i], NULL);
				lv_obj_set_style(labelPageUp[i], &tft_style_label_rel);
				lv_label_set_text(labelPageUp[i], public_buf_m);
				lv_obj_align(labelPageUp[i], buttonText[i], LV_ALIGN_IN_BOTTOM_MID, 0, 0);
			}
			else
			{
				lv_obj_set_event_cb_mks(buttonGcode[i], event_handler,(i+1), "bmp_File.bin", 0);
				lv_imgbtn_set_src(buttonGcode[i], LV_BTN_STATE_REL, &bmp_pic);
				lv_imgbtn_set_src(buttonGcode[i], LV_BTN_STATE_PR, &bmp_pic);
				if(i < 3)
					lv_obj_set_pos(buttonGcode[i], BTN_X_PIXEL*i+INTERVAL_V*(i+1), titleHeight);
				else
					lv_obj_set_pos(buttonGcode[i], BTN_X_PIXEL*(i-3)+INTERVAL_V*((i-3)+1), BTN_Y_PIXEL+INTERVAL_H+titleHeight);

				labelPageUp[i] = lv_label_create(buttonGcode[i], NULL);
				lv_obj_set_style(labelPageUp[i], &tft_style_label_rel);
				lv_label_set_text(labelPageUp[i], public_buf_m);
				lv_obj_align(labelPageUp[i], buttonGcode[i], LV_ALIGN_IN_BOTTOM_MID, 0, -5);
			}
		}
        #else
        #endif    
	}
}

void lv_open_gcode_file(char *path)
{
	#if ENABLED (SDSUPPORT)
	uint32_t *ps4;
	int pre_sread_cnt;
	char *cur_name;

	cur_name=strrchr(path, '/');
	card.openFileRead(cur_name);
	card.read(public_buf, 512);
	ps4 = (uint32_t *)strstr((char *)public_buf, ";simage:");

	if(ps4)
	{
		pre_sread_cnt = (uint32_t)ps4-(uint32_t)((uint32_t *)(&public_buf[0]));
		card.setIndex(pre_sread_cnt+8);	
	}
	#endif
}

int ascii2dec_test(char *ascii)
{
	int result = 0;

	if(ascii == 0)
		return 0;

	if(*(ascii) >= '0' && *(ascii) <= '9')
		result = *(ascii) - '0';
	else if(*(ascii) >= 'a' && *(ascii) <= 'f')
		result = *(ascii) - 'a' + 0x0a;
	else if(*(ascii) >= 'A' && *(ascii) <= 'F')
		result = *(ascii) - 'A' + 0x0a;
	else
		return 0;

	return result;
}

void lv_gcode_file_read(uint8_t *data_buf)
{
	#if ENABLED (SDSUPPORT)
	uint16_t i=0, j=0, k=0;
	uint16_t row_1=0;
	char temp_test[200];

	while(1)
	{
		card.read(temp_test, 200);
		for(i=0; i<200; )
		{
			public_buf[row_1*200+100*k+j] = (char)(ascii2dec_test(&temp_test[i])<<4|ascii2dec_test(&temp_test[i+1]));
			j++;
			i+=2;
		}
		k++;
		j=0;
		if(k*100>=200)
		{
			k=0;
			card.read(temp_test, 9);
			break;
		}
	}
	memcpy(data_buf, public_buf, 200);
	#endif
}

void lv_close_gcode_file(void)
{
	#if ENABLED (SDSUPPORT)
	card.closefile();
	#endif
}

void cutFileName(char *path, int len, int bytePerLine, char *outStr)
{
	#if _LFN_UNICODE 
	TCHAR *tmpFile;
	TCHAR *strIndex1 = 0, *strIndex2 = 0, *beginIndex;
	TCHAR secSeg[10] = {0};
	TCHAR gFileTail[4] = {'~', '.', 'g', '\0'};
	#else
	char *tmpFile;
	char *strIndex1 = 0, *strIndex2 = 0, *beginIndex;
	char secSeg[10] = {0};
	#endif
	
	if(path == 0 || len <= 3 || outStr == 0)
	{
		return;
	}
	
	tmpFile = path;
	#if _LFN_UNICODE
	strIndex1 = (WCHAR *)wcsstr((const WCHAR *)tmpFile, (const WCHAR *)'/');
	strIndex2 = (WCHAR *)wcsstr((const WCHAR *)tmpFile, (const WCHAR *)'.');
	#else
	strIndex1 = (char *)strrchr(tmpFile, '/');
	strIndex2 = (char *)strrchr(tmpFile, '.');
	#endif

	if(strIndex1 != 0)
	{
		beginIndex = strIndex1 + 1;	
	}
	/*if((strIndex1 != 0) && (strIndex2 != 0) && (strIndex1 < strIndex2))
	{
		beginIndex = strIndex1 + 1;			
	}*/
	else
	{
		beginIndex = tmpFile;
	}

	if(strIndex2 == 0 ||  (strIndex1 > strIndex2))	//not gcode file
	{
		#if _LFN_UNICODE
		if(wcslen(beginIndex) > len)
		{
			wcsncpy(outStr, beginIndex, len);
		
		}
		else
		{
			wcscpy(outStr, beginIndex);
		}
		#else
		if((int)strlen(beginIndex) > len)
		{
			strncpy(outStr, beginIndex, len);
		
		}
		else
		{
			strcpy(outStr, beginIndex);
		}
		#endif
	}
	else //gcode file
	{
		if(strIndex2 - beginIndex > (len - 2))
		{
			#if _LFN_UNICODE
				wcsncpy(outStr, (const WCHAR *)beginIndex, len - 3);
				wcscat(outStr, (const WCHAR *)gFileTail);
			#else
				//strncpy(outStr, beginIndex, len - 3);
				strncpy(outStr, beginIndex, len - 4);
				strcat(outStr, "~.g");
			#endif
		
		}
		else
		{
			#if _LFN_UNICODE
				wcsncpy(outStr, (const WCHAR *)beginIndex, strIndex2 - beginIndex + 1);
				wcscat(outStr, (const WCHAR *)&gFileTail[3]);
			#else
				strncpy(outStr, beginIndex, strIndex2 - beginIndex + 1);
				strcat(outStr, "g");
			#endif
		}
		
	}
	#if _LFN_UNICODE
	if(wcslen(outStr) > bytePerLine)
	{
		
		wcscpy(secSeg, (const WCHAR *)&outStr[bytePerLine]);
		outStr[bytePerLine] = '\n';
		outStr[bytePerLine + 1] = '\0';
		wcscat(outStr, (const WCHAR *)secSeg);
		
	}
	#else
	if((int)strlen(outStr) > bytePerLine)
	{
		
		strcpy(secSeg, &outStr[bytePerLine]);
		outStr[bytePerLine] = '\n';
		outStr[bytePerLine + 1] = '\0';
		strcat(outStr, secSeg);
		
	}
	else
	{
		strcat(outStr,"\n");
	}
	#endif
}

void lv_clear_print_file(void)
{
	lv_obj_del(scr);
}

#endif
