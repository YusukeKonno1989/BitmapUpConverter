#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <math.h>
#include <zlib.h>
#include <png.h>
#include <cstdint>
#include "image_up_converter.h"

static const int PALLET_SIZE_8_BIT = 256;
static const int PALLET_SIZE_4_BIT =  16;
static const int TYPE_32_BIT	   =  32;
static const int TYPE_24_BIT	   =  24;
static const int TYPE_16_BIT	   =  16;
static const int TYPE_8_BIT		   =   8;
static const int TYPE_4_BIT		   =   4;

inline bool operator==( const color_data& lhs , const color_data& rhs ) {
	return *(unsigned long*)&lhs == *(unsigned long*)&rhs;
}

// �t�@�C���I�[�v���֐�
static FILE *file_open( const char *mode , const char *file_name ) {
	FILE *fp;
	errno_t error;
	if( error = fopen_s( &fp , file_name , mode ) != 0 ){ 
		return NULL;
	}
	return fp;
}

// �t�@�C���N���[�Y�֐�
static void file_close( FILE *fp ){
	fclose( fp );
}

// �t�@�C���w�b�_�[�����ۑ��֐�
static void image_header_data_load( color_data_origin *color_data_origin , FILE *fp ) {
	fread( &color_data_origin->header , sizeof( color_data_origin->header ) , 1 , fp );
	fread( &color_data_origin->info   , sizeof( color_data_origin->info )   , 1 , fp );
}

// PNG�t�@�C���ꊇ�ǂݍ��݊֐�
static color_data_origin *png_image_data_load( color_data_origin *color_data_origin , const char *import_file_name , int file_change_flg ){

	// png�p�\���̍쐬
	png_image png;
	memset( &png , 0 , sizeof( png ) );
	png.version = PNG_IMAGE_VERSION;
	png_image_begin_read_from_file( &png , import_file_name );

	// �ǂݏo�������ɍ��킹�āA�f�[�^�����i�[����o�b�t�@���m��
	uint32_t stride = PNG_IMAGE_ROW_STRIDE( png );
	uint8_t  *buf   = new uint8_t[ PNG_IMAGE_BUFFER_SIZE( png , stride ) ];

	// �f�[�^�����擾 bmp�ɕϊ�����Ȃ�stride�𔽓]�����Ă���
	if( file_change_flg == 1 ){ file_change_flg = -1; }
	else{ file_change_flg = 1; }
	png_image_finish_read( &png , NULL , buf , stride * file_change_flg , NULL );

	// �o�C�g���ɉ������z����쐬
	color_data *three_color_data = new color_data[ png.height * png.width ];
	memset( three_color_data , 0 , ( png.height * png.width ) * sizeof( color_data ) );
	color_data_origin->origin_datas = three_color_data;

	// �F���Əc���̑傫�����i�[
	int i = 0;
	for( unsigned int x = 0 ; x < png.height * png.width ; x++ , i++ ){
		three_color_data[i].red   = buf[ x * 3 + 0 ]; 
		three_color_data[i].green = buf[ x * 3 + 1 ];
		three_color_data[i].blue  = buf[ x * 3 + 2 ];
	}
	color_data_origin->info.biHeight = png.height;
	color_data_origin->info.biWidth  = png.width;
	color_data_origin->info.biSize = sizeof( BITMAPINFOHEADER ); // bmp�ϊ��p�ɕۑ����Ă���
	color_data_origin->header.bfType = 0x4d42;

	return color_data_origin;
}

// 4bit�̃f�[�^�ۑ��֐�
static void image_color_data_load_4bit( color_data *color_data_origin , FILE *fp   , color_data *pallet_data , const int x ){
	static unsigned char pallet_number = 0;
	if( x % 2 == 0 ) { fread( &pallet_number , 1 , 1 , fp ); }
	const unsigned char pallet_index = ( pallet_number >> ( ( x % 2 == 0 ) ? 4 : 0 ) ) & 0x0F;
	*color_data_origin = pallet_data[pallet_index];
}

// 8bit�̃f�[�^�ۑ��֐�
static void image_color_data_load_8bit( color_data *color_data_origin , FILE *fp   , color_data *pallet_data , const int x ){
	unsigned char pallet_number = 0;
	fread( &pallet_number   , 1 , 1 , fp );	
	*color_data_origin = pallet_data[pallet_number];
}

// 16bit�̃f�[�^�ۑ��֐�
static void image_color_data_load_16bit( color_data *color_data_origin , FILE *fp  , color_data *pallet_data , const int x ){
	unsigned short w = 0;
	fread( &w , sizeof( w ) , 1 , fp );
	// �V�t�g���ĐF�̃f�[�^���i�[
	color_data_origin->red   = ( ( w >> 10 ) & 0x1F ) << 3;
	color_data_origin->green = ( ( w >>  5 ) & 0x1F ) << 3;
	color_data_origin->blue  = ( ( w >>  0 ) & 0x1F ) << 3;
}

// 24bit�̃f�[�^�ۑ��֐�
static void image_color_data_load_24bit( color_data *color_data_origin , FILE *fp , color_data *pallet_data , const int x ){ fread( color_data_origin , 3 , 1 , fp ); }

// 32bit�̃f�[�^�ۑ��֐�
static void image_color_data_load_32bit( color_data *color_data_origin , FILE *fp , color_data *pallet_data , const int x ){ fread( color_data_origin , 4 , 1 , fp ); }

// BMP�t�@�C���ǂݍ��݊֐�
static color_data_origin *bmp_image_color_data_load( color_data_origin *origin_data , FILE *fp ){

	// �o�C�g���ɉ������z����쐬
	color_data *three_color_data =  new color_data[ origin_data->info.biHeight * origin_data->info.biWidth ];
	memset( three_color_data , 0 , ( origin_data->info.biHeight * origin_data->info.biWidth ) * sizeof( color_data ) );
	origin_data->origin_datas = three_color_data;		// ���{�ƐF���̔z���ڑ� 

	// 4bit,8bit�p�����@�p���b�g�f�[�^�ǂݍ���
	color_data *pallet_data = NULL ;
	if(  origin_data->info.biBitCount == TYPE_4_BIT || origin_data->info.biBitCount == TYPE_8_BIT ){ 
		int pallet_size = (int)pow( 2.0 , origin_data->info.biBitCount ); 
		pallet_data = new color_data[ pallet_size ];
		memset( pallet_data , 0 , sizeof( pallet_data ) );
		fread( pallet_data , sizeof( color_data ) , pallet_size , fp );
	}

	// �֐��|�C���^�錾
	void ( *image_color_data_load )( color_data *origin_data , FILE *fp , color_data *pallet_data  , const int x ) = NULL;
	if( origin_data->info.biBitCount == TYPE_4_BIT ){ image_color_data_load = image_color_data_load_4bit; }
	else if( origin_data->info.biBitCount == TYPE_8_BIT ){ image_color_data_load = image_color_data_load_8bit; }
	else if( origin_data->info.biBitCount == TYPE_16_BIT ){ image_color_data_load = image_color_data_load_16bit; }
	else if( origin_data->info.biBitCount == TYPE_24_BIT ){ image_color_data_load = image_color_data_load_24bit; }
	else{ image_color_data_load = image_color_data_load_32bit; }
	
	// �J���[�f�[�^�ǂݍ��ݏ���
	for( int y = 0 ; y < origin_data->info.biHeight ; y++ ) { 
		for( int x = 0 ; x < origin_data->info.biWidth ; x++ ) { 
			image_color_data_load( &origin_data->origin_datas[ x + y * origin_data->info.biWidth ] , fp , pallet_data , x ); // �֐��|�C���^�Ő���
		}
		// 4byte��؂�
		int padding_size = 0;
		if(origin_data->info.biBitCount == TYPE_4_BIT ){ padding_size = ( 4 - ( (int)ceil((float)origin_data->info.biWidth / 2) % 4 ) ) % 4;}
		else{ padding_size = ( 4 - (  origin_data->info.biWidth * ( origin_data->info.biBitCount / 8 )  % 4 ) ) % 4; }
		fseek( fp , padding_size , SEEK_CUR );
	}
	delete[] pallet_data;

	return origin_data;
}

// 4bit�̃f�[�^�������݊֐�
static void image_data_write_4bit( color_data *color_data_origin , FILE *fp , color_data *pallet_data , const int x ){
	unsigned char number_of_digit;			// 1���ځA2���ڂ�ۑ�����ϐ�
	static unsigned char sum_digit = 0;			// 2�̐��������̂����i�[����ϐ�
	// 2�̃p���b�g���w���ԍ������̂����锻��&�����@����
	for( int j = 0 ; j < PALLET_SIZE_4_BIT ; j++ ){
		if( *color_data_origin == pallet_data[j] ){
			number_of_digit = j;
			break;
		}
	}
	// 1�o�C�g�p�ɃV�t�g���Ċi�[
	if( x % 2 == 0 ){ sum_digit = number_of_digit << 4; }
	else{ sum_digit = sum_digit | number_of_digit << 0; }
	// ��ԍ��̃C���f�b�N�X�̎��̂ݏ������݂��s��
	if( x % 2 != 0 ){ fwrite( &sum_digit  , sizeof( sum_digit ) , 1 , fp ); }
}

// 8bit�̃f�[�^�������݊֐�
static void image_data_write_8bit( color_data *color_data_origin , FILE *fp , color_data *pallet_data , const int x ){
	for( int j = 0 ; j < PALLET_SIZE_8_BIT ; j++ ){
		if( *color_data_origin == pallet_data[j] ){
			fwrite( &j , 1 , 1 , fp );
			break;
		}
	}
}

// 16bit�̃f�[�^�������݊֐�
static void image_data_write_16bit( color_data *color_data_origin , FILE *fp , color_data *pallet_data , const int x ){
	unsigned short w = 0;
	w |= ( ( color_data_origin->red   >> 3 ) & 0x1F ) << 10;
	w |= ( ( color_data_origin->green >> 3 ) & 0x1F ) <<  5;
	w |= ( ( color_data_origin->blue  >> 3 ) & 0x1F ) <<  0;
	fwrite( &w , sizeof( w )  , 1 , fp );
}

// 24bit�̃f�[�^�������݊֐�
static void image_data_write_24bit( color_data *color_data_origin , FILE *fp , color_data *pallet_data , const int x ){ fwrite( color_data_origin , 3 , 1 , fp ); }

// 32bit�̃f�[�^�������݊֐�
static void image_data_write_32bit( color_data *color_data_origin , FILE *fp , color_data *pallet_data , const int x ){ fwrite( color_data_origin , 4 , 1 , fp ); }

// �O���[�X�P�[���ϊ��֐�
static void image_transfer_gray_scale( color_data_origin *color_data_origin ){

	color_data *three_color_data = color_data_origin->origin_datas;
	for( int i = 0 ; i < color_data_origin->info.biHeight * color_data_origin->info.biWidth ; i++ , three_color_data++ ){
		int gray_scale = 0;
		gray_scale = three_color_data->blue +  three_color_data->green + three_color_data->red;
		gray_scale /= 3;
		three_color_data->blue  = gray_scale;
		three_color_data->green = gray_scale;
		three_color_data->red   = gray_scale;
	}
}

// �Z�s�A�ϊ��֐�
static void image_transfer_sepia( color_data_origin *color_data_origin ){

	color_data *three_color_data = color_data_origin->origin_datas;
	static const float table[3] = { 0.4f , 0.7f , 0.9f };			// �Z�s�A���p�␳�l B,G,R�̏���
	image_transfer_gray_scale( color_data_origin );			// ���߂ɃO���[�X�P�[����
	for( int i = 0 ; i < color_data_origin->info.biHeight * color_data_origin->info.biWidth ; i++ ){
		// �e�v�f�ɕ␳�l�������Ă���
		three_color_data[i].blue  = ( unsigned char )( three_color_data[i].blue  * table[0] ); 
		three_color_data[i].green = ( unsigned char )( three_color_data[i].green * table[1] ); 
		three_color_data[i].red   = ( unsigned char )( three_color_data[i].red   * table[2] ); 
	}
}

// �l�K�|�W�ϊ��֐�
static void image_transfer_negaposi( color_data_origin *color_data_origin ){

	color_data *three_color_data = color_data_origin->origin_datas;
	for( int i = 0 ; i < color_data_origin->info.biHeight * color_data_origin->info.biWidth ; i++ ){ 
		three_color_data[i].blue  = ~three_color_data[i].blue;
		three_color_data[i].green = ~three_color_data[i].green;
		three_color_data[i].red   = ~three_color_data[i].red;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*----------------------------------------------��--------�� ���C������Ă΂��֐� ��-----��--------------------------------------------------*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// �����t�@�C�����ǂݍ��݊֐�
color_data_origin *image_data_create( const char *file_name , const int file_change_flg ){

	// �t�@�C���I�[�v������
	FILE *fp = file_open( "rb" , file_name );
	if( fp == NULL ){ return NULL; }

	// �ǂݍ��񂾃t�@�C����BMP���ǂ�������
	unsigned short file_type;
	fread( &file_type , 2 , 1 , fp );
	fseek( fp , 0 , SEEK_SET );
	unsigned short bmp_file = 0x4d42;
	if( file_type == bmp_file ){ file_type = BMP_MODE; }

	// �t�@�C���̏��ۑ�����
	color_data_origin *color_data_origin =  new struct color_data_origin;
	memset( color_data_origin , 0 , sizeof( struct color_data_origin ) );

	// �ϊ��ɕK�v�ȃf�[�^�쐬
	if( file_type == BMP_MODE ){  // BMP�t�@�C������
		image_header_data_load( color_data_origin , fp );							// �t�@�C���w�b�_���쐬���ď����i�[
		color_data_origin = bmp_image_color_data_load( color_data_origin , fp );	// �F�ۑ��p�z��̍쐬
	}else{						  // PNG�t�@�C������
		color_data_origin = png_image_data_load( color_data_origin ,  file_name , file_change_flg );
	}

	file_close( fp );
	return color_data_origin;
}

// �摜�f�[�^���A�b�v�R���o�[�g����֐�
void image_data_up_convert( color_data_origin *color_data_origin , int image_bit_type ){

	// �A�b�v�R���o�[�g��̐�������
	if( image_bit_type == SELECT_32_BIT ){ image_bit_type = TYPE_32_BIT; }
	else if( image_bit_type == SELECT_24_BIT ){ image_bit_type = TYPE_24_BIT; }
	else if( image_bit_type == SELECT_16_BIT ){ image_bit_type = TYPE_16_BIT; }
	else if( image_bit_type == SELECT_8_BIT ){ image_bit_type = TYPE_8_BIT; }
	else{ return; }

	// 4byte��؂�@�␳�l�v�Z
	const int bit_color_type = ( image_bit_type / 8 ); //�O�F�����o�C�g�ŕ\�����邩�̕ϐ�
	int padding_size = ( 4 - ( ( color_data_origin->info.biWidth * bit_color_type ) % 4 ) ) % 4;

	// �t�@�C���w�b�_�[�����ɑ������l�̌v�Z
	int image_offset = sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) ;
	if( image_bit_type <= 8 ) { image_offset += (int)pow( 2.0f , image_bit_type ) * sizeof( RGBQUAD ); } 
	
	color_data_origin->header.bfSize    = image_offset + ( ( color_data_origin->info.biWidth * bit_color_type ) + padding_size ) * color_data_origin->info.biHeight;
	color_data_origin->header.bfOffBits = image_offset;
	color_data_origin->info.biBitCount  = image_bit_type;
	color_data_origin->info.biSizeImage = color_data_origin->header.bfSize - image_offset;
}

// �ϊ������I���֐�
int image_data_transfer( const int select , color_data_origin *color_data_origin ){

	switch( select ) {
		case SELECT_GRAY_SCALE  : image_transfer_gray_scale( color_data_origin ); break;	// �O���[�X�P�[���ϊ�
		case SELECT_SEPIA		: image_transfer_sepia( color_data_origin );	  break;	// �Z�s�A�ϊ�
		case SELECT_NEGAPOSI	: image_transfer_negaposi( color_data_origin );   break;	// �l�K�|�W�ϊ�
		default					: return ERR_CONVERT;										// �G���[���b�Z�[�W����
	}
	return NO_ERR;
}

// �摜�f�[�^�������ݏ���
void image_data_write( color_data_origin *color_data_origin , const char *export_file_name , const int file_type , int file_change_flg ){

	// �t�@�C���̎�ނɉ����ď������݂��s��
	if( file_type == BMP_MODE ){  // �r�b�g�}�b�v���[�h

		// �w�b�_�[�̏����t�@�C���ɏ�������
		FILE *fp = file_open( "wb" , export_file_name );
		fwrite( &color_data_origin->header , sizeof( color_data_origin->header ) , 1 , fp );	// �t�@�C���w�b�_����
		fwrite( &color_data_origin->info   , sizeof( color_data_origin->info )   , 1 , fp );	// ���w�b�_����
		color_data *three_color_data = color_data_origin->origin_datas;							// �F���p�̃|�C���^�擾

		// 4bit,8bit�p�����@�p���b�g�f�[�^�\���̍쐬
		color_data *pallet_data = NULL ;
		if( color_data_origin->info.biBitCount == TYPE_4_BIT || color_data_origin->info.biBitCount == TYPE_8_BIT ){ 
			int pallet_size = (int)pow( 2.0 , color_data_origin->info.biBitCount ); 
			pallet_data = new color_data[ pallet_size ];
			memset( pallet_data , 0 , sizeof( pallet_data ) * pallet_size );
		
			// �p���b�g�f�[�^�̍쐬���s��
			int pallet_cnt = 0;
			for( int i = 0 ; i < color_data_origin->info.biHeight * color_data_origin->info.biWidth ; i++ ) {
				int n = 0;
				for( ; n < pallet_cnt ; n++ ) { 
					if( color_data_origin->origin_datas[i] == pallet_data[n] ) { break; }
				}
				if( n == pallet_cnt ) { pallet_data[n] = color_data_origin->origin_datas[i]; pallet_cnt++; }
			}
			fwrite( pallet_data , sizeof( pallet_data ) * pallet_size , 1 , fp );	// �J���[�p���b�g�̏������݂��s��
		}

		// �֐��|�C���^�錾
		void ( *image_color_data_write )( color_data *origin_data , FILE *fp , color_data *pallet_data  , const int x ) = NULL;
		if( color_data_origin->info.biBitCount == TYPE_4_BIT ){ image_color_data_write = image_data_write_4bit; }
		else if( color_data_origin->info.biBitCount == TYPE_8_BIT ){ image_color_data_write = image_data_write_8bit; }
		else if( color_data_origin->info.biBitCount == TYPE_16_BIT ){ image_color_data_write = image_data_write_16bit; }
		else if( color_data_origin->info.biBitCount == TYPE_24_BIT ){ image_color_data_write = image_data_write_24bit; }
		else{ image_color_data_write = image_data_write_32bit; }

		for( int y = 0 ; y < color_data_origin->info.biHeight ; y++ ) { 
			for( int x = 0 ; x < color_data_origin->info.biWidth ; x++ ) { 
				image_color_data_write( &color_data_origin->origin_datas[ x + y * color_data_origin->info.biWidth ] , fp , pallet_data , x );
			}
			// 4byte��؂�
			int padding_size = 0;
			if(color_data_origin->info.biBitCount == TYPE_4_BIT ){ padding_size = ( 4 - ( (int)ceil((float)color_data_origin->info.biWidth / 2) % 4 ) ) % 4;}
			else{ padding_size = ( 4 - (  color_data_origin->info.biWidth * ( color_data_origin->info.biBitCount / 8 )  % 4 ) ) % 4; }
			int blank_data = 0;
			fwrite( &blank_data  , 1 , padding_size , fp );
		}

		delete[] pallet_data;
		file_close( fp );

	}else{	// �s���O���[�h

		// �f�[�^���t�@�C���ɏ�������
		png_image png;
		memset( &png , 0 , sizeof( png ) );
		png.version = PNG_IMAGE_VERSION;
		png.width = color_data_origin->info.biWidth;
		png.height = color_data_origin->info.biHeight;
		png.format = PNG_FORMAT_RGB;

		// �ǂݏo�������ɍ��킹�āA�f�[�^�����i�[����o�b�t�@���m��
		uint32_t stride = PNG_IMAGE_ROW_STRIDE(png);
		uint8_t  *buf   = new uint8_t[ PNG_IMAGE_BUFFER_SIZE( png , stride ) ];

		// �J���[�f�[�^��������
		for( unsigned int i = 0 ; i < png.height * png.width ; i++ ){
			buf[ i * 3 + 0 ] = color_data_origin->origin_datas[i].red;
			buf[ i * 3 + 1 ] = color_data_origin->origin_datas[i].green;
			buf[ i * 3 + 2 ] = color_data_origin->origin_datas[i].blue;
		}

		// �g���q���ύX�����Ȃ�摜�𔽓]�����鏈��
		if( file_change_flg == 1 ){ file_change_flg = -1; }
		else{ file_change_flg = 1; }
		png_image_write_to_file( &png , export_file_name , 0 , buf , stride * file_change_flg , NULL ); // �`�揈��

		delete[] buf; // �������
	}
}

// �������
void image_data_release( color_data_origin *color_data_origin ){
	color_data *three_color_data = color_data_origin->origin_datas;
	delete[] three_color_data;
	delete[] color_data_origin;
}
