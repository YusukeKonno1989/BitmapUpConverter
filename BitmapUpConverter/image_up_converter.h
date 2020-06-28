#ifndef  __IMAGE_UP_CONVERTER_H__
#define  __IMAGE_UP_CONVERTER_H__

enum{
	SELECT_32_BIT,
	SELECT_24_BIT,
	SELECT_16_BIT,
	SELECT_8_BIT,
	SELECT_4_BIT,
};

enum{
	SELECT_GRAY_SCALE = 1,  
	SELECT_SEPIA,		
	SELECT_NEGAPOSI,	
};	

enum ERROR_CODE {
	NO_ERR  = 0 , 
	ERR_CONVERT , 
	ERR_SELECT  ,
};

enum{
	BMP_MODE = 1,
	PNG_MODE
};

struct color_data{
	unsigned char blue,green,red,blank; // B,G,R,空白
};

struct color_data_origin{
	color_data *origin_datas;
	BITMAPFILEHEADER  header;	// ヘッダファイル
	BITMAPINFOHEADER  info;		// 情報ヘッダ	
};

////////////////////////////// プロトタイプ宣言 ////////////////////////////////////
color_data_origin *image_data_create( const char * , const int );
void image_data_up_convert( color_data_origin * , int );
int image_data_transfer( const int , color_data_origin * );
void image_data_write( color_data_origin * , const char * , const int , int );
void image_data_release( color_data_origin  * );
////////////////////////////////////////////////////////////////////////////////////

#endif //__IMAGE_UP_CONVERTER_H__